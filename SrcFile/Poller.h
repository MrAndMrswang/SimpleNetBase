#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>
#include <boost/noncopyable.hpp>

#include "Timestamp.h"
#include "EventLoop.h"

class Channel;

class Poller : boost::noncopyable
{
public:
	typedef std::vector<Channel*> ChannelList;

	Poller(EventLoop* loop);
	virtual ~Poller();
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
	virtual void updateChannel(Channel* channel) = 0;
	virtual void removeChannel(Channel* channel) = 0;
	virtual bool hasChannel(Channel* channel) const;

	static Poller* newDefaultPoller(EventLoop* loop);

	void assertInLoopThread() const
	{
		ownerLoop_->assertInLoopThread();
	}

protected:
	typedef std::map<int, Channel*> ChannelMap;
	ChannelMap channels_;

private:
	EventLoop* ownerLoop_;
};


#endif  // MUDUO_NET_POLLER_H

