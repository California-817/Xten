#include "rock_server.h"
#include "rock_stream.h"
#include "../log.h"
#include "../module/module.h"
#if ROCK_CATEGORY == SYNC
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    RockServer::RockServer(Xten::IOManager *accept_worker,
                           Xten::IOManager *io_worker,
                           Xten::IOManager *process_worker,
                           TcpServerConf::ptr conf)
        : TcpServer(accept_worker, io_worker, process_worker, conf)
    {
    }

    // 处理请求函数
    static bool handleRequest(RockStream::ptr self, RockRequest::ptr request, IOManager *process)
    {
        RockResponse::ptr rsp = request->CreateResponse();
        bool handleRet = true;
        {
            SwitchScheduler sw(process); // 该协程转移到了process调度器执行
            handleRet = self->GetRequestHandleCb()(request, rsp, self);
        }
        // error
        if (!handleRet)
        {
            // 发送响应
            self->SendMessage(rsp);
            self->Close();
            return handleRet;
        }
        // success
        self->SendMessage(rsp);
        return handleRet;
    }
    static bool handleNotify(RockStream::ptr self, RockNotify::ptr notify, IOManager *process)
    {
        bool handleRet = true;
        {
            SwitchScheduler sw(process);
            bool handleRet = self->GetNotifyHandleCb()(notify, self);
        }
        if (!handleRet)
        {
            self->Close();
        }
        return handleRet;
    }

    // 处理一个session的函数->由_ioWorker调度器执行网络io（and 逻辑处理）
    void RockServer::handleClient(TcpServer::ptr self, Socket::ptr client)
    {
        // 由Socket创建RockSession
        RockSession::ptr session = std::make_shared<RockSession>(client);
        // 设置处理函数
        session->SetConnectCb([](RockStream::ptr self) -> bool // 执行注册的所有Module的对应的OnConnect函数
                              { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                         [self](Module::ptr mod) -> bool
                                                                         {
                                                RockModule::ptr rockmod=std::dynamic_pointer_cast<RockModule>(mod);
                                                if(rockmod)
                                                {
                                                    return rockmod->OnConnect(self);
                                                }
                                                return false; }); });
        session->SetDisConnectCb([](RockStream::ptr self) -> bool
                                 { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                            [self](Module::ptr mod) -> bool
                                                                            {
                        RockModule::ptr rockmod=std::dynamic_pointer_cast<RockModule>(mod);
                        if(rockmod)
                        {
                            return rockmod->OnDisConnect(self);
                        }
                        return false; }); });
        session->SetRequestHandleCb([](RockRequest::ptr request, RockResponse::ptr response,
                                       RockStream::ptr stream) -> bool
                                    { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                               [request, response, stream](Module::ptr mod) -> bool
                                                                               {
                                                                                   RockModule::ptr rockmod = std::dynamic_pointer_cast<RockModule>(mod);
                                                                                   if (rockmod)
                                                                                   {
                                                                                       return rockmod->OnHandleRockRequest(request, response, stream);
                                                                                   }
                                                                                   return false;
                                                                               }); });
        session->SetNotifyHandleCb([](RockNotify::ptr notify, RockStream::ptr stream) -> bool
                                   { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                              [notify, stream](Module::ptr mod) -> bool
                                                                              {   
                                            RockModule::ptr rockmod = std::dynamic_pointer_cast<RockModule>(mod);
                                            if(rockmod)
                                            {
                                                rockmod->OnHandleRockNotify(notify,stream);
                                            }
                                            return false; }); });

        // 服务端进行对一个连接的处理
        do
        {
            // 调用onConnect函数
            if (!session->OnConnect(session))
                break;
            while (true)
            {
                Message::ptr msg = session->RecvMessage();
                if(!msg)
                    break;
                XTEN_LOG_INFO(g_logger)<<msg->ToString();
                Message::MessageType type = (Message::MessageType)msg->GetMessageType();
                if (type == Message::MessageType::REQUEST)
                {
                    if(!handleRequest(session, std::dynamic_pointer_cast<RockRequest>(msg), _processWorker))
                        break;
                }
                else if (type = Message::MessageType::NOTIFY)
                {
                    if(!handleNotify(session, std::dynamic_pointer_cast<RockNotify>(msg), _processWorker))
                        break;
                }
                else
                {
                    XTEN_LOG_ERROR(g_logger) << "RockServer recv Invalid Message Type!!!";
                    break;
                }
            }
            // 调用OnDisConnnect函数
            session->OnDisConnect(session);
        } while (false);
        // 关闭连接
        session->Close();
    }
}

#elif ROCK_CATEGORY == ASYNC
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    RockServer::RockServer(Xten::IOManager *accept_worker,
                           Xten::IOManager *io_worker,
                           Xten::IOManager *process_worker,
                           TcpServerConf::ptr conf)
        : TcpServer(accept_worker, io_worker, process_worker, conf)
    {
    }
    // 处理一个session的函数->由_ioWorker调度器执行网络io（and 逻辑处理）
    void RockServer::handleClient(TcpServer::ptr self, Socket::ptr client)
    {
        // 由Socket创建RockSession
        RockSession::ptr session = std::make_shared<RockSession>(client);
        // 设置调度器
        session->SetIOWorker(_ioWorker);
        session->SetProcessWorker(_processWorker);
        // 设置处理函数
        session->SetConnectCb([](AsyncSocketStream::ptr self) -> bool // 执行注册的所有Module的对应的OnConnect函数
                              { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                         [self](Module::ptr mod) -> bool
                                                                         {
                                                RockModule::ptr rockmod=std::dynamic_pointer_cast<RockModule>(mod);
                                                if(rockmod)
                                                {
                                                    return rockmod->OnConnect(self);
                                                }
                                                return false; }); });
        session->SetDisConnectCb([](AsyncSocketStream::ptr self) -> bool
                                 { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                            [self](Module::ptr mod) -> bool
                                                                            {
                        RockModule::ptr rockmod=std::dynamic_pointer_cast<RockModule>(mod);
                        if(rockmod)
                        {
                            return rockmod->OnDisConnect(self);
                        }
                        return false; }); });
        session->SetRequestHandleCb([](RockRequest::ptr request, RockResponse::ptr response,
                                       RockStream::ptr stream) -> bool
                                    { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                               [request, response, stream](Module::ptr mod) -> bool
                                                                               {
                                                                                   RockModule::ptr rockmod = std::dynamic_pointer_cast<RockModule>(mod);
                                                                                   if (rockmod)
                                                                                   {
                                                                                       return rockmod->OnHandleRockRequest(request, response, stream);
                                                                                   }
                                                                                   return false;
                                                                               }); });
        session->SetNotifyHandleCb([](RockNotify::ptr notify, RockStream::ptr stream) -> bool
                                   { return ModuleMgr::GetInstance()->Foreach(Module::ModuleType::ROCK,
                                                                              [notify, stream](Module::ptr mod) -> bool
                                                                              {   
                                            RockModule::ptr rockmod = std::dynamic_pointer_cast<RockModule>(mod);
                                            if(rockmod)
                                            {
                                                rockmod->OnHandleRockNotify(notify,stream);
                                            }
                                            return false; }); });
        // 启动异步的RockSession(会在内部开启读写协程进行处理session,通过shared_from_this传入智能指针,
        // 无需担心handleclient出去后,session对象被析构)----伪闭包机制
        session->Start(); // 服务端不会自动重连
    }
}

#endif