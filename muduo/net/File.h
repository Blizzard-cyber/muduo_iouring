#ifndef MUDUO_NET_FILE_H
#define MUDUO_NET_FILE_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/Channel.h>
#include <functional>

namespace muduo
{
    namespace net
    {
        class File : noncopyable
        {
        public:
            using FileCallback = std::function<void(const std::string &)>;

            File(EventLoop *loop, const std::string &filename, int flags);
            ~File();

            void write(const std::string &data);
            void read();
            void close();
            void setReadCallback(const FileCallback &cb) { readCallback_ = cb; }

        private:
            void handleWrite();
            void handleRead();

            int fd_;
            EventLoop *loop_;
            Channel channel_;
            std::string buffer_;        // 用于write()函数的缓冲区
            FileCallback readCallback_; // 参数为read读取到的string，提供给用户进行处理
        };

    } // namespace net
} // namespace muduo

#endif // MUDUO_NET_FILE_H