// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "../Poller.h"
// #include "PollPoller.h"
// #include "EPollPoller.h"
#include "iouringPoller.h"

#include <stdlib.h>

using namespace muduo::net;

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
  // if (::getenv("MUDUO_USE_POLL"))
  // {
  //   return new PollPoller(loop);
  // }

  // else if (::getenv("MUDUO_USE_EPOLL"))
  // {
  //   return new EPollPoller(loop);
  // }

  // 这个函数在Poller.h中声明
  // 原本代码通过获取环境变量的值来判断使用poll还是epoll
  // 因为一般不会设置MUDUO_USE_POLL环境变量，因此默认使用EPOLL作为IO复用
  // 现在默认使用IOURING作为IO复用
  // else
  {
    return new IoUringPoller(loop);
  }
}
