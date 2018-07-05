#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "noncopyable.h"
#include "Mutex.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{
public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();
	
	TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
	void cancel(TimerId timerId);

private:
	typedef std::pair<Timestamp,Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;
	
	void addTimerInLoop(Timer* timer);
	ActiveTimerSet activeTimers_;
	bool callingExpiredTimers_;

	ActiveTimerSet cancelingTimers_;
	void cancelInLoop(TimerId timerId);	

	std::vector<Entry> getExpired(Timestamp now);
	void handleRead();
	bool insert(Timer * timer);
	EventLoop* loop_;

	void reset(const std::vector<Entry>& expired, Timestamp now);

	const int timerfd_;
	Channel timerfdChannel_;
	TimerList timers_;
	
};
#endif
