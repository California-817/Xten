#ifndef __XTEN_DAEMON_H__
#define __XTEN_DAEMON_H__
#include"../singleton.hpp"
#include<time.h>
#include<sstream>
#include<string>
#include<unistd.h>
#include<functional>
#include"../util.h"
namespace Xten
{
    typedef std::function<int(int argc,char** argv)> main_cb;
    class ProcessInfo : public singleton<ProcessInfo>
    {
    public:
        pid_t parent_id; //父进程id
        pid_t main_id;  //子进程id
        time_t parent_start_time=0; //父进程启动时间
        time_t main_start_time=0; //子进程启动时间
        uint64_t restart_main_count=0; //子进程重启次数
        std::string ToString()
        {
            std::stringstream ss;
            ss<<" ParentId="<<parent_id<<" MainId="<<main_id<<""<<std::endl
                <<" ParentStartTime: "<<Xten::Time2Str(parent_start_time,"%a, %d %b %Y %H:%M:%S")<<std::endl
                <<" MainStartTime: "<<Xten::Time2Str(main_start_time,"%a, %d %b %Y %H:%M:%S")<<std::endl
                <<" Main Restart Count: "<<restart_main_count<<""<<std::endl;
            return ss.str();
        }
    };
    //xten框架启动函数
    int xten_start(int argc,char** argv,main_cb mainCb,bool is_daemon);
}
#endif