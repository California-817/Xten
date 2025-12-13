# KCP服务器架构(适用于实时游戏服务器)
## 1.消息的接收
1.服务器启动后，调度一个协程，该协程从一个udp连接中读取所有的包，这些包可能是：

1. 发起连接的包 ---这类包是裸包，没有经过kcp处理，可直接memcmp判断
2. 某个连接的数据包 ---这类包被kcp处理了，但是kcp提供了函数进行获取convid，通过这个convid就可以将包放入kcpsession进行处理[input then recv]，然后发送给逻辑层

那么这个协程要进行判断：

1. 看包的内容发现是连接建立包，创建逻辑层的kcpsession并放入smap管理，并启动一个写协程，写协程发送数据
2. 根据smap进行查找，找到某个连接后，直接将这个连接和对应的数据包发送到逻辑处理单元进行数据处理

## 2.回调函数调用
每个连接在连接建立和销毁的时候都需要进行一些操作，在kcpsession中注册两个回调函数，并且保证回调在合适的时候调用，并且只可以调用一次，由于连接不存在唯一的读协程，而对应有一个写协程，那么可以由写协程在开始和结束阶段进行回调函数调用

超时回调函数：定时器超时的时候执行

## 3.消息处理
编写一个logicsystem，这个worker类内包含若干个协程，每个协程都有自己的任务队列，任务来自前面的读协程，并且为了保证处理的顺序性，使用客户端ip进行hash，保证同一个连接的数据在一个协程中顺序处理，处理后即可发送数据

## 4.数据发送
每个kcpsession对应一个发送队列和写协程，写协程的作用是将逻辑层投入的响应包放入kcpcb的发送队列即可[ send ]，单协程发送保证正确，
同时，在放入队列的时候那个真正发送协程可能会和写协程产生数据竞争，所有最好使用一把锁保证互斥

## 5.真正的发送函数
服务器启动后，再调度一个协程，这个协程while(usleep(10ms)) 循环调用每个kcp的update函数，将包文从kcpcb中真正发送到内核中。

## 6.超时处理
有两种方案，一是每个连接一个定时器，二是借用真正发送协程进行判断，但是数据更新和真正发送函数不一定在一个线程

1. 连接建立后启动定时器，接收协程在每次分派包文的时候取消并更新定时器
2. 考虑到accept调度器一般只有一个线程，我认为使用方案二更简单，更新时间戳在接收协程，判断超时在发送协程，两者在一个线程，不需要加锁

## 7.连接管理
server中增加成员sessionmap管理所有已经建立的连接，写协程开始时加入，写协程终止时移除。

## 8.接收协程优化---减少udp接收系统调用开销
```c
#include <netinet/in.h>
#include <sys/socket.h>
#define BATCH   256
struct mmsghdr  msgs[BATCH];
struct iovec    iovs[BATCH];
char            bufs[BATCH][MTU];

void prep_buffers() {
    for (int i = 0; i < BATCH; ++i) {
        iovs[i].iov_base = bufs[i];
        iovs[i].iov_len  = MTU;
        msgs[i].msg_hdr.msg_iov     = &iovs[i];
        msgs[i].msg_hdr.msg_iovlen  = 1;
        msgs[i].msg_hdr.msg_name    = &addrs[i];
        msgs[i].msg_hdr.msg_namelen = sizeof(struct sockaddr_in);
    }
}

int recv_batch(int fd) {
    int ret = recvmmsg(fd, msgs, BATCH, 0, nullptr);
    for (int i = 0; i < ret; ++i) {
        dispatch(bufs[i], msgs[i].msg_len,   // 用户态无锁分派
                 (struct sockaddr_in *)msgs[i].msg_hdr.msg_name);
    }
    return ret;
}

```