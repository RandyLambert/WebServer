#ifndef SSXRVER_HTTP_HTTTPSERVER_H
#define SSXRVER_HTTP_HTTTPSERVER_H
#include "../net/TcpServer.h"
#include <boost/noncopyable.hpp>
#include "../net/EventLoop.h"
#include "../base/ThreadPool.h"
namespace ssxrver::net
{
class HttpRequest;
class HttpResponse;
class HttpServer : boost::noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    using HttpCallback = std::function<void(const HttpRequest &,
                               HttpResponse *)>;
    explicit HttpServer(EventLoop *loop,
               struct sockaddr_in listenAddr,
                       int taskProcesses);
    ~HttpServer() = default;
    [[nodiscard]] EventLoop *getLoop() const { return server_.getLoop(); }
    void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }
    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); } //多线程
    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn,                   //当服务器端收到了一个客户端发过来的http请求
                   Buffer *buf);                                   //首先回调onmessage，在onmessage中调用了onrequest，
    void onRequest(const TcpConnectionPtr &, const HttpRequest &); //在onRequest中调用了httpcallback_
    base::ThreadPool threadPool_;
    int threadPoolNumber_;
    TcpServer server_;
    HttpCallback httpCallback_; //在处理http请求的时候(即调用onrequest)的过程中回调此函数，并且对请求做处处理
};

} // namespace ssxrver::net
#endif
