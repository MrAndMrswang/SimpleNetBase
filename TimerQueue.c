


int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
	if(timerfd<0)
		//LOG_SYSFATAL<<"Failed in timerfd create";
	}
	return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
	if(microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microseconds/Timestamp::kMicroSecondsPerSecond);
	ts.tc_nsec  =static_cast<long>((microseconds% Timestamp::kMicroSecondsPerSecond)*1000);
	return ts;
}

void readTimerfd(int timerfd,Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd,&howmany,sizeof howmany);
	//LOG_TRACE<<"TimerQueue::handleRead()"<<howmany<<" at " << now.toString();
	if (n != sizeof howmany)
	{
		//LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}
}

void resetTimerfd(int timerfd, Timerstamp expiration)
{
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldvalue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd,0,&newValue,&oldValue);
	if(ret)
	{
		//LOG_SYSERR<<"timerfd_settime()";
	}
}



TimerId TimerQueue::addTimer(const TimerCallback& cb,Timestamp when,double interval)
{
	Timer* timer = new Timer(cb,when,interval);
	loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop,this,timer));
	return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assert(InLoopThread());
	bool earliestChanged = insert(timer);
	
	if(earliestChanged)
	{
		resetTimerfd(timerfd_,timer->expiration());
	}
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();
	bool earliestChanged = insert(timer);
	if(earliestChanged)
	{
		resetTimerfd(timerfd_,timer->expiration());
	}
}

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(boost::bind(&TimerQueue::cancelInLoop,this,timerId));
}


void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->asserTInLoopThread();
	assert(timers_.size() == activeTimers_.size());
	ActiveTimer timer(timerId.timer_,timerId.sequence_);
	ActiveTimerSet::iterator it = activeTimers_.find(timer);
	if(it != activeTimers_.end()) //if it in activeTimerset
	{
		size_t n =timers_.erase(Entry(it->first->expiration(),it->first));
		assert(n==1);(void)n;
		delete it->first;
		activeTimers_.erase(it);
	}
	else if(callingExpiredTimers_)  //if calling expired function
	{
		cancelingTimers_.insert(timer);
	}
	assert(timers_.size() == activeTimers_.size());
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() = activeTimers_.size());
	std::vector<Entry> expired;
	Entry sentry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
	
	TimerList::iterator end= timers_.lower_bound(sentry);
	assert(end == timers_.end() || now < end->first);

	std::copy(timers_.begin(),end,back_inserter(expired)); //back_inserter in case of allocate space
	timers_.erase(timers_.begin(),end);

	for(std::vector<Entry>::iterator it = expired.begin();it!=expired.end();++it)
	{
		ActiveTimer timer(it->second,it->second->sequence());
		size_t n = activeTimers_.erase(timer);
		assert(n == 1);(void) n;
	}	

	assert(timers_.size() == activeTimers_.size());
	return expired;
	
}

void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());
	readTimerfd(timerfd_,now);

	std::vector<Entry> expired = getExpired(now);
	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	for(std::vector<Entry>::iterator it = expired.begin();it !=expired.end();++it)
	{
		it->second->run();
	}
	callingExpiredTimers_ = false;
	reset(expired,now);
}

bool TimerQueue::insert(Timer* timer)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());
	bool earliestChanged = false;
	Timestamp when = timer->expiration();
	TimerList::iterator it = timers_.begin();
	if(it == timers_.end() || when < it->first) //if this timer is earliest 
	{
		earliestChanged  = true;
	}
	{
		std::pair<TimerList::iterator,bool> result = timers._insert(Entry(when,timer));
		assert(result.second);(void)result;
	}
	{
		std::pair<ActiveTimerSet::iterator,bool> result = activeTimers_.insert(ActiveTimer(timer,timer->sequence()));
		assert(result.second);(void) result;
	}
	assert(timers_.size() == activeTimers_.size());
	return earliestChanged;
}


void TimerQueue::reset(const std::vector<Entry>& expired,Timestamp now)
{
	Timestamp nextExpire;
	for(std::vector<Entry>::const_iterator it = expired.begin());it!=expired.end();it++)
	{
		ActiveTimer timer(it->second,it->second->sequence());
		if(it->second->repeat()&&cancelingTimers_.find(timer) == cancelingTimers_.end())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
			delete it->second;
		}
	}
	if(!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}
	if(nextExpire.valid())
	{
		resetTimerfd(timerfd_,nextExpire);
	}
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false)
{
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	::close(timerfd_);
	for(TimerList::iterator it = timers_.begin();it!=timers_.end();++it)
	{
		delete it->second;
	}
}









