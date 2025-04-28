#include "muduo/net/poller/iouringPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"

#include <poll.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

IoUringPoller::IoUringPoller(EventLoop *loop)
    : Poller(loop)
{
    int ret = io_uring_queue_init(256, &ring_, 0); // 初始化struct io_uring
    if (ret < 0)
    {
        LOG_SYSFATAL << "io_uring_queue_init failed";
    }
}

IoUringPoller::~IoUringPoller()
{
    io_uring_queue_exit(&ring_);  // 析构
}

Timestamp IoUringPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    struct io_uring_cqe *cqe;

    // 将timeoutMs转化为__kernel_timespec
    struct __kernel_timespec ts;
    ts.tv_sec = timeoutMs / 1000;
    ts.tv_nsec = (timeoutMs % 1000) * 1000000;

    // 等待完成队列（CQ）中至少出现一个完成事件（CQE）或超时
    // 返回 0 表示取到第一个事件，>0 表示超时，<0 表示出错
    int ret = io_uring_wait_cqe_timeout(&ring_, &cqe, &ts);

    if (ret >= 0)
    {
        events_.clear();

        // 取到一个CQE,并循环取出所有CQE
        while (ret == 0)
        {
            events_.push_back(cqe);
            io_uring_cqe_seen(&ring_, cqe); // 标记CQE已被处理
            ret = io_uring_peek_cqe(&ring_, &cqe);  // 非阻塞地检查 CQ 是否有事件,有事件时返回 0 并通过 cqe_ptr 返回
        }

        // 将结果填充到activateChannels中
        fillActiveChannels(static_cast<int>(events_.size()), activeChannels);
    }
    else if (ret < 0 && ret != -ETIME)
    {
        LOG_ERROR << "io_uring_wait_cqe_timeout error: " << strerror(-ret);
    }

    return Timestamp::now();
}

void IoUringPoller::updateChannel(Channel *channel)
{
    const int fd = channel->fd();

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_); // 从提交队列（SQ）获取一个空闲的 SQE（
    if (!sqe)
    {
        LOG_ERROR << "io_uring_get_sqe failed";
        return;
    }

    io_uring_prep_poll_add(sqe, fd, channel->events()); // 在 SQE 中填充一个 “poll_add” 请求

    // io-uring cqe返回结果中没有revent,因此在user_data中设置
    PollerInfo *user_data = new PollerInfo;
    user_data->channel = channel;
    user_data->events = channel->events();

    io_uring_sqe_set_data(sqe, user_data); // 将PollerInfo结构体指针设置到SQE中
    int ret = io_uring_submit(&ring_);  // 提交SQE到提交队列（SQ）中
    if (ret < 0)
    {
        LOG_ERROR << "io_uring_submit failed: " << strerror(-ret);
        delete user_data;  // 释放内存
        return;
    }
    channels_[fd] = channel;
}

void IoUringPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
    if (!sqe)
    {
        LOG_ERROR << "io_uring_get_sqe failed";
        return;
    }

    io_uring_prep_poll_remove(sqe,reinterpret_cast<void*>(static_cast<intptr_t>(fd)));


    int ret = io_uring_submit(&ring_);
    if (ret < 0)
    {
        LOG_ERROR << "io_uring_submit failed: " << strerror(-ret);
    }
}

void IoUringPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        PollerInfo *user_data = reinterpret_cast<PollerInfo *>(events_[i]->user_data);

        Channel *channel = user_data->channel;
        channel->set_revents(user_data->events);
        activeChannels->push_back(channel);

        delete user_data;  // 释放内存 
    }
}