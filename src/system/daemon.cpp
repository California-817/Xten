#include "daemon.h"
#include "../log.h"
#include "../config.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include"start_show.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static ConfigVar<int64_t>::ptr g_daemon_core =
        Config::LookUp("daemon.core", (int64_t)-1, "daemon core size");
    static ConfigVar<int32_t>::ptr g_daemon_restart_interVal =
        Config::LookUp("daemon.restart.interval", (int32_t)5, "daemon restart interval");
    // 非daemon启动函数
    static int off_daemon(int argc, char **argv, main_cb mainCb)
    {
        ProcessInfo::GetInstance()->main_id = getpid();
        ProcessInfo::GetInstance()->main_start_time = time(0);
        return mainCb(argc, argv);
    }
    // 设置核心转储文件大小
    static void ulimitc(const rlim_t &s)
    {
        struct rlimit limit;
        limit.rlim_max = limit.rlim_cur = s;
        setrlimit(RLIMIT_CORE, &limit);
    }
    // daemon启动函数
    static int on_daemon(int argc, char **argv, main_cb mainCb)
    {
        // daemon(1, 0);
        ProcessInfo::GetInstance()->parent_id = getpid();
        ProcessInfo::GetInstance()->parent_start_time = time(0);
        while (true)
        {
            if (ProcessInfo::GetInstance()->restart_main_count == 0)
            {
                ulimitc(g_daemon_core->GetValue());
            }
            else
            {
                ulimitc(0);
            }
            // 创建子进程执行主体逻辑
            pid_t ret = fork();
            if (ret == 0)
            {
                // 子进程(可以看到与父进程一样的ProcessInfo,不是同一个ProcessInfo,设置值后对父进程不可见)
                ProcessInfo::GetInstance()->main_id = getpid();
                ProcessInfo::GetInstance()->main_start_time = time(0);
                //子进程真正执行框架代码
                return mainCb(argc, argv);
            }
            else if (ret > 0)
            {
                // 父进程
                int status = 0;
                // 阻塞等待子进程退出
                waitpid(ret, &status, 0);
                if (status == 9)
                {
                    // 被 kill -9 杀死
                    XTEN_LOG_INFO(g_logger) << "Kill -9 Main Process, pid=" << ret;
                    break;
                }
                else if (status == 0)
                {
                    // 正常退出
                    XTEN_LOG_INFO(g_logger) << "Main Process Success Finish, pid=" << ret;
                    break;
                }
                else
                {
                    // 异常退出
                    XTEN_LOG_ERROR(g_logger) << "Main Process Crash, pid=" << ret
                                             << " status=" << status;
                    // 增加重启次数
                    ProcessInfo::GetInstance()->restart_main_count++;
                    // 间隔一段时间再重启子进程(保证资源已经释放)
                    sleep(g_daemon_restart_interVal->GetValue());
                }
            }
            else
            {
                // fork error
                XTEN_LOG_ERROR(g_logger) << "Fork child process failed";
                return -1;
            }
        }
        // 父进程退出
        return 0;
    }
    // xten框架启动函数
    int xten_start(int argc, char **argv, main_cb mainCb, bool is_daemon)
    {
        // 展示启动界面
        Xten::StartShow::showBanner();
        if (!is_daemon)
        {
            // 非守护进程启动
            ProcessInfo::GetInstance()->parent_id = getpid();
            ProcessInfo::GetInstance()->parent_start_time = time(0);
            return off_daemon(argc, argv, mainCb);
        }
        else
        {
            return on_daemon(argc, argv, mainCb);
        }
    }
}