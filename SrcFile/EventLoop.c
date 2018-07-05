#include "Channel.h"
#include "Poller.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "SocketOps.h"
//#include "Logging.h"
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/eventfd.h>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;
int createEventfd()
{
	int evtfd = ::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
	if(evtfd < 0)
	{
		printf("failed in eventfd\n");
		//LOG_SYSERR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

//#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
	IgnoreSigPipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};
//#pragma GCC diagnostic error "-wold-style-cast"

IgnoreSigPipe initObj;


void EventLoop::abortNotInLoopThread()
{
	printf("EventLoop::abortNotInLoopThread()\n");
 /* LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();*/
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for(size_t i = 0;i<functors.size();i++)
	{
		functors[i]();
	}
	callingPendingFunctors_ = false;
}


EventLoop::EventLoop():looping_(false) ,quit_(false),eventHandling_(false),
callingPendingFunctors_(false),iteration_(0),threadId_(CurrentThread::tid()),
poller_(Poller::newDefaultPoller(this)),timerQueue_(new TimerQueue(this)),wakeupFd_(createEventfd()),
wakeupChannel_(new Channel(this,wakeupFd_)),currentActiveChannel_(NULL)
{
	printf("EventLoop::EventLoop created\n");
	//LOG_TRACE<<"EventLoop created" <<this<<"in thread"<<threadId_;
	if(t_loopInThisThread)
	{
		printf("EventLoop::Another EventLoop\n");
		abort();
		//LOG_FATAL <<"Another EventLoop " <<t_loopInThisThread<<" exists in this thread "<<threadId; 
	}
	else
	{
		
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
	wakeupChannel_->enableReading();

}

EventLoop::~EventLoop()
{
	//LOG_DEBUG<<"EventLoop"<<this<<"of thread"<<threadId_<<"destructs in thread"<<CurrentThread::tid();
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = NULL;

}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_,&one,sizeof one);
	
	if(n!=sizeof one)
	{
		//LOG_ERROR<<"EventLoop::handleRead() reads "<< n <<"bytes instead of 8";
	}
}


bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	//LOG_TRACE<<"EventLoop"<<this<<"start looping";	

	while(!quit_)
	{
		activeChannels_.clear();
		//detect for fill activeChannel[]
		pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
		++iteration_;
		//if(Logger::logLevel()<=Logger::TRACE)
		//{
		//	printActiveChannels();
		//}
		eventHandling_ = true;
		for(ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end();++it)
		{
			currentActiveChannel_ = *it;
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}
	
	printf("EventLoop::EventLoop stop looping\n");
	//LOG_TRACE << " EventLoop "<<this<<" stop looping ";
	looping_ = false;
}

void EventLoop::printActiveChannels()const
{
	for (ChannelList::const_iterator it = activeChannels_.begin();it!=activeChannels_.end();++it)
	{
		const Channel * ch = *it;
		printf("channel info is %s",ch->reventsToString().c_str());
		//LOG_TRACE<<"{"<<ch->reventsToString()<<"}";
	}
}


void EventLoop::quit()
{
	quit_ = true;
	if(!isInLoopThread())
	{
		wakeup();
	}
}

size_t EventLoop::queueSize()const
{
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}
	if(!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}


void EventLoop::runInLoop(Functor cb)
{
	if(isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(std::move(cb));
	}
}

void EventLoop::cancel(TimerId timerId)
{
	return timerQueue_->cancel(timerId);
}


void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if(eventHandling_)
	{
		assert(currentActiveChannel_ == channel || std::find(activeChannels_.begin(),activeChannels_.end(),channel) == activeChannels_.end());		
	}
	poller_->removeChannel(channel);
}


TimerId EventLoop::runAt(Timestamp time , TimerCallback cb)
{
	printf("EventLoop::runAt \n");
	return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay , TimerCallback cb)
{
	printf("EventLoop::runAfter %f\n",delay);
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
	printf("EventLoop::runEvery %f\n",interval);
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::updateChannel(Channel * channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_,&one,sizeof one);
	
	if((n) != sizeof one)
	{
		printf("LOG_ERROR,wakeup\n");
		//LOG_ERROR<<"EventLoop::wakeup() writes "<<n <<"bytes instead of 8";
	}
}
