#ifndef CHANNEL_H
#define CHANNEL_H
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "Timestamp.h"
class EventLoop;

class Channel: public boost::noncopyable
{
public:
	typedef boost::function<void()> EventCallback;
	typedef boost::function<void(Timestamp)> ReadEventCallback;
	//init channel info
	Channel(EventLoop* loop, int fd);
	~Channel();
	//handleEvent() to exec corresponding function
	void handleEvent(Timestamp receiveTime);
	//set read write errro function
	void setReadCallback(const ReadEventCallback& cb)
	{ readCallback_ = cb; }
	void setWriteCallback(const EventCallback& cb)
	{ writeCallback_ = cb; }
	void setCloseCallback(const EventCallback& cb)
	{ closeCallback_ = cb; }
	void setErrorCallback(const EventCallback& cb)
	{ errorCallback_ = cb; }
	
	void tie(const boost::shared_ptr<void> &);
	//return  struct poll
	int fd() const {return fd_;}
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt;}
	//judge event is kNoneEvent
	bool isNoneEvent() const { return events_ == kNoneEvent; }
	
	void enableReading() {events_ |= kReadEvent;update();}
	void disableReading(){events_&=~kReadEvent;update();}
	void enableWriting(){events_|=kWriteEvent;update();}
	void disableWriting(){events_&=~kWriteEvent;update();}
	void disableAll(){events_ = kNoneEvent;update();}
	bool isWriting()const {return events_ & kWriteEvent;}
	bool isReading()const {return events_ & kReadEvent;}

	
	//for poller to ourloop
	//index for unique index for fd number
	int index(){return index_;}
	void set_index(int idx){ index_ = idx;}

	string reventsToString()const;
	string eventsToString()const;
	
	void doNotLogHup(){logHup_ = false;}

	EventLoop* ownerLoop() {return loop_;}
	void remove();
	
private:
	static string eventsToString(int fd,int ev);
	void update();
	void handleEventWithGuard(Timestamp receiveTime);	

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
	
	EventLoop* loop_;
	const int fd_;
	int events_;
	int revents_;
	int index_;
	bool logHup_;

	boost::weak_ptr<void> tie_;
	bool tied_;
	bool eventHandling_;
	bool addedToLoop_;
	
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback errorCallback_;
	EventCallback closeCallback_;
};

#endif
