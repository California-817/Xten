            // 连接管理
#ifndef __XTEN_WS_SERVER_H__
#define __XTEN_WS_SERVER_H__
#include "../tcp_server.h"
#include "ws_servlet.h"
#include "ws_session.h"
#include <unordered_map>
namespace Xten
{
    namespace http
    {
        // 连接管理容器
        class Session_container : public NoCopyable
        {
        public:
            typedef std::shared_ptr<Session_container> ptr;
            WSSession::ptr get(const std::string &key);
            Session_container() = default;
            ~Session_container()
            {
                FiberMutex::Lock lock(_mtx);
                _connsMap.clear();
            }
            void remove(const std::string &key);
            void add(const std::string &key, WSSession::ptr se);
            // 发送消息
            bool sendmsg(const std::string &key, const std::string &data,
                         int32_t opcode = WSFrameHead::OPCODE::TEXT_FRAME, bool fin = true);
            // 发送消息给部分session
            void sengmsg(const std::vector<std::string> &keys, const std::string &data,
                         int32_t opcode = WSFrameHead::OPCODE::TEXT_FRAME, bool fin = true);
            // 广播消息
            void broadcastmsg(const std::string &data,
                              int32_t opcode = WSFrameHead::OPCODE::TEXT_FRAME, bool fin = true);

        private:
            FiberMutex _mtx;
            std::unordered_map<std::string, WSSession::ptr> _connsMap; // 连接管理
        };
        class WSServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<WSServer> ptr;
            WSServer(IOManager *accept = IOManager::GetThis(), IOManager *io = IOManager::GetThis(),
                     IOManager *process = IOManager::GetThis(), TcpServerConf::ptr config = nullptr);
            ~WSServer() = default;
            // 获取servlet分发器
            WSServletDispatch::ptr GetWSServletDispatch() const { return _dispatch; }
            // 获取process逻辑处理调度器(在servlet中可能要进行切换调度器)
            IOManager *GetProcessIOManager() const { return _processWorker; }
            Session_container &GetConnsMap() { return _connsMap; }

        protected:
            void handleClient(TcpServer::ptr self, Socket::ptr client) override;
            const char* formSessionId(const WSSession::ptr& session);
        private:
            WSServletDispatch::ptr _dispatch;
            // 连接管理
            Session_container _connsMap;
            uint64_t _sn;
        };
    }

}

#endif