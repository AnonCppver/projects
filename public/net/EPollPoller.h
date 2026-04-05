#ifndef _LEEF_NET_POLLER_EPOLLPOLLER_H
#define _LEEF_NET_POLLER_EPOLLPOLLER_H

#include "Poller.h"

#include <vector>

struct epoll_event;

namespace leef
{
  namespace net
  {

    // 基于 epoll 的 I/O 多路复用器实现
    class EPollPoller : public Poller
    {
    public:
      // 构造函数：创建 epoll 实例
      // loop: 所属的 EventLoop
      EPollPoller(EventLoop *loop);

      // 析构函数：关闭 epoll fd
      ~EPollPoller() override;

      // 核心函数：等待 I/O 事件发生
      // timeoutMs: 超时时间（毫秒）
      // activeChannels: 输出参数，返回就绪的 Channel
      //
      // 内部调用 epoll_wait：
      // - 阻塞等待事件
      // - 返回发生事件的 fd 列表
      // - 再转换为 Channel
      Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

      // 更新 Channel 关注的事件（读/写/关闭等）
      // - 新增 fd（EPOLL_CTL_ADD）
      // - 修改事件（EPOLL_CTL_MOD）
      void updateChannel(Channel *channel) override;

      // 从 epoll 中移除 Channel
      // - fd 关闭
      // - 不再关注该事件
      void removeChannel(Channel *channel) override;

    private:
      // 初始 events_ 容器大小
      static const int kInitEventListSize = 16;

      static const char *operationToString(int op);

      // 将 epoll 返回的事件填充到 activeChannels
      //
      // numEvents: epoll_wait 返回的事件数量
      // activeChannels: 输出结果
      void fillActiveChannels(int numEvents,
                              ChannelList *activeChannels) const;

      // 将关注的fd_event添加/修改/删除到 epoll 实例中
      // operation:
      //   EPOLL_CTL_ADD / MOD / DEL
      void update(int operation, Channel *channel);

      int epollfd_;

      // epoll_wait 返回的事件数组
      //
      // - 就绪事件集合
      // - 每次 poll() 后会被填充
      // - 如果不够用会扩容
      typedef std::vector<struct epoll_event> EventList;
      EventList events_;
    };

  } // namespace net
} // namespace leef
#endif // _LEEF_NET_POLLER_EPOLLPOLLER_H
