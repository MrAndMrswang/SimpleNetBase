#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include "copyable.h"

class Timer;

class TimerId 
{
public:
	TimerId()
	: timer_(NULL),
	sequence_(0)
	{
	}

	TimerId(Timer* timer, int64_t seq)
	: timer_(timer),
	sequence_(seq)
	{
	}

friend class TimerQueue;

private:
	Timer* timer_;
	int64_t sequence_;
};

#endif 

