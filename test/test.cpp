#include"../include/log.h"
int main()
{
    XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello log";
    XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fmt");
    return 0;
}