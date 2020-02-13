// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef SSERVER_NET_TIMER_H
#define SSERVER_NET_TIMER_H

#include "../sserver_base/Atomic.h"
#include "../sserver_base/Timestamp.h"
#include "Callbacks.h"

namespace sserver
{
namespace net
{
//
//Internal class for timer event.

class Timer //对定时操作的高层次抽象
{
public:
  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;

  Timer(TimerCallback cb, Timestamp when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval), //构造函数
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet()) //先加后获取，原子操作，如果有多个计时器同时生成，也不会出问题
  {
  }

  void run() const //调用回调函数
  {
    callback_();
  }

  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  void restart(Timestamp now);

  static int64_t numCreated() { return s_numCreated_.get(); }

private:
  const TimerCallback callback_; //定时器回调函数
  Timestamp expiration_;         //下一次超时时刻
  const double interval_;        //超时时间间隔，如果为一次行定时器，该值为0
  const bool repeat_;            //时候重复
  const int64_t sequence_;       //定时器序号

  static AtomicInt64 s_numCreated_; //定时器计数，当前已经创建的定时器数量
};
} // namespace net
} // namespace sserver
#endif // SSERVER_NET_TIMER_H