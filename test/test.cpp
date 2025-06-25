#include "../include/Xten.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
static Xten::Logger::ptr g_logger = XTEN_LOG_ROOT();
int sock = 0;
void test_iomanager() // 也是在协程中调用
{
    XTEN_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    // sleep(3);

    // close(sock);
    // sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, O_NONBLOCK | flags);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr *)&addr, sizeof(addr)))
    {
    }
    else if (errno == EINPROGRESS)
    {
        XTEN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        Xten::IOManager::GetThis()->AddEvent(sock, Xten::IOManager::Event::READ, []()
                                             { XTEN_LOG_INFO(g_logger) << "read callback"; });
        Xten::IOManager::GetThis()->AddEvent(sock, Xten::IOManager::Event::WRITE, []()
                                             {
            XTEN_LOG_INFO(g_logger) << "write callback";
            Xten::IOManager::GetThis()->CancelEvent(sock, Xten::IOManager::Event::READ);close(sock); });
    }
    else
    {
        XTEN_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}
void test_hook()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    XTEN_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    XTEN_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if (rt)
    {
        return;
    }

    const char data[] = "GET /register HTTP/1.1\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    XTEN_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    XTEN_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    buff.resize(rt);
    XTEN_LOG_INFO(g_logger) << buff;
}
void test_socket()
{
    Xten::IPAddress::ptr addr = Xten::Address::LookupAnyIPAddress("www.baidu.com:80");
    if (addr)
    {
        XTEN_LOG_DEBUG(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        XTEN_LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    Xten::Socket::ptr sock = Xten::Socket::CreateTCP(addr);
    if (!sock->Connect(addr))
    {
        XTEN_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    }
    else
    {
        XTEN_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = Xten::TimeUitl::GetCurrentMS();
    for (size_t i = 0; i < 10000000000ul; ++i)
    {
        if (int err = sock->GetError())
        {
            XTEN_LOG_INFO(g_logger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        // struct tcp_info tcp_info;
        // if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //     SYLAR_LOG_INFO(g_looger) << "err";
        //     break;
        // }
        // if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //     SYLAR_LOG_INFO(g_looger)
        //             << " state=" << (int)tcp_info.tcpi_state;
        //     break;
        // }
        static int batch = 10000000;
        if (i && (i % batch) == 0)
        {
            uint64_t ts2 = Xten::TimeUitl::GetCurrentMS();
            XTEN_LOG_INFO(g_logger) << "10000000 use time:" << (ts2 - ts) << " ms";
            XTEN_LOG_INFO(g_logger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " ms";
            ts = ts2;
            break;
        }
    }
}
void test_socket2()
{
    Xten::IPAddress::ptr addr = Xten::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr)
    {
        XTEN_LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        XTEN_LOG_ERROR(g_logger) << "get address fail";
        return;
    }
    int i = 0;
    while (true)
    {
        if ((i++) == 50)
        {
            break;
        }
        sleep(2);
        Xten::Socket::ptr sock = Xten::Socket::CreateTCP(addr);
        addr->setPort(80);
        XTEN_LOG_INFO(g_logger) << "addr=" << addr->toString();
        if (!sock->Connect(addr))
        {
            XTEN_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
            return;
        }
        else
        {
            XTEN_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
        }

        const char buff[] = "GET / HTTP/1.0\r\n\r\n";
        int rt = sock->Send(buff, sizeof(buff));
        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "send fail rt=" << rt;
            return;
        }

        std::string buffs;
        buffs.resize(4096);
        rt = sock->Recv(&buffs[0], buffs.size());

        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "recv fail rt=" << rt;
            return;
        }

        buffs.resize(rt);
        XTEN_LOG_INFO(g_logger) << buffs;
    }
}
void handle_client(Xten::Socket::ptr client)
{
    while (true)
    {
        std::string buffs;
        buffs.resize(4096);
        int rt = client->Recv(&buffs[0], buffs.size());
        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "recv fail rt=" << rt;
            return;
        }
        // std::cout << "client:: " << buffs << std::endl;
        int i=0;
        char ret[1024];
        snprintf(ret,1024,"server recv:%d",i);
        int rt2 = client->Send(ret,1024);
        if (rt2 <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "send fail rt=" << rt;
            return;
        }
    }
}
void test_server()
{
    Xten::Socket::ptr apt = Xten::Socket::CreateTCPSocket();
    auto addr = Xten::IPv4Address::Create("127.0.0.1", 8080);
    apt->Bind(addr);
    apt->Listen();
    while (true)
    {
        Xten::Socket::ptr client = apt->Accept();
        if(!client)
        {
            return;
        }
        Xten::Scheduler::GetThis()->Schedule(std::bind(&handle_client, client));
    }
}
int main()
{
    // Xten::Config::LoadFromConFDir(".");
    Xten::IOManager iom(2

                        ,
                        false, "test");
    iom.Schedule(&test_server);
    // Xten::TimerManager mgr;
    //     XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add";

    // tm=mgr.AddTimer(5000,[&mgr](){
    //     XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"hello";
    //     mgr.AddTimer(3000,[](){
    //         XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add other timer";

    //     },false);
    // },true);
    // int i=0;
    // while(true)
    // {
    //    usleep(10000);
    //     std::vector<std::function<void()>> cbs;
    //     mgr.ListAllExpireCb(cbs);
    //     for(auto& e:cbs)
    //     {
    //         e();
    //     }
    // }
    // std::cout<<"main"<<std::endl;
    // Xten::Thread::SetName("main_thread");
    // Xten::Fiber::GetThis();

    // XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello log";
    // XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fmt");
    // XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system";
    // Xten::Fiber::ptr fiber(Xten::NewFiber(0,
    //     [](){
    //         Xten::Thread::SetName("fiber_thread");
    //         XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello fiber log";
    //         XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fiber fmt");
    //         XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber";
    //         Xten::Fiber::GetThis()->Back();
    //         XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber222222";
    //     }
    //     ,true
    // ),Xten::FreeFiber);
    // fiber->Call();
    // std::cout<<"retrun"<<std::endl;
    // fiber->Call();
    // std::cout<<"retrun again"<<std::endl;
    // std::cout<<Xten::LoggerManager::GetInstance()->GetLogger("system");
    return 0;
}