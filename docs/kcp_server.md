# KCP服务器架构(适用于实时游戏服务器)
注意：在任何时刻，对同一个kcpcb的处理不能并发进行，也就是说不管是对kcpcb的input，recv，send，update接口的调用都需要加协程锁====>这个认识非常重要
## 1.KcpListener对象
在kcp服务器中，由于udp是无连接的，所以服务端其实只有一个socket来接收发送来自所有客户端的数据包，因此抽象出一个listener来进行数据收发
### 1.连接创建
listener仿照tcp的实现，进行连接的建立并返回连接，在listener内部有新连接队列和所有所有连接的map，当从socket中读取一个udp包文的时候，先通过fromip从连接map中查找，
看是不是已经建立的连接，如果不是，则判断是不是一个kcp连接握手包文(kcp_util.h中定义)，是则创建一个KcpSession并放到新连接队列中，等待上层获取连接，上层对内部的行为是无感知的，上层编程方式类似tcp
### 2.数据收发
整个服务器的数据收发工作实际上就是由listener内部的协程进行的，可通过配置更改内部协程数量，同时，为了保证每一个协程都能够进行数据收发提高并发性能，

优化一：引入了SO_REUSEPORT标记，并且listener的sessionmap需要使用socketfd进行所有session的分类
```c
//UDP是支持多线程并发读取发送的：在 Linux 中，SO_REUSEPORT 选项允许多个套接字绑定到相同的 IP 地址和端口，内核会自动将数据包负载均衡到不同的套接字（线程）上，从而实现高效的多挟持读取 UDP 数据。
    int sockfds[THREAD_NUM];
    struct sockaddr_in server_addr;
    int opt = 1;
    // 创建多个套接字，绑定到同一端口
    for (int i = 0; i < THREAD_NUM; i++) {
        sockfds[i] = socket(AF_INET, SOCK_DGRAM, 0);
        // 设置 SO_REUSEPORT 选项
        setsockopt(sockfds[i], SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0;
        // 绑定到同一端口
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);
        bind(sockfds[i], (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0;
    }
```
优化二：每个recv协程进行socket的recv时，使用recvmmsg这个系统调用接口，一次调用批量获取udp包文，优化在高并发场景下的性能
```c
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
当收到包文是已经建立的连接的包文时，通过包文的fromip找到session，同时获取包文的convid和session的convid进行匹配，成功就调用session的inputpacket接口将包文放到
session的ikcpcb的接收缓冲区中【调用ikcp_input接口】
### 3.数据发送
数据的发送由listener内部的send协程进行，每个协程定期调用属于自己socket通道的所有session的update接口【对ikcp_update接口进行封装】
### 4.accept
上层需要用到的接口就是accept接口，即从连接队列中获取一个KcpSession对象，然后再对这个对象进行处理即可
### 5.与KcpSession的交互
第一，要知道，真正进行数据收发的是listener而不是session，那么listener在收发的时候遇到错误的时候要通知session，因此在read错误的时候，调用notify接口对map中所有sessions进行通知错误，此时所有session就都能知道了

第二，当上层sessions关闭的时候，要同时listener这个连接关闭了，否则listener仍然会将udp包文发给这个session，因此在session的close方法中将map中自己给移除
## 2.KcpSession对象
KcpSession的使用实际上和普通的tcp连接一致，提供recvmsg方法，sendmsg方法，forceclose方法，close方法，startsender方法

recvmsg：ikcpcb中获取一个udp包文【ikcp_recv方法】，阻塞获取，首先加锁进行ikcpcb的访问，然后调用ikcp_peeksize方法看是不是有包文，如果没有就在条件变量上挂起，等listener的接收协程收到包后唤醒即可，然后进行数据反序列化返回，为了捕获超时，读错误，连接关闭等错误，使用输入参数code进行判断，这样的好处就是当读超时的时候返回，可以进行判断并调用超时处理函数，上层不需要进行额外的复杂处理

sendmsg：和wssession的设计一样，每个session都启动一个发送协程，sendmsg接口只是将包文发送到发送队列，然后由这个发送协程顺序且单独发送，保证上层调用这个接口发送的时候的顺序性和互斥性，这个发送协程将队列中的包文发送到ikcpcb的发送缓冲区中【ikcp_send接口】

forceclose：发送一个nullptr包文，发送协程接收后直接退出停止发送即可，然后调用close

close：close方法首先需要对自己ikcpcb发送缓冲区中的包文进行刷新操作，防止包文没有发送完就关闭，然后通知listener删除这个连接

startsender：启动发送协程
## 3.msghandler消息处理
编写一个logicsystem，这个worker类内包含若干个协程，每个协程都有自己的任务队列，任务来自前面的读协程，并且为了保证处理的顺序性，使用客户端ip进行hash，保证同一个连接的数据在一个协程中顺序处理，处理后即可发送数据

逻辑处理类【任务处理协程池】的设计(参考zinx的设计)：
1. hash：fiberpool中worker数量确定，上层使用轮询的方式分配任务

    缺点：连接a的处理会影响连接b的处理，不适用于阻塞场景，任务很多的时候处理不过来【惊群现象】

    优点：worker利用率高，负载均衡

    场景：业务处理快速、无阻塞的场景【如redis这种场景】
2. bind：每个连接绑定一个worker协程，根据配置的maxconn直接创建worker
    缺点：worker利用率低，可能会创建大量worker，在连接数量波动的时候尤其明显

    优点：连接间的处理互不影响，连接a阻塞操作不影响连接b的处理，每个连接队列大小可以适度减少【只处理一个连接】

    场景：连接数稳定、业务处理可能阻塞的场景

3. dynamicbind：有两组worker，freeworker固定数量且一直存在，externalworker是当freeworker不足的时候创建，连接断开后自动销毁

    缺点：实现复杂度高，频繁创建销毁worker开销较大【优化：对象内存池技术】

    优点：连接间互不影响，按需创建，不会造成资源大类浪费

    场景：连接数波动大、希望平衡性能和资源的场景

## 6.超时处理
通过recvmsg时，传入参数获取的值进行判断超时并调用超时处理函数

## 7.连接管理
server中增加成员sessionmap管理所有已经建立的连接，写协程开始时加入，写协程终止时移除。通过这个连接管理就能进行session交互，

session->getServer()->getcontainer()->sendmsg(id,msg)  //将消息发送给服务器中的某个session

服务器会进行连接最大个数的限制：maxConn

