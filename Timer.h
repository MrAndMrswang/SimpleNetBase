#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <boost/noncopyable.hpp>

class Timer : boost::noncopyable
{
public:
  Timer(const TimerCallback& cb, Timestamp when, double interval)
    : callback_(cb), //outtime back function
      expiration_(when),	//outtime time
      interval_(interval), //repeat time
      repeat_(interval > 0.0), //judge if repeat
      sequence_(s_numCreated_.incrementAndGet()) //set timer seq
  { }

/*#ifdef __GXX_EXPERIMENTAL_CXX0X__
  Timer(TimerCallback&& cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(s_numCreated_.incrementAndGet())
  { }
#endif*/

	void run() const
	{
		callback_();
	}

	Timestamp expiration() const  { return expiration_; }
	bool repeat() const { return repeat_; }
	int64_t sequence() const { return sequence_; }

	void restart(Timestamp now);

	static int64_t numCreated() { return s_numCreated_.get(); }

private:
	const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
	const int64_t sequence_;

	static AtomicInt64 s_numCreated_;
};

#endif

