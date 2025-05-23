#include "muduo/net/File.h"
#include "muduo/base/Logging.h"

#include <iostream>
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <string.h>

using namespace muduo;
using namespace muduo::net;

File::File(EventLoop *loop, const std::string &filename, int flags)
    : fd_(::open(filename.c_str(), flags, 0666)),
      loop_(loop),
      channel_(loop, fd_)
{
    if (channel_.fd() < 0)
    {
        LOG_SYSFATAL << "Failed to open file: " << filename;
    }
}

File::~File()
{
    if (channel_.fd() >= 0)
    {
        ::close(channel_.fd());
    }
}

void File::write(const std::string &data)
{
    buffer_ = data;

    // 对于普通文件描述符，epoll/io_uring 不会提供 EPOLLIN/EPOLLOUT 就绪事件
    // 回调函数始终无法触发
    // 如果是普通文件（regular file），直接同步写
    struct stat st;
    if (::fstat(channel_.fd(), &st) == 0 && S_ISREG(st.st_mode)) {
        handleWrite();
        return;
    }
    // 如果是管道（pipe）或套接字（socket），则使用异步写
    channel_.setWriteCallback(std::bind(&File::handleWrite, this));
    channel_.enableWriting(); // channel新增监听fd的write事件
}

void File::read()
{
    channel_.setReadCallback(std::bind(&File::handleRead, this));
    channel_.enableReading(); // channel新增监听fd的read事件
}

void File::handleWrite()
{
    ssize_t n = ::write(channel_.fd(), buffer_.data(), buffer_.size());
    if (n > 0)
    {
        LOG_INFO << "Write " << n << " bytes to file.";
        buffer_.erase(0, n);
        if (buffer_.empty())
        {
            channel_.disableWriting(); // 数据写完后，取消监听fd的write事件
        }
    }
    else
    {
        LOG_ERROR << "Write error.";
    }
}

void File::handleRead()
{
    char buf[1024];
    ssize_t n = ::read(channel_.fd(), buf, sizeof(buf) - 1);
    if (n > 0)
    {
        buf[n] = '\0';
        LOG_INFO << "Read " << n << " bytes from file: " << buf;
        // 如果有用户设置的回调函数，则调用回调函数处理读取到的数据
        // 否则直接输出到标准输出
        if (readCallback_)
        {
            readCallback_(std::string(buf));
        }
        else
        {
            std::cout.write(buf, n + 1);
        }
        channel_.disableReading(); // 数据读完后，取消监听fd的read事件
    }
    else if (n == 0)
    {
        LOG_INFO << "EOF reached.";
        // EOF 时触发用户回调，即使没有数据，也要通知上层退出 loop
        if (readCallback_)
        {
            readCallback_(std::string());
        }
        // 取消读监听，关闭 fd
        channel_.disableReading();
        ::close(channel_.fd());
    }
    else
    {
        LOG_ERROR << "Read error.";
    }
}

void File::close()
{
    if (fd_ >= 0)
    {
        channel_.disableAll();
        ::close(fd_);
        fd_ = -1;
    }
}