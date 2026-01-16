#include "../include/Xten.h"
#include "../src/kcp/kcp_util.hpp"
#include "../src/kcp/third_part/ikcp.h"
#include "../src/kcp/kcp_protocol.h"
#include <thread>
using namespace Xten;
using namespace Xten::kcp;

// kcp_client.cpp
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
static int count = 0;
static int g_sock = 0;
static sockaddr_in g_serv;
static std::mutex g_kcp_mtx;

/* ------------ KCP 输出回调 ------------ */
int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    return sendto(g_sock, buf, len, 0, (sockaddr *)&g_serv, sizeof(g_serv));
}

/* ------------ 获取毫秒时间 ------------ */
uint32_t get_ms()
{
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1'000'000;
}

/* ------------ 后台更新线程 ------------ */
void update_thread(ikcpcb *kcp)
{
    while (true)
    {
        {
            std::lock_guard<std::mutex> lk(g_kcp_mtx);
            ikcp_update(kcp, get_ms());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
static std::atomic_int sn=1;
/* ------------ 接收线程 ------------ */
void recv_thread(ikcpcb *kcp)
{
    char buf[4096];
    char rcvbuf[4096];
    while (true)
    {
        ssize_t n = recvfrom(g_sock, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n > 0) {
            std::lock_guard<std::mutex> lk(g_kcp_mtx);
            ikcp_input(kcp, buf, n);
        }
        // recv
        int ret = 0;
        {
            std::lock_guard<std::mutex> lk(g_kcp_mtx);
            ret = ikcp_recv(kcp, rcvbuf, 4096);
        }
        if (ret > 0)
        {
            // rcvbuf[ret] = 0;
            // ByteArray::ptr ba = std::make_shared<ByteArray>(ret);
            // ba->Write(rcvbuf, ret);
            // ba->SetPosition(0);
            // // KcpResponse::ptr rsp = std::make_shared<KcpResponse>();
            // // std::cout<<ba->ReadFUint8()<<std::endl; //type
            // // rsp->ParseFromByteArray(ba);
            // if(sn>rsp->GetSn())
            // {
            //     std::cout<<"error"<<std::endl;
            //     exit(-1);
            // }
            // sn=rsp->GetSn();
            // std::cout << rsp->ToString() << std::endl;
        }
    }
}

/* ------------ main ------------ */
int main(int argc, char *argv[])
{
    // if (argc != 3)
    // {
    // std::cerr << "用法: " << argv[0] << " <ip> <port>\n";
    // return 1;
    // }

    /* 1. UDP socket */
    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    g_serv.sin_family = AF_INET;
    g_serv.sin_port = htons(8008);
    inet_pton(AF_INET, "127.0.0.1", &g_serv.sin_addr);

    sendto(g_sock, KcpUtil::making_connect_packet().c_str(), KcpUtil::making_connect_packet().length(), 0, (const sockaddr *)&g_serv, sizeof(g_serv));
    char buffer[1024];
    //     IPv4Address::ptr from = IPv4Address::Create("0.0.0.0", 0);
    //     sock->RecvFrom(buffer, 1023, from);
    int ret = recvfrom(g_sock, buffer, 1023, 0, nullptr, nullptr);
    buffer[ret] = 0;
    std::cout << "ret=" << buffer << std::endl;
    auto convid = KcpUtil::get_connect_backpacket_convid(buffer);

    /* 2. KCP 客户端会话（conv 随机） */
    uint32_t conv = rand() & 0x7FFFFFFF;
    ikcpcb *kcp = ikcp_create(convid, nullptr);
    ikcp_setoutput(kcp, kcp_output);
    ikcp_nodelay(kcp, 1, 10, 2, 1);
    ikcp_wndsize(kcp, 128, 128);

    /* 3. 启动后台线程 */
    std::thread th_up(update_thread, kcp);
    std::thread th_in(recv_thread, kcp);

    /* 4. 主循环：读键盘 → 发送 */
    std::string line;
    while (true)
    {
        // ByteArray::ptr ba = std::make_shared<ByteArray>();
        // char buffer[1024];
        // std::string str = "send kcp message,id=";
        // str += std::to_string(count++);
        // KcpRequest::ptr req = std::make_shared<KcpRequest>();
        // req->SetSn(count);
        // req->SetCmd(0);
        // req->SetData(str);
        // // ba->SetPosition(0);
        // // ba->SetSize(0);
        // req->SerializeToByteArray(ba);
        // ba->SetPosition(0);
        // auto reqstr = ba->ToString();
        // std::cout<<reqstr.length()<<std::endl;
        // {
        //     std::lock_guard<std::mutex> lk(g_kcp_mtx);
        //     ikcp_send(kcp, reqstr.c_str(), reqstr.length());
        //     // ikcp_send(kcp, line.data(), line.size());
        //     ikcp_flush(kcp); // 立即尝试发出
        // }
        usleep(10000);
    }

    ikcp_release(kcp);
    close(g_sock);
    th_up.detach();
    th_in.detach();
    return 0;
}

// Socket::ptr sock=nullptr;
// Address::ptr to = nullptr;
// int output(const char *buf, int len,
//            ikcpcb *kcp, void *user)
// {
//     // Address::ptr to=*(Address::ptr*)user;
//     // send
//     // Socket::ptr* sock = static_cast<Socket::ptr *>(user);
//     sock->SendTo(buf, len, to);
// }
// int main()
// {
//     to = IPv4Address::Create("127.0.0.1", 8080);
//     sock = Socket::CreateUDPSocket();
//     sock->SendTo(KcpUtil::making_connect_packet().c_str(), KcpUtil::making_connect_packet().length(), to);
//     char buffer[1024];
//     IPv4Address::ptr from = IPv4Address::Create("0.0.0.0", 0);
//     sock->RecvFrom(buffer, 1023, from);
//     std::cout << "ret=" << buffer << std::endl;
//     auto convid = KcpUtil::get_connect_backpacket_convid(buffer);
//     IKCPCB *kcpcb = ikcp_create(convid, nullptr);
//     // ikcp_setmtu(kcpcb,10);
//     ikcp_setoutput(kcpcb, output);
//     // ikcp_nodelay(kcpcb, 1, 10, 2, 1);
//     std::thread th([kcpcb]()
//                    { while(true){ikcp_update(kcpcb, TimeUitl::GetCurrentMS()); usleep(10000);} });
//     // th.detach();
//     while (true)
//     {
//         try
//         {
//             ByteArray::ptr ba = std::make_shared<ByteArray>();
//             char buffer[1024];
//             std::string str = "send kcp message,id=";
//             str += std::to_string(count++);
//             KcpRequest::ptr req = std::make_shared<KcpRequest>();
//             req->SetSn(count);
//             req->SetCmd(0);
//             req->SetData(str);
//             // ba->SetPosition(0);
//             // ba->SetSize(0);
//             req->SerializeToByteArray(ba);
//             auto reqstr = ba->ToString();
//             ikcp_send(kcpcb, reqstr.c_str(), reqstr.length());
//             // ikcp_update(kcpcb, TimeUitl::GetCurrentMS());
//             // ... rest of loop ...
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Exception in loop: " << e.what() << std::endl;
//             break;
//         }
//         catch (...)
//         {
//             std::cerr << "Unknown exception in loop" << std::endl;
//             break;
//         }
//         std::cout << "send" << std::endl;
//         usleep(100000);
//     }

// iom.Schedule([&]()
//              {
//             while(true){
//                     int ret = sock->RecvFrom(buffer, 1023, from);
// ikcp_input(kcpcb, buffer, ret);
//         ByteArray::ptr ba = std::make_shared<ByteArray>();
// int ret2 = ikcp_recv(kcpcb, (char *)ba->GetBeginNodePtr(), ba->GetNodeSize());
// ba->SetSize(ret2);
// KcpResponse::ptr rsp = std::make_shared<KcpResponse>();
// rsp->ParseFromByteArray(ba);
// // recvbuf[ret2] = 0;
// std::cout << rsp->ToString() << std::endl;} });

// return 0;
// }