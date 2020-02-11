// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef SSERVER_TCPCLIENT_H
#define SSERVER_TCPCLIENT_H

#include "../sserver_base/Mutex.h"
#include "TcpConnection.h"

namespace sserver
{
namespace net
{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient
{
public:
    // TcpClient(EventLoop* loop);
    // TcpClient(EventLoop* loop, const string& host, uint16_t port);
    TcpClient(const TcpClient &) = delete;
    TcpClient &operator=(const TcpClient &) = delete;

    TcpClient(EventLoop *loop,
              const InetAddress &serverAddr,
              const string &name);
    ~TcpClient(); // force out-line dtor, for scoped_ptr members.

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    EventLoop *getLoop() const { return loop_; }
    bool retry() const;
    void enableRetry() { retry_ = true; }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setConnectionCallback(ConnectionCallback &&cb)
    {
        connectionCallback_ = std::move(cb);
    }
    void setMessageCallback(MessageCallback &&cb)
    {
        messageCallback_ = std::move(cb);
    }
    void setWriteCompleteCallback(WriteCompleteCallback &&cb)
    {
        writeCompleteCallback_ = std::move(cb);
    }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);
    /// Not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    ConnectorPtr connector_; // avoid revealing Connector用于发起主动连接
    const string name_;//名称
    ConnectionCallback connectionCallback_;//连接建立的回调函数
    MessageCallback messageCallback_;//消息到来的回调函数
    WriteCompleteCallback writeCompleteCallback_;//数据发送完毕回调函数
    bool retry_;   // atomic重连，是指连接建立之后又意外断开的时候是否重连
    bool connect_; // atomic连接
    // always in loop thread
    int nextConnId_;//name_+nextconnid_用于标识一个连接
    mutable MutexLock mutex_;
    TcpConnectionPtr connection_; // @GuardedBy mutex_ connector连接成功以后，得到一个tcpconnection
};

} // namespace net
} // namespace sserver

#endif // SSERVER_TCPCLIENT_H
