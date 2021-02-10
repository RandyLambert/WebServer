#ifndef SSXRVER_NET_CONNECTION_H
#define SSXRVER_NET_CONNECTION_H
#include <memory>
#include <netinet/in.h>
#include <functional>
#include <string_view>
#include <atomic>
#include <boost/noncopyable.hpp>
#include "Buffer.h"
#include "CallBacks.h"
#include "../http/HttpParser.h"
#include "../base/File.h"
namespace ssxrver::net
{
class EventLoop;
class Channel;

class Connection : boost::noncopyable,
                   public std::enable_shared_from_this<Connection>
{
public:
    explicit Connection(EventLoop *loop,
                        int sockFd);
    ~Connection();

    EventLoop *getLoop() const { return loop_; }

    bool connected() const { return state_ == kConnected; }
    void send(std::string_view message);
    void send(Buffer *buf);
    void shutdown();
    void forceClose();
    void setTcpNoDelay(bool on) const;
    void startRead();
    void stopRead();
    bool isReading() const { return reading_; }
    bool isSendFile() const { return sendFile_ != nullptr;}
    void sendFileReset() { if(sendFile_ != nullptr)  sendFile_.reset(); };
    const std::unique_ptr<HttpRequestParser>& getContext() const //获取未知类型，不能更改
    {
        return context_;
    }

    std::unique_ptr<HttpRequestParser>& getMutableContext() //get可变的，可以更改
    {
        return context_;
    }

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    std::unique_ptr<Channel>& getChannel(){ return channel_; }
    void setSendFile(std::unique_ptr<base::file::SendFileUtil> file){ sendFile_ = std::move(file); }
    Buffer *inputBuffer() { return &inputBuffer_; }
    Buffer *outputBuffer() { return &outputBuffer_; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();
    void connectReset(int sockFd);
private:
    enum StateE //连接的状态
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string_view& message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { state_ = s;}
    void startReadInLoop();
    void stopReadInLoop();

    EventLoop *loop_; // 所属eventLoop
    std::atomic_uint8_t state_;
    int sockFd_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;

    Buffer inputBuffer_;  //应用层的接收缓冲区
    Buffer outputBuffer_; //应用层的发送缓冲区，当outputbuffer高到一定程度
    std::unique_ptr<base::file::SendFileUtil> sendFile_;
    std::unique_ptr<HttpRequestParser> context_;  //位置类型的上线文对象
    bool reading_;
};

typedef std::shared_ptr<Connection> TcpConnectionPtr;
} // namespace ssxrver

#endif