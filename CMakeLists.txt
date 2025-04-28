cmake_minimum_required(VERSION 3.5)

project(muduo_iouring_test)


find_package(GTest REQUIRED)
# 添加 lib 目录到头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/lib)

# 把 File.cc 源文件也编译进测试可执行文件
add_executable(FileTest
  FileTest.cc
  lib/File.cc
)


target_link_libraries(FileTest
  PRIVATE
    muduo_net
    muduo_base
    pthread
    rt
    uring
    GTest::GTest
    GTest::Main
)
enable_testing()
add_test(NAME FileTest COMMAND FileTest)