#ifndef __XTEN_FDMANAGER_H__
#define __XTEN_FDMANAGER_H__
#include <memory>
#include <vector>
#include "mutex.h"
#include "singleton.hpp"
namespace Xten
{
    class FdCtx : public std::enable_shared_from_this<FdCtx>
    {
    public:
        typedef std::shared_ptr<FdCtx> ptr;
        FdCtx(int fd);
        ~FdCtx() = default;
        // 是否是socketfd
        bool IsSocket();
        // 是否初始化
        bool IsInit();
        // 是否关闭
        bool IsClose();
        // 是否用户设置非阻塞
        bool GetUserNoBlock();
        // 用户设置非阻塞
        void SetUserNoBlock(bool v);
        // 是否系统设置非阻塞
        bool GetSysNoBlock();
        // 系统设置非阻塞
        void SetSysNoBlock(bool v);
        // 设置超时时间
        void SetTimeOut(int type, uint64_t time_ms);
        // 获取超时时间
        uint64_t GetTimeOut(int type);
        bool init();

    private:
        bool _isInit;              // 是否初始化
        bool _isSocket;            // 是否是socket
        bool _isClose;             // 是否关闭
        bool _isUserSetNoBlock;    // 是否用户设置非阻塞
        bool _isSysSetNoBlock;     // 是否系统设置非阻塞
        int _fd;                   // fd
        uint64_t _readTimeOut_ms;  // 读超时时间
        uint64_t _writeTimeOut_ms; // 写超时时间
    };
    class FdCtxMgr : public singleton<FdCtxMgr>
    {
    public:
        FdCtxMgr();
        //获取 不存在则创建
        FdCtx::ptr Get(int fd,bool auto_create=false);
        //删除fdctx
        void Del(int fd);
        ~FdCtxMgr()=default;
    private:
        RWMutex _mutex;
        std::vector<FdCtx::ptr> _fds;
    };
}
#endif