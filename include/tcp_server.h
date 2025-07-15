#ifndef __XTEN_TCP_SERVER_H__
#define __XTEN_TCP_SERVER_H__
#include "socket.h"
#include "iomanager.h"
#include "nocopyable.hpp"
#include "config.h"
#include <functional>
namespace Xten
{
    // 定义tcpserver的配置结构
    struct TcpServerConf
    {
        typedef std::shared_ptr<TcpServerConf> ptr;

        std::vector<std::string> address;                  // 绑定的所有ip+port
        int keepalive = 0;                                 // 是否长连接
        int timeout = 1000 * 60 * 2;                       // 读取超时时间 默认2min
        int ssl = 0;                                       // 是否TLS加密
        int timewheel=0;                                    //是否启动时间轮定时器处理定时事件
        std::string id;                                    // server的id
        std::string type = "http";                         // 服务器类型  http websocket rock......
        std::string name;                                  // 名称
        std::string cert_file;                             // 加密证书
        std::string key_file;                              // 密钥对
        std::string accept_worker;                         // 接受链接调度器
        std::string io_worker;                             // 网络io调度器
        std::string process_worker;                        // 逻辑处理调度器
        std::unordered_map<std::string, std::string> args; // 参数
        bool IsVaild()
        {
            return !address.empty();
        }
        bool operator==(const TcpServerConf &oth)
        {
            return address == oth.address &&
                   keepalive == oth.keepalive &&
                   timeout == oth.timeout &&
                   ssl == oth.ssl &&
                   id == oth.id &&
                   timewheel == oth.timewheel &&
                   type == oth.type &&
                   name == oth.name &&
                   cert_file == oth.cert_file &&
                   key_file == oth.key_file &&
                   accept_worker == oth.accept_worker &&
                   io_worker == oth.io_worker &&
                   process_worker == oth.process_worker &&
                   args == oth.args;
        }
    };
    // 模板类全特化
    template <>
    class lexicalCast<std::string, TcpServerConf>
    {
        TcpServerConf operator()(const std::string &str)
        {
            YAML::Node node = YAML::Load(str);
            TcpServerConf conf;
            conf.id = node["id"].as<std::string>(conf.id);
            conf.type = node["type"].as<std::string>(conf.type);
            conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
            conf.timeout = node["timeout"].as<int>(conf.timeout);
            conf.name = node["name"].as<std::string>(conf.name);
            conf.ssl = node["ssl"].as<int>(conf.ssl);
            conf.timewheel=node["timewheel"].as<int>(conf.timewheel);
            conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
            conf.key_file = node["key_file"].as<std::string>(conf.key_file);
            conf.accept_worker = node["accept_worker"].as<std::string>();
            conf.io_worker = node["io_worker"].as<std::string>();
            conf.process_worker = node["process_worker"].as<std::string>();
            conf.args = lexicalCast<std::string, std::unordered_map<std::string, std::string>>()(node["args"].as<std::string>(""));
            if (node["address"].IsDefined())
            {
                for (size_t i = 0; i < node["address"].size(); ++i)
                {
                    conf.address.push_back(node["address"][i].as<std::string>());
                }
            }
            return conf;
        }
    };
    template <>
    class lexicalCast<TcpServerConf, std::string>
    {
        std::string operator()(const TcpServerConf &conf)
        {
            YAML::Node node;
            node["id"] = conf.id;
            node["type"] = conf.type;
            node["name"] = conf.name;
            node["keepalive"] = conf.keepalive;
            node["timeout"] = conf.timeout;
            node["ssl"] = conf.ssl;
            node["timewheel"]=conf.timewheel;
            node["cert_file"] = conf.cert_file;
            node["key_file"] = conf.key_file;
            node["accept_worker"] = conf.accept_worker;
            node["io_worker"] = conf.io_worker;
            node["process_worker"] = conf.process_worker;
            node["args"] = YAML::Load(lexicalCast<std::unordered_map<std::string, std::string>, std::string>()(conf.args));
            for (auto &i : conf.address)
            {
                node["address"].push_back(i);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    // tcpserver服务器类
    //(继承自shared_form_this是因为这个server的函数是在调度器中执行的，而可能存在server销毁而调度器还没执行这个函数)
    //为了防止这种情况产生，需要保证传入的函数中携带server的智能指针，该由this生成的智能指针和外部需要共享引用计数--->本质是同步处理
    //形成闭包，延长生命周期
    class TcpServer : public std::enable_shared_from_this<TcpServer>, public NoCopyable
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        TcpServer(Xten::IOManager *accept_worker = Xten::IOManager::GetThis(),
                  Xten::IOManager *io_worker = Xten::IOManager::GetThis(),
                  Xten::IOManager *process_worker = Xten::IOManager::GetThis(),
                  TcpServerConf::ptr conf=nullptr);
        //这个tcpServer的析构是在整个程序的主线程中进行
        virtual ~TcpServer();
        // 绑定一个地址
        bool Bind(Address::ptr addr);
        // 绑定多组地址并返回绑定失败的地址
        bool Bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails);
        // 加载证书
        bool LoadCertificates(const std::string &cert_file, const std::string &key_file);
        // 启动服务器
        virtual bool Start();
        // 终止服务器
        virtual void Stop();
        // 获取配置
        TcpServerConf::ptr GetServerConf() const
        {
            return _conf;
        }
        // 是否stop
        bool IsStop() const
        {
            return _isStop;
        }
        // server信息转化成字符串
        virtual std::string ToString(const std::string& prefix = "") const;
        // 获取Listensockets
        std::vector<Socket::ptr> GetListenSockets() const
        {
            return _listenSockets;
        }
        // 获取读取超时时间
        uint64_t GetRecvTimeout() const
        {
            return _recvTimeout;
        }
        // 获取name
        std::string GetName() const
        {
            return _name;
        }
        //获取时间轮定时器
        TimerWheelManager::ptr GetTimer() const
        {
            return _timeWheelMgr;
        }
    protected:
        //内部函数-->由_acceptWorker调度器执行接受链接函数
        void startAccept(TcpServer::ptr self,Socket::ptr listensocket);
        //处理一个session的函数->由_ioWorker调度器执行网络io（and 逻辑处理）
        virtual void handleClient(TcpServer::ptr self,Socket::ptr client);
    protected:
        std::vector<Socket::ptr> _listenSockets; // 所有该服务器的监听套接字
        IOManager *_acceptWorker;                // 接受链接调度器
        IOManager *_ioWorker;                    // 网络io调度器
        IOManager *_processWorker;               // 逻辑处理调度器
        uint64_t _recvTimeout;                   // 接受数据超时时间
        std::string _name;                       // 服务器name
        std::string _type = "tcp";               // 服务器类型
        bool _isSSL;                             // 是否TLS加密
        bool _isStop;                            // 是否停止
        bool _timeWheel;                        //是否启用时间轮定时器处理海量高精度定时任务
        TcpServerConf::ptr _conf;                // 配置项
        Xten::TimerWheelManager::ptr _timeWheelMgr;  //时间轮定时器
    };
}
#endif