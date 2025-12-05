#include "redis.h"
#include "../config.h"
#include "../log.h"
#include"../scheduler.h"
#include"../util.h"
namespace Xten {

static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");
static Xten::ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_redis =
    Xten::Config::LookUp("redis.config", std::map<std::string, std::map<std::string, std::string> >(), "redis config");
// redis:
//     config:
//         blog:
//             host: 127.0.0.1:6379
//             type: fox_redis
//             pool: 1
//             timeout: 100
//     desc: "type: redis,redis_cluster,fox_redis,fox_redis_cluster"

static std::string get_value(const std::map<std::string, std::string>& m
                             ,const std::string& key
                             ,const std::string& def = "") {
    auto it = m.find(key);
    return it == m.end() ? def : it->second;
}

//深拷贝一个一模一样的对象并返回
redisReply* RedisReplyClone(redisReply* r) {
    redisReply* c = (redisReply*)calloc(1, sizeof(*c));
    c->type = r->type;

    switch(r->type) {
        case REDIS_REPLY_INTEGER:
            c->integer = r->integer;
            break;
        case REDIS_REPLY_ARRAY:
            if(r->element != NULL && r->elements > 0) {
                c->element = (redisReply**)calloc(r->elements, sizeof(r));
                c->elements = r->elements;
                for(size_t i = 0; i < r->elements; ++i) {
                    c->element[i] = RedisReplyClone(r->element[i]);
                }
            }
            break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if(r->str == NULL) {
                c->str = NULL;
            } else {
                //c->str = strndup(r->str, r->len);
                c->str = (char*)malloc(r->len + 1);
                memcpy(c->str, r->str, r->len);
                c->str[r->len] = '\0';
            }
            c->len = r->len;
            break;
    }
    return c;
}


Redis::Redis() {
    m_type = IRedis::REDIS;
}

Redis::Redis(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS;
    auto tmp = get_value(conf, "host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0, pos);
    m_port = Xten::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_passwd = get_value(conf, "passwd");
    m_logEnable = Xten::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));

    tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = Xten::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

bool Redis::reconnect() {
    return redisReconnect(m_context.get());
}

bool Redis::connect() {
    return connect(m_host, m_port, 50);
}

bool Redis::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = redisConnectWithTimeout(ip.c_str(), port, tv); //hiredis封装的连接服务端接口，并返回redisContext
    if(c) {
        if(m_cmdTimeout.tv_sec || m_cmdTimeout.tv_usec) {
            //设置redis命令超时时间
            setTimeout(m_cmdTimeout.tv_sec * 1000 + m_cmdTimeout.tv_usec / 1000);
        }
        m_context.reset(c, redisFree);

        if(!m_passwd.empty()) {
            //登陆验证
            auto r = (redisReply*)redisCommand(c, "auth %s", m_passwd.c_str());
            if(!r) {
                XTEN_LOG_ERROR(g_logger) << "auth error:("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(r->type != REDIS_REPLY_STATUS) {
                XTEN_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(!r->str) {
                XTEN_LOG_ERROR(g_logger) << "auth reply str error: NULL("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(strcmp(r->str, "OK") == 0) {
                return true;
            } else {
                XTEN_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Redis::setTimeout(uint64_t v) {
    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
    redisSetTimeout(m_context.get(), m_cmdTimeout);
    return true;
}

ReplyPtr Redis::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr Redis::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisvCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            XTEN_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            XTEN_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::getReply() {
    redisReply* r = nullptr;
    if(redisGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int Redis::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int Redis::appendCmd(const char* fmt, va_list ap) {
    return redisvAppendCommand(m_context.get(), fmt, ap);
}

int Redis::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}




FoxRedis::FoxRedis(Xten::FoxThread* thr, const std::map<std::string, std::string>& conf)
    :m_thread(thr)
    ,m_status(UNCONNECTED)
    ,m_event(nullptr) {
    m_type = IRedis::FOX_REDIS;
    auto tmp = get_value(conf, "host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0, pos);
    m_port = Xten::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_passwd = get_value(conf, "passwd");
    m_ctxCount = 0;
    m_logEnable = Xten::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));

    tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = Xten::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}
//用户认证回调函数
void FoxRedis::OnAuthCb(redisAsyncContext* c, void* rp, void* priv) {
    FoxRedis* fr = (FoxRedis*)priv;
    redisReply* r = (redisReply*)rp;
    //判断用户认证是否通过
    if(!r) {
        XTEN_LOG_ERROR(g_logger) << "auth error:("
            << fr->m_host << ":" << fr->m_port << ", " << fr->m_name << ")";
        return;
    }
    if(r->type != REDIS_REPLY_STATUS) {
        XTEN_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
            << fr->m_host << ":" << fr->m_port << ", " << fr->m_name << ")";
        return;
    }
    if(!r->str) {
        XTEN_LOG_ERROR(g_logger) << "auth reply str error: NULL("
            << fr->m_host << ":" << fr->m_port << ", " << fr->m_name << ")";
        return;
    }
    if(strcmp(r->str, "OK") == 0) {
        XTEN_LOG_INFO(g_logger) << "auth ok: " << r->str << "("
            << fr->m_host << ":" << fr->m_port << ", " << fr->m_name << ")";
    } else {
        XTEN_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
            << fr->m_host << ":" << fr->m_port << ", " << fr->m_name << ")";
    }
}

void FoxRedis::ConnectCb(const redisAsyncContext* c, int status) {
    FoxRedis* ar = static_cast<FoxRedis*>(c->data); //init函数中已经将data设置为当前FoxRedis
    if(!status) {
        XTEN_LOG_INFO(g_logger) << "FoxRedis::ConnectCb "
                   << c->c.tcp.host << ":" << c->c.tcp.port
                   << " success";
        ar->m_status = CONNECTED; //状态设置成已经建立连接
        if(!ar->m_passwd.empty()) {
            //发起异步命令进行redis用户认证
            int rt = redisAsyncCommand(ar->m_context.get(), FoxRedis::OnAuthCb, ar, "auth %s", ar->m_passwd.c_str());
            if(rt) {
                XTEN_LOG_ERROR(g_logger) << "FoxRedis Auth fail: " << rt;
            }
        }

    } else {
        //连接回调调用错误
        XTEN_LOG_ERROR(g_logger) << "FoxRedis::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " fail, error:" << c->errstr;
        ar->m_status = UNCONNECTED; //状态设置成未连接
    }
}


void FoxRedis::DisconnectCb(const redisAsyncContext* c, int status) {
    XTEN_LOG_INFO(g_logger) << "FoxRedis::DisconnectCb "
               << c->c.tcp.host << ":" << c->c.tcp.port
               << " status:" << status;
    FoxRedis* ar = static_cast<FoxRedis*>(c->data);
    ar->m_status = UNCONNECTED; //状态设置成未建立
}

//foxthread进行异步redis请求后，得到响应后执行的回调函数
void FoxRedis::CmdCb(redisAsyncContext *ac, void *r, void *privdata) {
    Ctx* ctx = static_cast<Ctx*>(privdata);
    if(!ctx) {
        return;
    }
    if(ctx->timeout) {
        //得到响应的时候已经超时了-----不需要后续处理
        delete ctx;
        return;
    }

    auto m_logEnable = ctx->rds->m_logEnable; //是否记录日志

    redisReply* reply = (redisReply*)r;
    if(ac->err) {
        if(m_logEnable) {
            //  有错误
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "(" << ac->err << ") " << ac->errstr;
        }
        if(ctx->fctx->fiber) {
            //重新调度协程
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    } else if(!reply) {
        if(m_logEnable) {
            //响应为null
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: NULL";
        }
        if(ctx->fctx->fiber) {
            //调度协程
            ctx->fctx->scheduler->Schedule(ctx->fctx->fiber);
        }
    } else if(reply->type == REDIS_REPLY_ERROR) {
        if(m_logEnable) {
            //error
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: " << reply->str;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    } else {
        //成功获取到响应
        if(ctx->fctx->fiber) {
            //设置响应到fctx请求上下文中
            //注意：这里需要进行clone这个reply，因为这个reply生命周期是由hiredis内部管理的，直接传上去会导致内存错误
            ctx->fctx->rpy.reset(RedisReplyClone(reply), freeReplyObject);
            //调度协程
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    }
    delete ctx; //删除，new出来的
}
//定期发送ping心跳包给redis服务器进行保活
void FoxRedis::TimeCb(int fd, short event, void* d) {
    FoxRedis* ar = static_cast<FoxRedis*>(d); //传入的是this指针
    XTEN_ASSERT(ar->m_thread == Xten::FoxThread::GetThis());
    //异步执行ping命令保活--->心跳机制
    redisAsyncCommand(ar->m_context.get(), CmdCb, nullptr, "ping"); 
    XTEN_LOG_DEBUG(g_logger)<<"ping server: address="<<ar->m_host<<":"<<ar->m_port;
}

// no use
struct Res {
    redisAsyncContext* ctx;
    struct event* event;
};

//void DelayTimeCb(int fd, short event, void* d) {
//    XTEN_LOG_INFO(g_logger) << "DelayTimeCb";
//    Res* res = static_cast<Res*>(d);
//    redisAsyncFree(res->ctx);
//    evtimer_del(res->event);
//    event_free(res->event);
//    delete res;
//}

bool FoxRedis::init() {
    if(m_thread == Xten::FoxThread::GetThis()) {
        return pinit();  //是当前线程，直接执行pinit函数----->内部foxthread在获知连接没有正常建立的时候也会执行这个操作
    } else {
        //让这个foxredis内部工作线程执行这个函数
        m_thread->dispatch(std::bind(&FoxRedis::pinit, this));
    }
    return true;
}

void FoxRedis::delayDelete(redisAsyncContext* c) {
}

bool FoxRedis::pinit() {
    if(m_status != UNCONNECTED) {
        return true;
    }
    //FoxRedis是异步方式，所以异步创建redis连接上下文

    //1.建立redis连接并返回异步上下文
    auto ctx = redisAsyncConnect(m_host.c_str(), m_port);
    if(!ctx) {
        XTEN_LOG_ERROR(g_logger) << "redisAsyncConnect (" << m_host << ":" << m_port
                    << ") null";
        return false;
    }
    if(ctx->err) {
        XTEN_LOG_ERROR(g_logger) << "Error:(" << ctx->err << ")" << ctx->errstr;
        return false;
    }
    //2.设置data
    ctx->data = this;
    //3.与libevent网络库进行attach，使用foxThread进行异步命令操作
    redisLibeventAttach(ctx, m_thread->getBase());
    redisAsyncSetConnectCallback(ctx, ConnectCb); //注册连接建立回调
    redisAsyncSetDisconnectCallback(ctx, DisconnectCb); //注册连接断开回调
    m_status = CONNECTING;
    //m_context.reset(ctx, redisAsyncFree);
    m_context.reset(ctx, Xten::nop<redisAsyncContext>); //这个删除器不会做任何事情!!!!
    //m_context.reset(ctx, std::bind(&FoxRedis::delayDelete, this, std::placeholders::_1));
    if(m_event == nullptr) {
        //创建一个周期定时器时间并返回，此时并没注册
        m_event = event_new(m_thread->getBase(), -1, EV_TIMEOUT | EV_PERSIST, TimeCb, this);
        struct timeval tv = {120, 0};
        //添加定时器事件---周期2min，由内部的foxthread执行
        evtimer_add(m_event, &tv);
    }
    //立即触发一次定时事件
    TimeCb(0, 0, this);
    return true;
}

ReplyPtr FoxRedis::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto r = cmd(fmt, ap);
    va_end(ap);
    return r;
}

ReplyPtr FoxRedis::cmd(const char* fmt, va_list ap) {
    char* buf = nullptr;
    //int len = vasprintf(&buf, fmt, ap);
    int len = redisvFormatCommand(&buf, fmt, ap); //形成RESP字符串
    if(len == -1) {
        XTEN_LOG_ERROR(g_logger) << "redis fmt error: " << fmt;
        return nullptr;
    }

    FCtx fctx;
    fctx.cmd.append(buf, len); //保存到请求上下文的cmd中
    free(buf);
    
    //保存协程信息
    fctx.scheduler = Xten::Scheduler::GetThis();
    fctx.fiber = Xten::Fiber::GetThis();
    //通知foxthread发起异步redis请求
    m_thread->dispatch(std::bind(&FoxRedis::pcmd, this, &fctx));

    //协程挂起
    Xten::Fiber::YieldToHold();
    //恢复
    return fctx.rpy;
}

ReplyPtr FoxRedis::cmd(const std::vector<std::string>& argv) {
    FCtx fctx;  //发起请求的上下文
    do {
        std::vector<const char*> args;
        std::vector<size_t> args_len;
        for(auto& i : argv) {
            args.push_back(i.c_str());
            args_len.push_back(i.size());
        }
        char* buf = nullptr;
        //hiredis内部使用参数生成RESP字符串
        int len = redisFormatCommandArgv(&buf, argv.size(), &(args[0]), &(args_len[0]));
        if(len == -1 || !buf) {
            XTEN_LOG_ERROR(g_logger) << "redis fmt error";
            return nullptr;
        }
        fctx.cmd.append(buf, len);  //追加到cmd字符串中
        free(buf);
    } while(0);


    //将当前协程及其调度器进行保存到请求ctx中
    fctx.scheduler = Xten::Scheduler::GetThis();
    fctx.fiber = Xten::Fiber::GetThis();
    //将当前请求上下文交给foxthread调用pcmd进行异步redis请求
    m_thread->dispatch(std::bind(&FoxRedis::pcmd, this, &fctx));

    //挂起当前协程，使得线程可以进行调度其他协程不至于阻塞，提高访问redis的性能
    Xten::Fiber::YieldToHold();
    //协程被重新调度---说明结果得到--->
    //foxthread不会再访问&fctx处的参数，也就是说fctx虽然在栈空间上，但是栈帧在foxthread使用fctx期间并没有销毁---是内存访问安全的
 
    return fctx.rpy;
}

void FoxRedis::pcmd(FCtx* fctx) {
    if(m_status == UNCONNECTED) {
        //连接还没有正常建立
        XTEN_LOG_INFO(g_logger) << "redis (" << m_host << ":" << m_port << ") unconnected "
                   << fctx->cmd;
        init(); //建立连接----重连过程
        if(fctx->fiber) {
            //唤醒上层请求的协程---->协程的这次请求失败
            fctx->scheduler->Schedule(std::move(fctx->fiber));
        }
        return;
    }
    //连接成功建立

    //建立请求上下文----这个ctx是作为foxthread进行异步请求时，回调函数的参数传入
    Ctx* ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;

    if(!ctx->cmd.empty()) {
        //使用的是异步的redis接口实现-----只是注册事件，并没有发出请求
        redisAsyncFormattedCommand(m_context.get(), CmdCb, ctx, ctx->cmd.c_str(), ctx->cmd.size());
    }
}

FoxRedis::~FoxRedis() {
    if(m_event) {
        //删除定期保活的定时事件
        evtimer_del(m_event);
        event_free(m_event);
    }
}

FoxRedis::Ctx::Ctx(FoxRedis* r)
    :ev(nullptr)
    ,timeout(false)
    ,rds(r)
    ,thread(nullptr) {
    Xten::Atomic::addFetch(rds->m_ctxCount, 1);
}

FoxRedis::Ctx::~Ctx() {
    XTEN_ASSERT(thread == Xten::FoxThread::GetThis());
    Xten::Atomic::subFetch(rds->m_ctxCount, 1);
    if(ev) {
        //取消超时定时事件
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }

}


bool FoxRedis::Ctx::init() {
    //创建一个超时事件--->执行一次
    ev = evtimer_new(rds->m_thread->getBase(), EventCb, this);
    evtimer_add(ev, &rds->m_cmdTimeout);
    return true;
}

void FoxRedis::Ctx::EventCb(int fd, short event, void* d) {
    Ctx* ctx = static_cast<Ctx*>(d);
    ctx->timeout = 1; //标记超时----->foxthread执行异步回调的时候可以判断
    if(ctx->rds->m_logEnable) {
        Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
        XTEN_LOG_INFO(g_logger) << "redis cmd: '" << ctx->cmd << "' reach timeout "
                   << (ctx->rds->m_cmdTimeout.tv_sec * 1000
                           + ctx->rds->m_cmdTimeout.tv_usec / 1000) << "ms";
    }
    if(ctx->fctx->fiber) {
        //重新调度协程
        ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
    }
}










RedisCluster::RedisCluster() {
    m_type = IRedis::REDIS_CLUSTER;
}

RedisCluster::RedisCluster(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS_CLUSTER;
    m_host = get_value(conf, "host");
    m_passwd = get_value(conf, "passwd");
    m_logEnable = Xten::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));
    auto tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = Xten::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}


////RedisCluster
bool RedisCluster::reconnect() {
    return true;
    //return redisReconnect(m_context.get());
}

bool RedisCluster::connect() {
    return connect(m_host, m_port, 50);
}

bool RedisCluster::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = redisClusterConnectWithTimeout(ip.c_str(), tv, 0);
    if(c) {
        m_context.reset(c, redisClusterFree);
        if(!m_passwd.empty()) {
            auto r = (redisReply*)redisClusterCommand(c, "auth %s", m_passwd.c_str());
            if(!r) {
                XTEN_LOG_ERROR(g_logger) << "auth error:("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(r->type != REDIS_REPLY_STATUS) {
                XTEN_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(!r->str) {
                XTEN_LOG_ERROR(g_logger) << "auth reply str error: NULL("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(strcmp(r->str, "OK") == 0) {
                return true;
            } else {
                XTEN_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
        }
        return true;
    }
    return false;
}

bool RedisCluster::setTimeout(uint64_t ms) {
    //timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    //redisSetTimeout(m_context.get(), tv);
    return true;
}

ReplyPtr RedisCluster::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisCluster::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisClustervCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            XTEN_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisClusterCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            XTEN_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::getReply() {
    redisReply* r = nullptr;
    if(redisClusterGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        XTEN_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int RedisCluster::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int RedisCluster::appendCmd(const char* fmt, va_list ap) {
    return redisClustervAppendCommand(m_context.get(), fmt, ap);
}

int RedisCluster::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisClusterAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}

FoxRedisCluster::FoxRedisCluster(Xten::FoxThread* thr, const std::map<std::string, std::string>& conf)
    :m_thread(thr)
    ,m_status(UNCONNECTED)
    ,m_event(nullptr) {
    m_ctxCount = 0;

    m_type = IRedis::FOX_REDIS_CLUSTER;
    m_host = get_value(conf, "host");
    m_passwd = get_value(conf, "passwd");
    m_logEnable = Xten::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));
    auto tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = Xten::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

void FoxRedisCluster::OnAuthCb(redisClusterAsyncContext* c, void* rp, void* priv) {
    FoxRedisCluster* fr = (FoxRedisCluster*)priv;
    redisReply* r = (redisReply*)rp;
    if(!r) {
        XTEN_LOG_ERROR(g_logger) << "auth error:("
            << fr->m_host << ", " << fr->m_name << ")";
        return;
    }
    if(r->type != REDIS_REPLY_STATUS) {
        XTEN_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
            << fr->m_host << ", " << fr->m_name << ")";
        return;
    }
    if(!r->str) {
        XTEN_LOG_ERROR(g_logger) << "auth reply str error: NULL("
            << fr->m_host << ", " << fr->m_name << ")";
        return;
    }
    if(strcmp(r->str, "OK") == 0) {
        XTEN_LOG_INFO(g_logger) << "auth ok: " << r->str << "("
            << fr->m_host << ", " << fr->m_name << ")";
    } else {
        XTEN_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
            << fr->m_host << ", " << fr->m_name << ")";
    }
}

void FoxRedisCluster::ConnectCb(const redisAsyncContext* c, int status) {
    //XTEN_LOG_INFO(g_logger) << "ConnectCb " << status;
    //FoxRedisCluster* ar = static_cast<FoxRedisCluster*>(c->data);
    if(!status) {
        XTEN_LOG_INFO(g_logger) << "FoxRedisCluster::ConnectCb "
                   << c->c.tcp.host << ":" << c->c.tcp.port
                   << " success";
    } else {
        XTEN_LOG_ERROR(g_logger) << "FoxRedisCluster::ConnectCb "
                    << c->c.tcp.host << ":" << c->c.tcp.port
                    << " fail, error:" << c->errstr;
    }
}

void FoxRedisCluster::DisconnectCb(const redisAsyncContext* c, int status) {
    XTEN_LOG_INFO(g_logger) << "FoxRedisCluster::DisconnectCb "
               << c->c.tcp.host << ":" << c->c.tcp.port
               << " status:" << status;
}

void FoxRedisCluster::CmdCb(redisClusterAsyncContext *ac, void *r, void *privdata) {
    Ctx* ctx = static_cast<Ctx*>(privdata);
    if(ctx->timeout) {
        delete ctx;
        //if(ctx && ctx->fiber) {
        //    XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd << "' timeout("
        //                << (ctx->rds->m_cmdTimeout.tv_sec * 1000
        //                        + ctx->rds->m_cmdTimeout.tv_usec / 1000)
        //                << "ms)";
        //    ctx->scheduler->schedule(&ctx->fiber);
        //    ctx->cancelEvent();
        //}
        return;
    }
    auto m_logEnable = ctx->rds->m_logEnable;
    //ctx->cancelEvent();
    redisReply* reply = (redisReply*)r;
    //++ctx->callback_count;
    if(ac->err) {
        if(m_logEnable) {
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "(" << ac->err << ") " << ac->errstr;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    } else if(!reply) {
        if(m_logEnable) {
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: NULL";
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    } else if(reply->type == REDIS_REPLY_ERROR) {
        if(m_logEnable) {
            Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
            XTEN_LOG_ERROR(g_logger) << "redis cmd: '" << ctx->cmd
                        << "' "
                        << "reply: " << reply->str;
        }
        if(ctx->fctx->fiber) {
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    } else {
        if(ctx->fctx->fiber) {
            ctx->fctx->rpy.reset(RedisReplyClone(reply), freeReplyObject);
            ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
        }
    }
    //ctx->ref = nullptr;
    delete ctx;
    //ctx->tref = nullptr;
}

void FoxRedisCluster::TimeCb(int fd, short event, void* d) {
    //FoxRedisCluster* ar = static_cast<FoxRedisCluster*>(d);
    //redisAsyncCommand(ar->m_context.get(), CmdCb, nullptr, "ping");
}

bool FoxRedisCluster::init() {
    if(m_thread == Xten::FoxThread::GetThis()) {
        return pinit();
    } else {
        m_thread->dispatch(std::bind(&FoxRedisCluster::pinit, this));
    }
    return true;
}

void FoxRedisCluster::delayDelete(redisAsyncContext* c) {
    //if(!c) {
    //    return;
    //}

    //Res* res = new Res();
    //res->ctx = c;
    //struct event* event = event_new(m_thread->getBase(), -1, EV_TIMEOUT, DelayTimeCb, res);
    //res->event = event;
    //
    //struct timeval tv = {60, 0};
    //evtimer_add(event, &tv);
}

bool FoxRedisCluster::pinit() {
    if(m_status != UNCONNECTED) {
        return true;
    }
    XTEN_LOG_INFO(g_logger) << "FoxRedisCluster pinit:" << m_host;
    redisClusterAsyncContext *ctx = redisClusterAsyncContextInit();
    redisClusterAsyncSetConnectCallback(ctx, ConnectCb);
    redisClusterAsyncSetDisconnectCallback(ctx, DisconnectCb);
    redisClusterSetOptionAddNodes(ctx->cc, m_host.c_str());
    if(!m_passwd.empty()) {
        redisClusterSetOptionPassword(ctx->cc, m_passwd.c_str());
    }
    redisClusterLibeventAttach(ctx, m_thread->getBase());
    redisClusterConnect2(ctx->cc);
    if(ctx->cc->err) {
        XTEN_LOG_ERROR(g_logger) << "Error:(" << ctx->cc->err << ")" << ctx->cc->errstr
            << " passwd=" << m_passwd;
        return false;
    }
    //ctx->data = this;

    m_status = CONNECTED;
    //m_context.reset(ctx, redisClusterAsyncFree);
    m_context.reset(ctx, Xten::nop<redisClusterAsyncContext>);
    //m_context.reset(ctx, std::bind(&FoxRedisCluster::delayDelete, this, std::placeholders::_1));
    if(m_event == nullptr) {
        m_event = event_new(m_thread->getBase(), -1, EV_TIMEOUT | EV_PERSIST, TimeCb, this);
        struct timeval tv = {120, 0};
        evtimer_add(m_event, &tv);
        TimeCb(0, 0, this);
    }
    return true;
}

ReplyPtr FoxRedisCluster::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto r = cmd(fmt, ap);
    va_end(ap);
    return r;
}

ReplyPtr FoxRedisCluster::cmd(const char* fmt, va_list ap) {
    char* buf = nullptr;
    //int len = vasprintf(&buf, fmt, ap);
    int len = redisvFormatCommand(&buf, fmt, ap);
    if(len == -1 || !buf) {
        XTEN_LOG_ERROR(g_logger) << "redis fmt error: " << fmt;
        return nullptr;
    }
    FCtx fctx;
    fctx.cmd.append(buf, len);
    free(buf);
    fctx.scheduler = Xten::Scheduler::GetThis();
    fctx.fiber = Xten::Fiber::GetThis();
    //Ctx::ptr ctx(new Ctx(this));
    //if(buf) {
    //    ctx->cmd.append(buf, len);
    //    free(buf);
    //}
    //ctx->scheduler = Xten::Scheduler::GetThis();
    //ctx->fiber = Xten::Fiber::GetThis();
    //ctx->thread = m_thread;

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd, this, &fctx));
    Xten::Fiber::YieldToHold();
    return fctx.rpy;
}

ReplyPtr FoxRedisCluster::cmd(const std::vector<std::string>& argv) {
    //Ctx::ptr ctx(new Ctx(this));
    //ctx->parts = argv;
    FCtx fctx;
    do {
        std::vector<const char*> args;
        std::vector<size_t> args_len;
        for(auto& i : argv) {
            args.push_back(i.c_str());
            args_len.push_back(i.size());
        }
        char* buf = nullptr;
        int len = redisFormatCommandArgv(&buf, argv.size(), &(args[0]), &(args_len[0]));
        if(len == -1 || !buf) {
            XTEN_LOG_ERROR(g_logger) << "redis fmt error";
            return nullptr;
        }
        fctx.cmd.append(buf, len);
        free(buf);
    } while(0);

    fctx.scheduler = Xten::Scheduler::GetThis();
    fctx.fiber = Xten::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd, this, &fctx));
    Xten::Fiber::YieldToHold();
    return fctx.rpy;
}

void FoxRedisCluster::pcmd(FCtx* fctx) {
    if(m_status != CONNECTED) {
        XTEN_LOG_INFO(g_logger) << "redis (" << m_host << ") unconnected "
                   << fctx->cmd;
        init();
        if(fctx->fiber) {
            fctx->scheduler->Schedule(std::move(fctx->fiber));
        }
        return;
    }
    Ctx* ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;
    //ctx->ref = ctx;
    //ctx->tref = ctx;
    if(!ctx->cmd.empty()) {
        //redisClusterAsyncCommand(m_context.get(), CmdCb, ctx.get(), ctx->cmd.c_str());
        redisClusterAsyncFormattedCommand(m_context.get(), CmdCb, ctx, &ctx->cmd[0], ctx->cmd.size());
    //} else if(!ctx->parts.empty()) {
    //    std::vector<const char*> argv;
    //    std::vector<size_t> argv_len;
    //    for(auto& i : ctx->parts) {
    //        argv.push_back(i.c_str());
    //        argv_len.push_back(i.size());
    //    }
    //    redisClusterAsyncCommandArgv(m_context.get(), CmdCb, ctx.get(), argv.size(),
    //            &(argv[0]), &(argv_len[0]));
    }
}

FoxRedisCluster::~FoxRedisCluster() {
    if(m_event) {
        evtimer_del(m_event);
        event_free(m_event);
    }
}

FoxRedisCluster::Ctx::Ctx(FoxRedisCluster* r)
    :ev(nullptr)
    ,timeout(false)
    ,rds(r)
    //,scheduler(nullptr)
    ,thread(nullptr) {
    //,cancel_count(0)
    //,destory(0)
    //,callback_count(0) {
    fctx = nullptr;
    Xten::Atomic::addFetch(rds->m_ctxCount, 1);
}

FoxRedisCluster::Ctx::~Ctx() {
    XTEN_ASSERT(thread == Xten::FoxThread::GetThis());
    //XTEN_ASSERT(destory == 0);
    Xten::Atomic::subFetch(rds->m_ctxCount, 1);
    //++destory;
    //cancelEvent();

    if(ev) {
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }
}

void FoxRedisCluster::Ctx::cancelEvent() {
    //XTEN_LOG_INFO(g_logger) << "cancelEvent " << Xten::FoxThread::GetThis()
    //           << " - " << thread
    //           << " - " << Xten::IOManager::GetThis()
    //           << " - " << cancel_count;
    //if(thread != Xten::FoxThread::GetThis()) {
    //    XTEN_LOG_INFO(g_logger) << "cancelEvent " << Xten::FoxThread::GetThis()
    //               << " - " << thread
    //               << " - " << Xten::IOManager::GetThis()
    //               << " - " << cancel_count;

    //    //XTEN_LOG_INFO(g_logger) << "cancelEvent thread=" << thread << " " << thread->getId()
    //    //           << " this=" << Xten::FoxThread::GetThis();
    //    //XTEN_ASSERT(thread == Xten::FoxThread::GetThis());
    //}
    //XTEN_ASSERT(!Xten::IOManager::GetThis());
    ////if(Xten::Atomic::addFetch(cancel_count) > 1) {
    ////    return;
    ////}
    ////XTEN_ASSERT(!Xten::Fiber::GetThis());
    ////Xten::RWMutex::WriteLock lock(mutex);
    //if(++cancel_count > 1) {
    //    return;
    //}
    //if(ev) {
    //    auto e = ev;
    //    ev = nullptr;
    //    //lock.unlock();
    //    //evtimer_del(e);
    //    //event_free(e);
    //    if(thread == Xten::FoxThread::GetThis()) {
    //        evtimer_del(e);
    //        event_free(e);
    //    } else {
    //        thread->dispatch([e](){
    //            evtimer_del(e);
    //            event_free(e);
    //        });
    //    }
    //}
    //ref = nullptr;
}

bool FoxRedisCluster::Ctx::init() {
    XTEN_ASSERT(thread == Xten::FoxThread::GetThis());
    ev = evtimer_new(rds->m_thread->getBase(), EventCb, this);
    evtimer_add(ev, &rds->m_cmdTimeout);
    return true;
}

void FoxRedisCluster::Ctx::EventCb(int fd, short event, void* d) {
    Ctx* ctx = static_cast<Ctx*>(d);
    if(!ctx->ev) {
        return;
    }
    ctx->timeout = 1;
    if(ctx->rds->m_logEnable) {
        Xten::replace(ctx->cmd, "\r\n", "\\r\\n");
        XTEN_LOG_INFO(g_logger) << "redis cmd: '" << ctx->cmd << "' reach timeout "
                   << (ctx->rds->m_cmdTimeout.tv_sec * 1000
                           + ctx->rds->m_cmdTimeout.tv_usec / 1000) << "ms";
    }
    ctx->cancelEvent();
    if(ctx->fctx->fiber) {
        ctx->fctx->scheduler->Schedule(std::move(ctx->fctx->fiber));
    }
    //ctx->ref = nullptr;
    //delete ctx;
    //ctx->tref = nullptr;
}

IRedis::ptr RedisManager::get(const std::string& name) {
    Xten::RWMutex::WriteLock lock(m_mutex);
    auto it = m_datas.find(name);
    if(it == m_datas.end()) {
        return nullptr;
    }
    if(it->second.empty()) {
        return nullptr;
    }
    auto r = it->second.front();
    it->second.pop_front();
    if(r->getType() == IRedis::FOX_REDIS
            || r->getType() == IRedis::FOX_REDIS_CLUSTER) {
        //如果是异步的redis客户端连接，则多线程可以复用这一个连接，因为异步redis命令的执行是由内部的foxthread单线程异步执行的
        it->second.push_back(r); //重新放回队列尾部，保证上层可以多线程同时使用
        return std::shared_ptr<IRedis>(r, Xten::nop<IRedis>); //智能指针析构函数不需要做任何事情
    }
    lock.unlock();
    //走到这说明是同步的redis连接
    auto rr = dynamic_cast<ISyncRedis*>(r); 
    //判断是否长时间没有使用
    if((time(0) - rr->getLastActiveTime()) > 30) {
        if(!rr->cmd("ping")) { //ping一下
            if(!rr->reconnect()) { //ping失败 重新连接
                Xten::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(r);
                return nullptr;
            }
        }
    }
    //更新上次活跃时间
    rr->setLastActiveTime(time(0));
    return std::shared_ptr<IRedis>(r, std::bind(&RedisManager::freeRedis
                        ,this, std::placeholders::_1)); //智能指针析构函数回收连接
}

//连接放回
void RedisManager::freeRedis(IRedis* r) {
    Xten::RWMutex::WriteLock lock(m_mutex);
    m_datas[r->getName()].push_back(r);
}

RedisManager::RedisManager() {
    init();
}

void RedisManager::init() {
    m_config = g_redis->GetValue();
    size_t done = 0;
    size_t total = 0;
    for(auto& i : m_config) {
        auto type = get_value(i.second, "type");
        auto pool = Xten::TypeUtil::Atoi(get_value(i.second, "pool"));
        total += pool;
        for(int n = 0; n < pool; ++n) {
            if(type == "redis") {
                Xten::Redis* rds(new Xten::Redis(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                Xten::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                Xten::Atomic::addFetch(done, 1);
            } else if(type == "redis_cluster") {
                Xten::RedisCluster* rds(new Xten::RedisCluster(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                Xten::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                Xten::Atomic::addFetch(done, 1);
            } else if(type == "fox_redis") {
                auto conf = i.second;
                auto name = i.first;
                //多个 FoxRedis 绑定到同一个 FoxThread 是允许且常见的（同一 event_base 管理多个连接）
                //不需要严格规定redis配置连接个数要和foxthread线程个数相同，完全可以出现一个foxthread处理多个redis连接的情况--->是合法并且框架支持
                Xten::FoxThreadManager::GetInstance()->dispatch("redis", [this, conf, name, &done](){
                    Xten::FoxRedis* rds(new Xten::FoxRedis(Xten::FoxThread::GetThis(), conf));
                    rds->init();
                    rds->setName(name);

                    Xten::RWMutex::WriteLock lock(m_mutex);
                    m_datas[name].push_back(rds);
                    Xten::Atomic::addFetch(done, 1);
                });
            } else if(type == "fox_redis_cluster") {
                auto conf = i.second;
                auto name = i.first;
                Xten::FoxThreadManager::GetInstance()->dispatch("redis", [this, conf, name, &done](){
                    Xten::FoxRedisCluster* rds(new Xten::FoxRedisCluster(Xten::FoxThread::GetThis(), conf));
                    rds->init();
                    rds->setName(name);

                    Xten::RWMutex::WriteLock lock(m_mutex);
                    m_datas[name].push_back(rds);
                    Xten::Atomic::addFetch(done, 1);
                });
            } else {
                Xten::Atomic::addFetch(done, 1);
            }
        }
    }
    //等待所有连接建立成功
    while(done != total) {
        usleep(5000);
    }
}

std::ostream& RedisManager::dump(std::ostream& os) {
    os << "[RedisManager total=" << m_config.size() << "]" << std::endl;
    for(auto& i : m_config) {
        os << "    " << i.first << " :[";
        for(auto& n : i.second) {
            os << "{" << n.first << ":" << n.second << "}";
        }
        os << "]" << std::endl;
    }
    return os;
}




ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = Cmd(name, fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, va_list ap) {
    auto rds = RedisManager::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(fmt, ap);
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const std::vector<std::string>& args) {
    auto rds = RedisManager::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(args);
}


ReplyPtr RedisUtil::TryCmd(const std::string& name, uint32_t count, const char* fmt, ...) {
    for(uint32_t i = 0; i < count; ++i) {
        va_list ap;
        va_start(ap, fmt);
        ReplyPtr rt = Cmd(name, fmt, ap);
        va_end(ap);

        if(rt) {
            return rt;
        }
    }
    return nullptr;
}

ReplyPtr RedisUtil::TryCmd(const std::string& name, uint32_t count, const std::vector<std::string>& args) {
    for(uint32_t i = 0; i < count; ++i) {
        ReplyPtr rt = Cmd(name, args);
        if(rt) {
            return rt;
        }
    }
    return nullptr;
}

}
