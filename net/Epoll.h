#ifndef SSXRVER_NET_EPOLL_H
#define SSXRVER_NET_EPOLL_H
#include <vector>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"

struct epoll_event;
namespace ssxrver::net
{

class Channel;
class TcpConnection;
class Epoll : boost::noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    explicit Epoll(EventLoop *loop);
    ~Epoll();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    void poll(ChannelList *activeChannels);

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    void update(int operation, Channel *channel);

    using ChannelMap = std::unordered_map<int, Channel *>;                      //fd和事件指针
    using TcpConnMap = std::unordered_map<int, std::shared_ptr<TcpConnection>>; //TcpConnectionMap
    ChannelMap channels_;                                             //监听检测通道
    TcpConnMap connections_;
    int epollfd_;
    using EventList = std::vector<struct epoll_event>;
    EventList events_; //实际返回的事件列表
    EventLoop *ownerLoop_;

};

} // namespace ssxrver::net
#endif
