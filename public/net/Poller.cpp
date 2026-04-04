#include "Poller.h"
#include "EPollPoller.h"

#include "Channel.h"

using namespace leef;
using namespace leef::net;

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const
{
  assertInLoopThread();
  ChannelMap::const_iterator it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
  return new EPollPoller(loop);
}
