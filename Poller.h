#ifndef POLLER_H
#define POLLER_H
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"

struct pollfd;
class Channel;

class Poller: public boost::noncopyable
{  
public:
	typedef std::vector<Channel*> ChannelList;
	Poller(EventLoop* Loop);
	~Poller();
	//Timestamp poll(int timeoutMs, ChannelList* activeChannels);
        void poll(int timeoutMs, ChannelList* activeChannels);
	void updateChannel(Channel * channel);
	
	void assertInLoopThread()
	{
		ownerLoop_->assertInLoopThread();
	}
	
private:
	void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
	typedef std::vector<struct pollfd> PollFdList;
	typedef std::map<int, Channel*> ChannelMap;
	EventLoop* ownerLoop_;
	PollFdList pollfds_;
	ChannelMap channels_;
	
};

#endif
