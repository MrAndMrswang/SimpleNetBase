#ifndef MUDUO_NET_POLLER_POLLPOLLER_H
#define MUDUO_NET_POLLER_POLLPOLLER_H

#include "Poller.h"
#include <vector>

struct pollfd;


class PollPoller : public Poller
{
public:
	/*Base class function
	virtual bool hasChannel(Channel* channel) const;
	static Poller* newDefaultPoller(EventLoop* loop);
	void assertInLoopThread() const
	{
		ownerLoop_->assertInLoopThread();
	}*/

	PollPoller(EventLoop* loop);
	virtual ~PollPoller();

	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
	virtual void updateChannel(Channel* channel);
	virtual void removeChannel(Channel* channel);

private:
	void fillActiveChannels(int numEvents,
		          ChannelList* activeChannels) const;

	typedef std::vector<struct pollfd> PollFdList;
	PollFdList pollfds_;
};


#endif  

