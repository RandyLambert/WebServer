// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_EVENTLOOPTHREADPOOL_H
#define SSERVER_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <functional>
#include <memory>

namespace sserver
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool //这个类的作用是开启若干个线程，且让每个线程都处于事件循环状态
{                         //mainreactor关注的是监听套接字，其他对应的是io操作的套接字
public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop *baseLoop, const string &nameArg);
  EventLoopThreadPool(const EventLoopThreadPool &) = delete;
  EventLoopThreadPool &operator=(EventLoopThreadPool &) = delete;
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop *getNextLoop();

  /// with the same hash code, it will always return the same EventLoop
  EventLoop *getLoopForHash(size_t hashCode);

  std::vector<EventLoop *> getAllLoops();

  bool started() const
  {
    return started_;
  }

private:
  EventLoop *baseLoop_; //与acceptor所属的eventloop相同
  string name_;
  bool started_;
  int numThreads_;                                        //线程数
  int next_;                                              //新连接到来，所选择的eventloop对象下标，是比较公平
  std::vector<std::unique_ptr<EventLoopThread>> threads_; //io线程列表
  std::vector<EventLoop *> loops_;                        //eventloop列表
};

} // namespace net
} // namespace sserver

#endif // SSERVER_EVENTLOOPTHREADPOOL_H