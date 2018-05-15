#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <stdio.h>
class Poller;
class Channel;

class EventLoop: public boost::noncopyable
{
public:
	EventLoop();
	~EventLoop();

	void loop();
	bool isInLoopThread() const
	{
		return true;
		//return threadId_ == CurrentThread::tid();
	}	

	void updateChannel(Channel * channel);
	void assertInLoopThread()
	{
		printf(" EventLoop.h assertInLoopThread()");
		if(!isInLoopThread())
		{
			abortNotInLoopThread();
		}
	}
	
	void quit();

	
private:
	EventLoop* getEventLoopOfCurrentThread();
	void abortNotInLoopThread();
	bool looping_;
	const pid_t threadId_;
	typedef std::vector<Channel*> ChannelList;
	bool quit_;
	boost::scoped_ptr<Poller> poller_;
	ChannelList activeChannels_;
};
#endif
