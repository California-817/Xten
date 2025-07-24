#include "fdmanager.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
namespace Xten
{
    FdCtx::FdCtx(int fd)
        : _fd(fd),
          _isInit(false),
          _isClose(true),
          _isSocket(false),
          _isSysSetNoBlock(false),
          _isUserSetNoBlock(false),
          _readTimeOut_ms(-1),
          _writeTimeOut_ms(-1)
    {
        init();
    }
    bool FdCtx::init()
    {
        if (_isInit)
        {
            return true;
        }
        _readTimeOut_ms = -1;
        _writeTimeOut_ms = -1;
        struct stat st;
        if (fstat(_fd, &st) == -1)
        {
            // error
            _isInit = false;
            _isSocket = false;
        }
        else
        {
            _isInit = true;
            _isSocket = S_ISSOCK(st.st_mode);
        }
        if (_isSocket)
        {
            int flags = fcntl_f(_fd, F_GETFL); // 使用原始接口
            if (!(flags & O_NONBLOCK))
            {
                // 未设置非阻塞
                fcntl_f(_fd, F_SETFL, flags | O_NONBLOCK);
            }
            _isSysSetNoBlock = true;
        }
        else
        {
            _isSysSetNoBlock = false;
        }
        _isClose = false;
        _isUserSetNoBlock = false;
        return _isInit;
    }
    // 是否是socketfd
    bool FdCtx::IsSocket()
    {
        return _isSocket;
    }
    // 是否初始化
    bool FdCtx::IsInit()
    {
        return _isInit;
    }
    // 是否关闭
    bool FdCtx::IsClose()
    {
        return _isClose;
    }
    // 是否用户设置非阻塞
    bool FdCtx::GetUserNoBlock()
    {
        return _isUserSetNoBlock;
    }
    // 用户设置非阻塞
    void FdCtx::SetUserNoBlock(bool v)
    {
        _isUserSetNoBlock = v;
    }
    // 是否系统设置非阻塞
    bool FdCtx::GetSysNoBlock()
    {
        return _isSysSetNoBlock;
    }
    // 系统设置非阻塞
    void FdCtx::SetSysNoBlock(bool v)
    {
        _isSysSetNoBlock = v;
    }
    // 设置超时时间
    void FdCtx::SetTimeOut(int type, uint64_t time_ms)
    {
        // 读超时时间
        if (type == SO_RCVTIMEO)
        {
            _readTimeOut_ms = time_ms;
        }
        else // 写超时
        {
            _writeTimeOut_ms = time_ms;
        }
    }
    // 获取超时时间
    uint64_t FdCtx::GetTimeOut(int type)
    {
        // 读超时时间
        if (type == SO_RCVTIMEO)
        {
            return _readTimeOut_ms;
        }
        else // 写超时
        {
            return _writeTimeOut_ms;
        }
    }
    FdCtxMgr::FdCtxMgr()
    {
        _fds.resize(64);
    }
    FdCtx::ptr FdCtxMgr::Get(int fd, bool auto_create)
    {
        if (fd == -1)
            return nullptr;
        {
            RWMutex::ReadLock rlock(_mutex);
            if (fd >= (int)_fds.size())
            {
                if (auto_create == false)
                {
                    return nullptr;
                }
            }
            else
            {
                if (_fds[fd] || !auto_create)
                {
                    return _fds[fd];
                }
            }
        }
        {
            RWMutex::WriteLock wlock(_mutex);
            FdCtx::ptr fdctx = std::make_shared<FdCtx>(fd);
            if (fd >= (int)_fds.size())
            {
                _fds.resize(fd * 1.5);
            }
            _fds[fd] = std::move(fdctx);
        }
        return _fds[fd];
    }
    void FdCtxMgr::Del(int fd)
    {
        RWMutex::WriteLock wlock(_mutex);
        if ((fd >= (int)_fds.size()) || fd < 0)
        {
            return;
        }
        _fds[fd].reset();
    }
}