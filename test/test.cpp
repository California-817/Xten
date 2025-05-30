#include"../include/Xten.h"
int main()
{
    Xten::Config::LoadFromConFDir(".");
    XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello log";
    XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fmt");
    XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system";
    std::cout<<Xten::LoggerManager::GetInstance()->toYamlString()<<std::endl;
    // std::cout<<Xten::Config::GetDatas()["logs"]->ToString();
    // std::cout<<Xten::LoggerManager::GetInstance()->GetLogger("system");
    return 0;
}