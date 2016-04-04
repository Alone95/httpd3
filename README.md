## 这是一个HTTP服务器

### Version-1.1.3

- 高性能高并发
- C语言开发
- `epoll` + 类线程池技术
- `Linux` 环境， 非跨平台
- `gcc -std=gnu99 -pthread -DWSX_DEBUG`
- 使用`CMake-3.3`。 可以直接修改版本号搭配自己平台的使用，当前`apt`上的版本为`2.8`
- 英文注释

### 测试使用
- `$ cmake .`
- `$ make`
- `$ ./httpd3`
- 配置参见下方说明

### 模块
- 最外部为入口程序，以及读取配置的函数。
- `handle` 模块则是对于 **读/写/错误** 事件的一个控制
- `memop`模块是用来扩展内存分配的，例如`jcmalloc`，目前只是使用自带的库函数，并加一层包装。
- `config` 暂时存放配置文件
- `timer` 模块(待开发，未添加)，用于无效`socket`的关闭回收
- `util` 模块为待开发模块，详见`unstable`分支

### 配置文件
- 可以在 `config` 文件夹下的 `wsx.conf` 参考详细配置格式，当前支持的配置选项只有四个
	- 详见配置文件
	- 以 `#` 开头的为注释，单行有效

- 配置文件放在 `"./wsx.conf", "/etc/wushxin/wsx.conf"`这两个地方，配置文件名字不能改变。

### 进度
- 完成总体程序框架的编写
- 基本功能完成
- 高并发时在 **response**中的`snprintf`似乎会有 Segment Fault 的错误

### TODO
- `timer`模块
- `util`模块
- **http** 请求头 : `GET POST`
- 使用新数据结构 `String` 解决发送文件过大问题。