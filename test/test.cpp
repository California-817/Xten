#include "../include/Xten.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
static Xten::Logger::ptr g_logger=XTEN_LOG_ROOT();
int sock=0;
void test_iomanager() //也是在协程中调用
{
    XTEN_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    // sleep(3);

    // close(sock);
    // sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int flags=fcntl(sock,F_GETFL);
    fcntl(sock, F_SETFL, O_NONBLOCK|flags);

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
            Xten::IOManager::GetThis()->CancelEvent(sock, Xten::IOManager::Event::READ);close(sock);
             });
    }
    else
    {
        XTEN_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}
int main()
{

    Xten::IOManager iom(2, false, "test");
    iom.Schedule(&test_iomanager);
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
    // Xten::Config::LoadFromConFDir(".");
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