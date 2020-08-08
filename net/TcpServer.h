#ifndef SSXRVER_NET_TCPSERVER_H
#define SSXRVER_NET_TCPSERVER_H
#include <map>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include "TcpConnection.h"
#include "Channel.h"
namespace ssxrver
{
namespace net
{

class EventLoop;
class EventLoopThreadPool;
class TcpServer : boost::noncopyable
{
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(EventLoop *)> ThreadInitCallback;
    typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
    typedef std::function<void(const TcpConnectionPtr &,
                               Buffer *buffer)>
        MessageCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
    TcpServer(EventLoop *loop,
              struct sockaddr_in listenAddr);
    ~TcpServer();

    EventLoop *getLoop() const { return loop_; }
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    //std::unique_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

    void start(); //启动线程池
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
    void acceptSockListen();

private:
    void newConnection(int socked);
    void removeConnection(const TcpConnectionPtr &conn);
    void acceptHandRead();

    EventLoop *loop_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    std::atomic<bool> started_;
    int acceptfd_;
    int idleFd_;
    Channel acceptChannel_;
};

} // namespace net
} // namespace ssxrver
#endif
