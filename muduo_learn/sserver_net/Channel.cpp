// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "../sserver_base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sstream>

#include <poll.h>

using namespace sserver;
using namespace sserver::net;

const int Channel::kNoneEvent = 0; //初始化默认值
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd__)
	: loop_(loop),
	  fd_(fd__),
	  events_(0),
	  revents_(0),
	  index_(-1),
	  logHup_(true),
	  tied_(false),
	  eventHandling_(false),
	  addedToLoop_(false)
{
}

Channel::~Channel()
{
	assert(!eventHandling_);
	assert(!addedToLoop_);
	if (loop_->isInLoopThread())
	{
		assert(!loop_->hasChannel(this));
	}
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
	tie_ = obj;
	tied_ = true;
}

void Channel::update()
{
	addedToLoop_ = true;
	loop_->updateChannel(this); //调用loop的update，loop的update又调用了channel的update
}

void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) //事件到来了，调用handlevent处理
{
	std::shared_ptr<void> guard;
	if (tied_)
	{
		guard = tie_.lock();
		if (guard)
		{
			handleEventWithGuard(receiveTime); //调用这个
		}
	}
	else
	{
		handleEventWithGuard(receiveTime); //调用了Channel中的readCallback_ writeCallback_ errorCallback_等这些函数
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) //判断一下返回的事件，进行处理
	{												  //被挂断了
		if (logHup_)								  //入过有这个信号，打印一下警告信息
		{
			LOG_WARN << "Channel::handle_event() POLLHUP";
		}
		if (closeCallback_)
			closeCallback_();
	}
	//TcpConnection和Acceptor进行注册
	if (revents_ & POLLNVAL) //文件描述符没有打开，或者异常
	{
		LOG_WARN << "Channel::handle_event() POLLNVAL"; //记录警告
	}

	if (revents_ & (POLLERR | POLLNVAL)) //错误的返回error
	{
		if (errorCallback_)
			errorCallback_(); //回调错误函数
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) //可读时间，最后一个是对等方关闭链接或关闭半连接，read返回0
	{
		if (readCallback_)
			readCallback_(receiveTime);
	}
	if (revents_ & POLLOUT) //写人物
	{
		if (writeCallback_)
			writeCallback_();
	}
	eventHandling_ = false;
}

std::string Channel::reventsToString() const
{ //调试，发生了什么事件
	std::ostringstream oss;
	oss << fd_ << ": ";
	if (revents_ & POLLIN)
		oss << "IN ";
	if (revents_ & POLLPRI)
		oss << "PRI ";
	if (revents_ & POLLOUT)
		oss << "OUT ";
	if (revents_ & POLLHUP)
		oss << "HUP ";
	if (revents_ & POLLRDHUP)
		oss << "RDHUP ";
	if (revents_ & POLLERR)
		oss << "ERR ";
	if (revents_ & POLLNVAL)
		oss << "NVAL ";

	return oss.str().c_str();
}
