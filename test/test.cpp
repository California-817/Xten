#include"../include/Xten.h"
Xten::Timer::ptr tm=nullptr;
int main()
{
    Xten::TimerManager mgr;
        XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add";

    tm=mgr.AddTimer(5000,[&mgr](){
        XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"hello";
        mgr.AddTimer(3000,[](){
            XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add other timer";

        },false);
    },true);
    int i=0;
    while(true)
    {
       usleep(10000);
        std::vector<std::function<void()>> cbs;
        mgr.ListAllExpireCb(cbs);
        for(auto& e:cbs)
        {
            e();
        }
    }
    // std::cout<<"main"<<std::endl;
    // Xten::Thread::SetName("main_thread");
    // Xten::Fiber::GetThis();
    Xten::Config::LoadFromConFDir(".");
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