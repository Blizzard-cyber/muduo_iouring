#include <gtest/gtest.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/File.h>
#include <unistd.h>
#include <fcntl.h>

using namespace muduo::net;

class FileTest : public ::testing::Test {
protected:
  EventLoop loop_;
  std::string filename_;

  void SetUp() override {
    // mkstemp 会在 /tmp 下生成 testfileXXXXXX，
    // 并打开文件返回 fd
    char tmpl[] = "/tmp/testfileXXXXXX";
    int fd = ::mkstemp(tmpl);
    ASSERT_GE(fd, 0);
    ::close(fd);
    filename_ = tmpl;
  }

  void TearDown() override {
    ::unlink(filename_.c_str());
  }
};

TEST_F(FileTest, WriteThenRead) {
  File file(&loop_, filename_, O_RDWR);

  // 1. 发起一次异步写
  std::string s = "hello muduo";
  file.write(s);

  // 运行 loop，等写事件完成
  loop_.runAfter(0.1, [&](){ loop_.quit(); });
  loop_.loop();

  // 2. 设置读回调，触发读
  std::string out;
  file.setReadCallback([&](const std::string& buf){
    out = buf;
  });
  file.read();

  loop_.runAfter(0.1, [&](){ loop_.quit(); });
  loop_.loop();

  EXPECT_EQ(out, s);
}