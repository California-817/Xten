#include"../include/Xten.h"
int main()
{
    std::cout<<"main"<<std::endl;
    Xten::Thread::SetName("main_thread");
    Xten::Fiber::GetThis();
    Xten::Config::LoadFromConFDir(".");
    XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello log";
    XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fmt");
    XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system";
    Xten::Fiber::ptr fiber(Xten::NewFiber(0,
        [](){
            Xten::Thread::SetName("fiber_thread");
            XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello fiber log";
            XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fiber fmt");
            XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber";
            Xten::Fiber::GetThis()->Back();
            XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber222222";
        }
        ,false
    ));
    fiber->Call();
    std::cout<<"retrun"<<std::endl;
    fiber->Call();
    std::cout<<"retrun again"<<std::endl;
    // std::cout<<Xten::LoggerManager::GetInstance()->GetLogger("system");
    return 0;
}