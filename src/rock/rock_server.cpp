#include "rock_server.h"
#include "rock_stream.h"
#include "../log.h"
#include "../module/module.h"
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