#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/any.hpp>
#include <functional>
#include <atomic>
#include "Mutex.h"
#include "CurrentThread.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"
#include <stdio.h>
#include <vector>
class Poller;
class Channel;
class TimerQueue;
class EventLoop : noncopyable
{
public:
	typedef std::function<void()> Functor;

	void assertInLoopThread()
	{
		
		if(!isInLoopThread())
		{
			//printf(" EventLoop : : abortNotInLoopThread()\n");
			abortNotInLoopThread();
		}
	}
	void cancel(TimerId timerId);

	bool eventHandling() const { return eventHandling_; }
	EventLoop();
	~EventLoop();

	static EventLoop* getEventLoopOfCurrentThread();
	const boost::any& getContext() const { return context_; }
	boost::any* getMutableContext(){ return &context_; }

	bool hasChannel(Channel* channel);

	int64_t iteration() const { return iteration_; }
	bool isInLoopThread() const
	{
		return threadId_ == CurrentThread::tid();
	}	

	void loop();
	Timestamp pollReturnTime() const { return pollReturnTime_; }

	void queueInLoop(Functor cb);
	size_t queueSize() const;
	void quit();

	void runInLoop(Functor cb);
	void removeChannel(Channel* channel);
	TimerId runAt(Timestamp time, TimerCallback cb);
	TimerId runAfter(double delay, TimerCallback cb);
	TimerId runEvery(double interval, TimerCallback cb);

	void setContext(const boost::any& context) { context_ = context; }
	void updateChannel(Channel * channel);
	void wakeup();
	
private:
	typedef std::vector<Channel*> ChannelList;
	ChannelList activeChannels_;
	void abortNotInLoopThread();

	boost::any context_;
	bool callingPendingFunctors_;
	Channel* currentActiveChannel_;

	void doPendingFunctors();
	bool eventHandling_; 	
	void handleRead();
	int64_t iteration_;
	bool looping_;
	
	mutable MutexLock mutex_;

	//acquire Poller active and this activechannel
	Timestamp pollReturnTime_;
	std::unique_ptr<Poller> poller_;
	std::vector<Functor> pendingFunctors_;
	void printActiveChannels() const;
	
	std::atomic<bool> quit_;

	std::unique_ptr<TimerQueue> timerQueue_;
	const pid_t threadId_;

	int wakeupFd_;
	std::unique_ptr<Channel> wakeupChannel_;

};
#endif
