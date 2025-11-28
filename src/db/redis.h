#ifndef __XTEN_DB_REDIS_H__
#define __XTEN_DB_REDIS_H__

#include <stdlib.h>
#include <hiredis_cluster/hircluster.h>
#include <hiredis_cluster/adapters/libevent.h>
#include <sys/time.h>
#include <string>
#include <memory>
#include "../mutex.h"
#include "fox_thread.h"
#include "../singleton.hpp"

namespace Xten {

typedef std::shared_ptr<redisReply> ReplyPtr;
//定义基类的redis接口
class IRedis {
public:
    enum Type { //类型
        REDIS = 1,
        REDIS_CLUSTER = 2,
        FOX_REDIS = 3,
        FOX_REDIS_CLUSTER = 4
    };
    typedef std::shared_ptr<IRedis> ptr;
    IRedis() : m_logEnable(true) { }
    virtual ~IRedis() {}

    virtual ReplyPtr cmd(const char* fmt, ...) = 0;
    virtual ReplyPtr cmd(const char* fmt, va_list ap) = 0;
    virtual ReplyPtr cmd(const std::vector<std::string>& argv) = 0;

    const std::string& getName() const { return m_name;}
    void setName(const std::string& v) { m_name = v;}

    const std::string& getPasswd() const { return m_passwd;}
    void setPasswd(const std::string& v) { m_passwd = v;}

    Type getType() const { return m_type;}
protected:
    std::string m_name;  //redis对象name
    std::string m_passwd; //密码
    Type m_type;
    bool m_logEnable; //是否日志输出，当redis命令错误时输出
};
//同步的redis连接接口实现
class ISyncRedis : public IRedis {
public:
    typedef std::shared_ptr<ISyncRedis> ptr;
    virtual ~ISyncRedis() {}

    virtual bool reconnect() = 0;
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0) = 0;
    virtual bool connect() = 0;
    virtual bool setTimeout(uint64_t ms) = 0;

    virtual int appendCmd(const char* fmt, ...) = 0;
    virtual int appendCmd(const char* fmt, va_list ap) = 0;
    virtual int appendCmd(const std::vector<std::string>& argv) = 0;

    virtual ReplyPtr getReply() = 0;

    uint64_t getLastActiveTime() const { return m_lastActiveTime;}
    void setLastActiveTime(uint64_t v) { m_lastActiveTime = v;}

protected:
    uint64_t m_lastActiveTime; //上次操作时间
};


//同步的redis连接对象
class Redis : public ISyncRedis {
public:
    typedef std::shared_ptr<Redis> ptr;
    //初始化字段
    Redis();
    Redis(const std::map<std::string, std::string>& conf);
    //连接/重连接口
    virtual bool reconnect();
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0);
    virtual bool connect();
    //设置命令执行超时时间
    virtual bool setTimeout(uint64_t ms);
    //上层进行redis请求的接口
    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    virtual int appendCmd(const char* fmt, ...);
    virtual int appendCmd(const char* fmt, va_list ap);
    virtual int appendCmd(const std::vector<std::string>& argv);
    //获取一个响应
    virtual ReplyPtr getReply();
private:
    std::string m_host;  //服务器ip
    uint32_t m_port;
    uint32_t m_connectMs;  //连接超时时间
    struct timeval m_cmdTimeout; //命令操作超时时间
    std::shared_ptr<redisContext> m_context;  //hiredis封装的同步连接上下文
};


//异步redis客户端的实现 [内部有一个线程进行事件循环+异步redis连接上下文结构，上层发送命令只是将命令入线程队列]
class FoxRedis : public IRedis {
public:
    typedef std::shared_ptr<FoxRedis> ptr;
    enum STATUS {  //状态
        UNCONNECTED = 0, 
        CONNECTING = 1,  //在 init 到 onconnectCb 调用期间处于这种状态
        CONNECTED = 2
    };
    enum RESULT { //结果描述码
        OK = 0,
        TIME_OUT = 1,
        CONNECT_ERR = 2,
        CMD_ERR = 3,
        REPLY_NULL = 4,
        REPLY_ERR = 5,
        INIT_ERR = 6
    };

    FoxRedis(Xten::FoxThread* thr, const std::map<std::string, std::string>& conf);
    ~FoxRedis();

    //上层进行redis请求的接口
    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    //建立连接并创建异步redis上下文，与libevent进行连接，注册2min定时ping事件保活[内部由foxthread完成]
    bool init();
    //获取当前请求数量
    int getCtxCount() const { return m_ctxCount;}
private:
    //用户认证回调函数---判断认证结果并输出日志
    static void OnAuthCb(redisAsyncContext* c, void* rp, void* priv);
private:
    //上层协程发起请求的请求上下文结构 [协程与foxThread交互的结构]
    struct FCtx {
        std::string cmd; //请求命令
        Xten::Scheduler* scheduler; //协程调度器
        Xten::Fiber::ptr fiber; //协程
        ReplyPtr rpy; //响应结果
    };
    
    //foxthread线程发起异步请求函数中，创建该ctx并且在收到响应的回调函数中删除
    struct Ctx {
        typedef std::shared_ptr<Ctx> ptr;

        event* ev; //超时事件
        bool timeout; //是否超时
        FoxRedis* rds; //当前redis
        std::string cmd; //命令
        FCtx* fctx; 

        FoxThread* thread; //foxthread

        Ctx(FoxRedis* rds);
        ~Ctx();
        //创建命令超时定时事件
        bool init();
        //超时回调函数---->标记超时同时调度协程
        static void EventCb(int fd, short event, void* d);
    };
private:
    //foxthread收到一个上层请求就需要执行一次这个函数，收到上层请求对应需要执行的函数
    virtual void pcmd(FCtx* ctx);
    //foxthread执行的init函数
    bool pinit();
    //no used
    void delayDelete(redisAsyncContext* c);
private:
    //连接建立回调---发起异步redis请求进行auth用户认证
    static void ConnectCb(const redisAsyncContext* c, int status);
    //连接断开回调---输出日志并设置状态未建立
    static void DisconnectCb(const redisAsyncContext* c, int status);
    //执行命令的回调函数-----foxthread进行异步redis请求后，得到响应后执行的回调函数[设置响应并重新调度协程]
    static void CmdCb(redisAsyncContext *c, void *r, void *privdata);
    //定时器回调-----定期发送ping心跳包给redis服务器进行保活
    static void TimeCb(int fd, short event, void* d);
private:
    Xten::FoxThread* m_thread;    //异步执行所有上层派发的redis命令和定期保活操作
    std::shared_ptr<redisAsyncContext> m_context; //hiredis实现的异步redis连接上下文结构
    std::string m_host;  //主机
    uint16_t m_port; //port
    STATUS m_status; //状态
    int m_ctxCount; //当前redis请求数量

    struct timeval m_cmdTimeout; //命令超时时间
    std::string m_err;
    struct event* m_event; //定时事件---ping
};



class RedisCluster : public ISyncRedis {
public:
    typedef std::shared_ptr<RedisCluster> ptr;
    RedisCluster();
    RedisCluster(const std::map<std::string, std::string>& conf);

    virtual bool reconnect();
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0);
    virtual bool connect();
    virtual bool setTimeout(uint64_t ms);

    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    virtual int appendCmd(const char* fmt, ...);
    virtual int appendCmd(const char* fmt, va_list ap);
    virtual int appendCmd(const std::vector<std::string>& argv);

    virtual ReplyPtr getReply();
private:
    std::string m_host;
    uint32_t m_port;
    uint32_t m_connectMs;
    struct timeval m_cmdTimeout;
    std::shared_ptr<redisClusterContext> m_context;
};


class FoxRedisCluster : public IRedis {
public:
    typedef std::shared_ptr<FoxRedisCluster> ptr;
    enum STATUS {
        UNCONNECTED = 0,
        CONNECTING = 1,
        CONNECTED = 2
    };
    enum RESULT {
        OK = 0,
        TIME_OUT = 1,
        CONNECT_ERR = 2,
        CMD_ERR = 3,
        REPLY_NULL = 4,
        REPLY_ERR = 5,
        INIT_ERR = 6
    };

    FoxRedisCluster(Xten::FoxThread* thr, const std::map<std::string, std::string>& conf);
    ~FoxRedisCluster();

    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    int getCtxCount() const { return m_ctxCount;}

    bool init();
private:
    struct FCtx {
        std::string cmd;
        Xten::Scheduler* scheduler;
        Xten::Fiber::ptr fiber;
        ReplyPtr rpy;
    };
    struct Ctx {
        typedef std::shared_ptr<Ctx> ptr;

        event* ev;
        bool timeout;
        FoxRedisCluster* rds;
        FCtx* fctx;
        std::string cmd;

        FoxThread* thread;

        void cancelEvent();  //no used 析构函数执行

        Ctx(FoxRedisCluster* rds);
        ~Ctx();
        //创建命令超时定时事件
        bool init();
        static void EventCb(int fd, short event, void* d);
    };
private:
    virtual void pcmd(FCtx* ctx);
    bool pinit();
    void delayDelete(redisAsyncContext* c);
    static void OnAuthCb(redisClusterAsyncContext* c, void* rp, void* priv);
private:
    static void ConnectCb(const redisAsyncContext* c, int status);
    static void DisconnectCb(const redisAsyncContext* c, int status);
    static void CmdCb(redisClusterAsyncContext*c, void *r, void *privdata);
    static void TimeCb(int fd, short event, void* d);
private:
    Xten::FoxThread* m_thread;
    std::shared_ptr<redisClusterAsyncContext> m_context;
    std::string m_host;
    STATUS m_status;
    int m_ctxCount;

    struct timeval m_cmdTimeout;
    std::string m_err;
    struct event* m_event;
};


//管理所有redis客户端连接[四种类型都管理]
class RedisManager : public singleton<RedisManager>{
public:
    RedisManager();
    //获取指定name的redis连接客户端
    IRedis::ptr get(const std::string& name);

    std::ostream& dump(std::ostream& os);
private:
    //释放redis连接-----仅用于同步redis连接上层获取智能指针后回收连接
    void freeRedis(IRedis* r);
    //根据配置文件初始化并创建redis连接
    void init();
private:
    Xten::RWMutex m_mutex;
    std::map<std::string, std::list<IRedis*> > m_datas; //所有redis连接
    std::map<std::string, std::map<std::string, std::string> > m_config;
};

//工具类--->提供静态方法快速进行redis请求操作
class RedisUtil {
public:
    static ReplyPtr Cmd(const std::string& name, const char* fmt, ...);
    static ReplyPtr Cmd(const std::string& name, const char* fmt, va_list ap); 
    static ReplyPtr Cmd(const std::string& name, const std::vector<std::string>& args); 
    //尝试进行count次请求，任意一次成功就返回，最多请求count次
    static ReplyPtr TryCmd(const std::string& name, uint32_t count, const char* fmt, ...);
    static ReplyPtr TryCmd(const std::string& name, uint32_t count, const std::vector<std::string>& args); 
};

}

#endif
