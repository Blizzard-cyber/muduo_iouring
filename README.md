

```
                       __  __           _
                      |  \/  |         | |
                      | \  / |_   _  __| |_   _  ___
                      | |\/| | | | |/ _` | | | |/ _ \
                      | |  | | |_| | (_| | |_| | (_) |
                      |_|  |_|\__,_|\__,_|\__,_|\___/
                      
```


# muduo_iouring

基于 Muduo 网络库，新增 `io_uring` Poller 和 `File` 类，并提供 Google Test 单元测试。

## 一、项目简介

本项目在原生 [Muduo](https://github.com/chenshuo/muduo) 源码基础上做了两项扩展：

1. **IoUringPoller**  
   使用 Linux `io_uring` 代替默认的 `EPollPoller`，提供更高效的 I/O 多路复用支持。  
2. **File**  
   为普通文件提供统一的读写封装：  
   - 普通文件同步读写
   - 管道/套接字异步读写（依托 Muduo `Channel` + `EventLoop`）  

同时，采用 Google Test 对 `File` 类的写入、读取、EOF、异常打开等功能进行了全面的单元测试。

## 二、项目结构

```
.
├── build.sh               # 一键构建脚本
├── README                 # 项目说明
├── muduo/                 # 原生 Muduo 源码（含扩展文件）
│   ├── net/
│   │   ├── CMakeLists.txt # 添加 File 和 IoUringPoller
│   │   ├── File.h         # 新增 File 接口
│   │   ├── File.cc        # 新增 File 实现
│   │   └── poller/
│   ├       |── DefaultPoller.cc  # 设置默认 Poller 为 IoUringPoller
│   │       ├── iouringPoller.h   # 新增 iouringPoller 接口
│   │       └── iouringPoller.cc  # 新增 iouringPoller 实现
│   └── …                  # 其他 Muduo 源码
└── test/                  # 单元测试工程
    ├── CMakeLists.txt
    └── FileTest.cc        # Google Test 用例
```

## 三、依赖

- Linux 5.x 内核（支持 `io_uring`）  
- CMake ≥ 3.5  
- GCC ≥ 7 或 Clang ≥ 5  
- liburing (`liburing-dev` 或 `liburing-devel`)  
- pthread / rt 库  
- Google Test (`gtest`, `gtest-devel`)

## 四、快速上手

```bash
# 克隆仓库
git clone https://github.com/Blizzard-cyber/muduo_iouring.git
cd muduo_iouring/

# 1. 构建并安装 Muduo 静态库和头文件到 test 目录
./build.sh           # 生成 build/release-cpp11
./build.sh install   # 拷贝 libmuduo_net.a、libmuduo_base.a 和头文件到 test/lib、test/include

# 2. 进入 test 子工程
cd test
mkdir build && cd build

# 3. 编译并运行单元测试
cmake .. -DCMAKE_BUILD_TYPE=Debug
make FileTest
./FileTest
# 或者
ctest -V
```

测试通过后，即可在 lib 和 include 中直接引用已编译好的静态库与头文件，将本项目集成到其他工程。

## 五、主要内容

- **muduo/net/poller/iouringPoller.\***  
  - `io_uring` 初始化、提交 SQE（poll_add/poll_remove）、等待/收集 CQE  
  - 替换默认 `Poller::newDefaultPoller` 为 IoUringPoller
- **muduo/net/File.\***  
  - 对普通文件（regular file）做同步写和同步/异步读（触发 EOF 回调）  
  - 对管道/套接字做异步读写，所有 I/O 操作回调驱动 `EventLoop`
- **test/FileTest.cc**  
  - 写-读验证、空文件读取、连续写入一次读取、关闭后写无效、打开不存在文件死亡测试  

