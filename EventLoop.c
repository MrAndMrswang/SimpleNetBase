#include"Channel.h"
#include"Poller.h"
#include"EventLoop.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/eventfd.h>

namespace
{
	__thread EventLoop* t_loopInthisThread = 0;
	const int kPollTimeMs = 10000;
	/*int createEventfd()
	{
		int evtfd = :: eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
		if(evtfd < 0)
		{
			printf("LOG_SYSERR : failed in eventfd");
			//LOG_SYSERR << "Failed in eventfd";
			abort();
		}
		return evtfd;
	}

	#pragma GCC diagnostic ignored "-Wold-style-cast"
	class IgnoreSigPipe
	{
	public:
		ignoreSigPipe()
		{
			::signal(SIGPIPE, SIG_IGN);
		}
	};
	#pragma GCC diagnostic error "-wold-style-cast"
	
	IgnoreSigPipe initObj;*/
};

//,threadId_(CurrentThread::tid())
EventLoop::EventLoop():looping_(false),threadId_(100)
{
	printf("LOG_TRACE<<EventLoop created");
	//LOG_TRACE<<"EventLoop created" <<this<<"in thread"<<threadId_;
	if(t_loopInthisThread)
	{
		printf("LOG_FATAL <<Another EventLoop<<t_loopInThisThread");
		//LOG_FATAL <<"Another EventLoop " <<t_loopInThisThread<<" exists in this thread "<<threadId; 
	}
	else
	{
		
		t_loopInthisThread = this;
	}

}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	
	while(!quit_)
	{
		int kPollTimesMs = 1;
		activeChannels_.clear();
		poller_->poll(kPollTimesMs,&activeChannels_);
		for(ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end();++it)
		{
			(*it) -> handleEvent();
		}
	}
	printf("LOG_TRACE EventLoop this stop looping ");
	//LOG_TRACE << " EventLoop "<<this<<" stop looping ";
	//looping_ = false;
}

void EventLoop::updateChannel(Channel * channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}


/*EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInthisThread;
}*/


EventLoop::~EventLoop()
{
	assert(!looping_);
	t_loopInthisThread = NULL;

}

void EventLoop::quit()
{
	quit_ = true;
}

void EventLoop::abortNotInLoopThread()
{
	printf("abortNotInLoopThread()");
 /* LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();*/
}
