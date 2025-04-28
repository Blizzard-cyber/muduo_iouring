cmake_minimum_required(VERSION 3.5)
project(muduo_iouring_test)

# 开启测试功能
enable_testing()
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
include_directories (./include DIR_INC)
link_directories(./lib DIR_LIB)

add_executable(FileTest FileTest.cc)
target_link_libraries(FileTest
  libmuduo_net.a
  libmuduo_base.a
  pthread
  rt
  uring
  GTest::GTest
  GTest::Main
)

add_test(NAME FileTest COMMAND FileTest)
