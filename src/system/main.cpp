#include"application.h"
#include<stdlib.h>
#include<time.h>
int main(int argc ,char** argv)
{
    setenv("TZ",":/etc/localtime",1);
    tzset();
    srand(time(0));
    Xten::Application app;
    if(app.Init(argc,argv))
    {
        //服务器框架初始化成功,运行服务器框架
        return app.Run();
    }
    return 0;
}