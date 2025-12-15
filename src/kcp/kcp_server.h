#ifndef __XTEN_KCP_SERVER_H__
#define __XTEN_KCP_SERVER_H__
#include <memory>  
#include<functional>
#include"../nocopyable.hpp"
#include"../socket.h"
#include"kcp_session.h"
#include"../iomanager.h"
#include "../msghandle.h"
#include"third_part/ikcp.h"
#include"../address.h"
namespace Xten
{
    namespace kcp
    {
        struct  KcpConfig //配置属性
        {
            /* data */
            typedef std::shared_ptr<KcpConfig> ptr;
            std::string _ip;
            uint16_t    _port;
            uint16_t    _read_coroutine_num; //读协程数量
        };
        
        class KcpServer : public NoCopyable, public std::enable_shared_from_this<KcpServer>
        {
        public:
            typedef std::shared_ptr<KcpServer> ptr;
            // typedef std::function<uint32_t(KcpSession::ptr)> onClientNoActiveCb;
            // typedef std::function<uint32_t(KcpSession::ptr)> onConnectCb;
            // typedef std::function<uint32_t(KcpSession::ptr)> onCloeCb;
            KcpServer(IOManager* io_worker = IOManager::GetThis(),
                      IOManager* process_worker = IOManager::GetThis(),
                      KcpConfig::ptr config = nullptr);
            ~KcpServer();
            //bind  
            bool Bind(Address::ptr addr);
            //启动若干协程进行io操作
            void Start();
            //停止服务器
            void Stop();

            // void SetOnClientNoActiveCb(onClientNoActiveCb cb) { _timeoutCb = cb; }
            // onClientNoActiveCb GetOnClientNoActiveCb() const { return _timeoutCb; }

            // void SetOnConnectCb(onConnectCb cb) { _connectCb = cb; }
            // onConnectCb GetOnConnectCb() const { return _connectCb; }

            // void SetOnCloseCb(onCloeCb cb) { _closeCb = cb; }
            // onCloeCb GetOnCloseCb() const { return _closeCb; }        

            uint64_t GetRecvTimeout() const { return _recvTimeout; }
    private:
            void doRead(Socket::ptr udp_socket, KcpServer::ptr self);
    private:
            IOManager* _io_worker;  //io timer
            IOManager* _process_worker; //process
            // handler --从配置选项中获取
            MsgHanler::ptr _msgHandler;
            bool _isStop;
            KcpConfig::ptr m_kcpConfig; //配置属性
            std::vector<Socket::ptr> _udpSockets; //udp套接字---每个读协程都有一个socket【即使在同一个端口】

            // onClientNoActiveCb _timeoutCb; //客户端无活动回调
            // onConnectCb _connectCb; //连接建立回调
            // onCloeCb _closeCb; //连接关闭回调
            
            uint64_t _recvTimeout; //接收超时时间
            
            KcpConfig::ptr _config; //config
        };
    } // namespace kcp
    
} // namespace Xten

 
#endif