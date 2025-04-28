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
    int fd = ::mkstemp(tmpl);
    ASSERT_GE(fd, 0);
    ::close(fd);
    filename_ = tmpl;
  }

  void TearDown() override {
    ::unlink(filename_.c_str());
  }
};

TEST_F(FileTest, WriteThenReadWithNewHandle) {
  // 1. 异步写
  File fileW(&loop_, filename_, O_RDWR);
  std::string toWrite = "hello muduo";
  fileW.write(toWrite);
  loop_.runAfter(0.1, [&](){ loop_.quit(); });
  loop_.loop();

  // 2. 新建一个只读 File 实例，偏移从头开始
  File fileR(&loop_, filename_, O_RDONLY);
  std::string out;
  fileR.setReadCallback([&](const std::string& buf){
    out = buf;
    loop_.quit();
  });
  fileR.read();
  // loop_.runAfter(0.1, [&](){ loop_.quit(); });
  loop_.loop();

  EXPECT_EQ(out, toWrite);
}


TEST_F(FileTest, ReadEmptyFile) {
  // 打开一个新文件，未写入任何字节，直接读应得到空字符串
  File fileR(&loop_, filename_, O_RDONLY);
  std::string out;
  fileR.setReadCallback([&](const std::string& buf) { out = buf; loop_.quit(); });
  fileR.read();
  loop_.runAfter(0.05, [&](){ loop_.quit(); });
  loop_.loop();
  EXPECT_EQ(out, "");
}


TEST_F(FileTest, MultipleWritesSingleRead) {
  File fileW(&loop_, filename_, O_RDWR);

  // 第一次写，然后跑 loop
  fileW.write("hello ");
  loop_.runAfter(0.05, [&](){ loop_.quit(); });
  loop_.loop();

  // 第二次写，再跑 loop
  fileW.write("world");
  loop_.runAfter(0.05, [&](){ loop_.quit(); });
  loop_.loop();

  // 读取
  File fileR(&loop_, filename_, O_RDONLY);
  std::string out;
  fileR.setReadCallback([&](const std::string& buf){
    out = buf; loop_.quit();
  });
  fileR.read();
  loop_.runAfter(0.05, [&](){ loop_.quit(); });
  loop_.loop();

  EXPECT_EQ(out, "hello world");
}

TEST_F(FileTest, WriteAfterClose) {
  File fileW(&loop_, filename_, O_RDWR);
  fileW.close();

  // 再次写不应抛异常，也不应触发写操作
  EXPECT_NO_THROW(fileW.write("data"));

  // 验证读不到任何数据
  File fileR(&loop_, filename_, O_RDONLY);
  std::string out;
  fileR.setReadCallback([&](const std::string&){ out = "got"; loop_.quit(); });
  fileR.read();
  loop_.runAfter(0.05, [&](){ loop_.quit(); });
  loop_.loop();

  EXPECT_EQ(out, "");
}

TEST_F(FileTest, OpenNonExistFileForRead) {
  std::string bad = filename_ + "_noexist";
  // 放宽死亡测试匹配规则，匹配任意终止
  EXPECT_DEATH(
    { File f(&loop_, bad, O_RDONLY); },
    ".*"
  );
}