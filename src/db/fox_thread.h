#ifndef __XTEN_DB_FOX_THREAD_H__
#define __XTEN_DB_FOX_THREAD_H__

#include <thread>
#include <vector>
#include <list>
#include <map>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>

#include "../singleton.hpp"
#include "../mutex.h"

namespace Xten {

class FoxThread;
class IFoxThread {
public:
    typedef std::shared_ptr<IFoxThread> ptr;
    typedef std::function<void()> callback;

    virtual ~IFoxThread(){};
    //派发函数task
    virtual bool dispatch(callback cb) = 0;
    virtual bool dispatch(uint32_t id, callback cb) = 0;
    //批量派发
    virtual bool batchDispatch(const std::vector<callback>& cbs) = 0;
    //等价派发函数
    virtual void broadcast(callback cb) = 0;

    //启动函数[创建线程进入事件循环]
    virtual void start() = 0;
    //通知线程退出事件循环
    virtual void stop() = 0;
    //等待线程退出
    virtual void join() = 0;
    virtual void dump(std::ostream& os) = 0;
    //获取任务处理总数
    virtual uint64_t getTotal() = 0;
};

//进入libevent事件循环的线程，依靠事件驱动的方式执行回调，在回调中处理上层派发的任务
class FoxThread : public IFoxThread {
public:
    typedef std::shared_ptr<FoxThread> ptr;
    typedef IFoxThread::callback callback;
    typedef std::function<void (FoxThread*)> init_cb;
    FoxThread(const std::string& name = "", struct event_base* base = NULL);
    ~FoxThread();

    static FoxThread* GetThis();
    static const std::string& GetFoxThreadName();
    static void GetAllFoxThreadName(std::map<uint64_t, std::string>& names);

    void setThis();
    void unsetThis();

    void start();

    virtual bool dispatch(callback cb);
    virtual bool dispatch(uint32_t id, callback cb);
    virtual bool batchDispatch(const std::vector<callback>& cbs);
    virtual void broadcast(callback cb);

    void join();
    void stop();
    bool isStart() const { return m_start;}

    struct event_base* getBase() { return m_base;}
    std::thread::id getId() const;

    void* getData(const std::string& name);
    template<class T>
    T* getData(const std::string& name) {
        return (T*)getData(name);
    }
    void setData(const std::string& name, void* v);
    //初始化回调
    void setInitCb(init_cb v) { m_initCb = v;}

    void dump(std::ostream& os);
    virtual uint64_t getTotal() { return m_total;}
private:
    //线程函数
    void thread_cb();
    //管道读回调
    static void read_cb(evutil_socket_t sock, short which, void* args);
private:
    evutil_socket_t m_read; //读fd
    evutil_socket_t m_write; //写fd
    struct event_base* m_base; //事件循环结构
    struct event* m_event;
    std::thread* m_thread; //线程
    Xten::RWMutex m_mutex; //锁--保证任务队列安全
    std::list<callback> m_callbacks; //上层派发的任务队列

    std::string m_name;//名字
    init_cb m_initCb; 

    std::map<std::string, void*> m_datas;

    bool m_working; //是否工作
    bool m_start; //是否启动
    uint64_t m_total; //总任务数
};

//foxthread组成的线程池
class FoxThreadPool : public IFoxThread {
public:
    typedef std::shared_ptr<FoxThreadPool> ptr;
    typedef IFoxThread::callback callback;

    FoxThreadPool(uint32_t size, const std::string& name = "", bool advance = false);
    ~FoxThreadPool();

    void start();
    void stop();
    void join();

    //随机线程执行
    bool dispatch(callback cb);
    bool batchDispatch(const std::vector<callback>& cb);
    //指定线程执行
    bool dispatch(uint32_t id, callback cb);

    FoxThread* getRandFoxThread();
    void setInitCb(FoxThread::init_cb v) { m_initCb = v;}

    void dump(std::ostream& os);

    void broadcast(callback cb);
    virtual uint64_t getTotal() { return m_total;}
private:
    void releaseFoxThread(FoxThread* t);
    void check();

    void wrapcb(std::shared_ptr<FoxThread>, callback cb);
private:
    uint32_t m_size;
    uint32_t m_cur;
    std::string m_name;
    bool m_advance; //是否使用更精密的任务调度机制
    bool m_start;
    RWMutex m_mutex;
    std::list<callback> m_callbacks; //当使用更精密的任务调度机制时，任务先派送到这个队列中
    std::vector<FoxThread*> m_threads;
    std::list<FoxThread*> m_freeFoxThreads;
    FoxThread::init_cb m_initCb;
    uint64_t m_total;
};

//使用一个单例管理所有的foxthread和foxthreadpool
class FoxThreadManager : public singleton<FoxThreadManager> {
public:
    typedef IFoxThread::callback callback;
    //向指定name的foxthread或者foxthreadpool派发任务
    void dispatch(const std::string& name, callback cb);
    void dispatch(const std::string& name, uint32_t id, callback cb);
    void batchDispatch(const std::string& name, const std::vector<callback>& cbs);
    void broadcast(const std::string& name, callback cb);

    void dumpFoxThreadStatus(std::ostream& os);
    //根据配置文件初始化
    void init();
    //启动线程进入事件循环
    void start();
    //终止线程并等待退出
    void stop();
    //获取线程或者线程池
    IFoxThread::ptr get(const std::string& name);
    //添加
    void add(const std::string& name, IFoxThread::ptr thr);
private:
    std::map<std::string, IFoxThread::ptr> m_threads;
};

}
#endif
