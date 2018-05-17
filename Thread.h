#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H


#include "Atomic.h"
#include "Types.h"
#include "CountDownLatch.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>

class Thread: boost::noncopyable
{
public:
	typedef boost::function<void ()> ThreadFunc;
	explicit Thread(const ThreadFunc&, const string& name = string());
	~Thread();
	void start();
	int join();
	bool started() const{return started;}
	pid_t tid() const {return tid_;}
	
	const string& name() const { return name_;}
	static int numCreated() {return numCreated_.get();}
private:
	void setDefaultName();
	bool started_;
	bool joined_;
	pthread_t pthreadId_;
	pid_t tid_;
	ThreadFunc func_;
	string name_;
	CountDownLatch latch_;
	static AtomicInt32 numCreated;

}

#endif
