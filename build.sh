#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
BUILD_TYPE=${BUILD_TYPE:-release}
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}-install-cpp11}
CXX=${CXX:-g++}

ln -sf $BUILD_DIR/$BUILD_TYPE-cpp11/compile_commands.json

mkdir -p $BUILD_DIR/$BUILD_TYPE-cpp11 \
  && cd $BUILD_DIR/$BUILD_TYPE-cpp11 \
  && cmake \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
           $SOURCE_DIR \
  && make $*

# 如果执行了 install，就把生成的静态库和头文件拷贝到 test 目录下
if [ "$1" = "install" ]; then
  echo "Copying libs and headers to test/"
  mkdir -p $SOURCE_DIR/test/lib
  mkdir -p $SOURCE_DIR/test/include
  # 只拷贝 muduo_net 和 muduo_base 两个静态库
  cp -v "$INSTALL_DIR/lib/libmuduo_net.a"   "$SOURCE_DIR/test/lib/"
  cp -v "$INSTALL_DIR/lib/libmuduo_base.a"  "$SOURCE_DIR/test/lib/"

  # 头文件保持全部拷贝
  cp -rv "$INSTALL_DIR/include/"*           "$SOURCE_DIR/test/include/"
fi
# Use the following command to run all the unit tests
# at the dir $BUILD_DIR/$BUILD_TYPE :
# CTEST_OUTPUT_ON_FAILURE=TRUE make test

# cd $SOURCE_DIR && doxygen

