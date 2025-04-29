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
    char tmpl[] = "/tmp/testfileXXXXXX";
    // 创建一个唯一的临时文件，并返回该文件的文件描述符
    int fd = ::mkstemp(tmpl);
    ASSERT_GE(fd, 0);
    ::close(fd);
    filename_ = tmpl;
  }

  void TearDown() override {
    ::unlink(filename_.c_str());
  }
};

// 1. 写一次再读
TEST_F(FileTest, WriteThenRead) {
  std::string result;
  // 写文件（同步写回）
  File fileW(&loop_, filename_, O_RDWR);
  fileW.write("hello muduo");

  // 读文件（异步触发回调后 quit）
  File fileR(&loop_, filename_, O_RDONLY);
  fileR.setReadCallback([&](const std::string& buf){
    result = buf;
    loop_.quit();
  });
  fileR.read();

  loop_.loop();
  EXPECT_EQ(result, "hello muduo");
}

// 2. 读空文件(无写)
TEST_F(FileTest, ReadEmptyFile) {
  std::string result;
  File fileR(&loop_, filename_, O_RDONLY);
  fileR.setReadCallback([&](const std::string& buf){
    result = buf;
    loop_.quit();
  });
  fileR.read();

  loop_.loop();
  EXPECT_EQ(result, "");
}

// 3. 多次写入，单次读取
TEST_F(FileTest, MultipleWritesSingleRead) {

  // 1) 用 File::write 同步写两段
  File fileW(&loop_, filename_, O_RDWR);
  fileW.write("hello ");
  fileW.write("world");

  // 2) 异步读并退出 loop
  std::string result;
  File fileR(&loop_, filename_, O_RDONLY);
  fileR.setReadCallback([&](const std::string& buf) {
    result = buf;
    loop_.quit();
  });
  fileR.read();

  // 3) run loop 等待读回调触发 quit
  loop_.loop();
  EXPECT_EQ(result, "hello world");
}

// 4. 关闭后写不生效
TEST_F(FileTest, WriteAfterCloseNoEffect) {
  File fileW(&loop_, filename_, O_RDWR);
  fileW.close();
  // 再写不会崩溃
  EXPECT_NO_THROW(fileW.write("data"));

  // 读不到任何数据
  std::string result = "init";
  File fileR(&loop_, filename_, O_RDONLY);
  fileR.setReadCallback([&](const std::string& buf){
    result = buf;
    loop_.quit();
  });
  fileR.read();

  loop_.loop();
  EXPECT_EQ(result, "");
}

// 5. 打开不存在文件应致死（死亡测试）
TEST_F(FileTest, OpenNonExistFileDeath) {
  std::string bad = filename_ + "_noexist";
  EXPECT_DEATH(
    { File f(&loop_, bad, O_RDONLY); },
    ".*"
  );
}