cmake_minimum_required(VERSION 3.5)
project(muduo_iouring_test)

# 开启测试功能
enable_testing()


# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories (./include DIR_INC)
link_directories(./lib DIR_LIB)

find_library(MUDUO_BASE muduo_base PATHS ${CMAKE_SOURCE_DIR}/../build/lib NO_DEFAULT_PATH)
find_library(MUDUO_NET  muduo_net  PATHS ${CMAKE_SOURCE_DIR}/../build/lib NO_DEFAULT_PATH)



add_executable(FileTest FileTest.cc)
target_link_libraries(FileTest
  muduo_net
  muduo_base
  pthread
  rt
  uring
  libgtest.a 
  libgtest_main.a
)

add_test(NAME FileTest COMMAND FileTest)
# # 指定 libmuduo_*.a 位于 build/lib
# find_library(LIB_MUDUO_BASE
#   NAMES muduo_base
#   PATHS ${CMAKE_BINARY_DIR}/lib
#   NO_DEFAULT_PATH
# )
# find_library(LIB_MUDUO_NET
#   NAMES muduo_net
#   PATHS ${CMAKE_BINARY_DIR}/lib
#   NO_DEFAULT_PATH
# )

# # 或者直接让 link_directories 搜索 build/lib
# link_directories(${CMAKE_BINARY_DIR}/lib)

# add_executable(IoUringPollerTest IoUringPollerTest.cc)


# target_link_libraries(IoUringPollerTest
#   ${LIB_MUDUO_NET}
#   ${LIB_MUDUO_BASE}
#   pthread rt uring
#   gtest gtest_main gmock
# )

# add_test(NAME IoUringPollerTest COMMAND IoUringPollerTest)