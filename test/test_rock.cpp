#include "../src/iomanager.h"
#include "../src/address.h"
#include "../include/Xten.h"
#include "../src/objpool.h"
#include "../src/kcp/kcp_listener.h"
int main()
{
    Xten::Logger::ptr logger = XTEN_LOG_NAME("system");
    // logger->SetLevelLimit(Xten::LogLevel::INFO);
    Xten::IOManager iom(1);
    auto addr=Xten::IPv4Address::Create("0.0.0.0",8080);
    Xten::kcp::KcpListener::ptr listener=std::make_shared<Xten::kcp::KcpListener>(addr);
    listener->Listen();
    listener->Accept();




    // 计算协程创建和销毁的开销
    // auto start = Xten::TimeUitl::GetCurrentMS();
    // for (int i = 0; i < 50; i++)
    // {
    //     iom.Schedule([&iom]()
    //                  {
    //     for (int i = 0; i < 1000000; i++)
    //     {
    //         usleep(2000);
    //         iom.Schedule(std::shared_ptr<Xten::Fiber>(Xten::NewFiberFromObjPool(0, []()
    //                                                                             {
    //                                                                                 // usleep(1000);
    //                                                                                 // std::cout << "fiber=" << Xten::Fiber::GetThis()->GetFiberId() << std::endl;
    //                                                                             }),
    //                                                   Xten::FreeFiberToObjPool));
    //         // iom.Schedule([]()
    //                     //  {
    //             // usleep(1000);
    //             // std::cout << "fiber=" << Xten::Fiber::GetThis()->GetFiberId() << std::endl;  
    //             // });
    //     } });
    // }
    // iom.Stop();
    // auto end = Xten::TimeUitl::GetCurrentMS();
    // std::cout << "use time=" << end - start << "ms" << std::endl;
    // std::cout << Xten::FiberObjPoolInfo();
    // Xten::Socket::ptr socket = Xten::Socket::CreateTCPSocket();
    // // Xten::RockConnection::ptr conn(std::make_shared<Xten::RockConnection>());
    // // conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    // Xten::RockConnection::ptr conn(new Xten::RockConnection);
    // conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    // conn->Start();
    // for (int i = 0; i < 50; ++i)
    // {
    //     // Xten::RockConnection::ptr conn(new Xten::RockConnection);
    //     // conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    //     // conn->Connect(addr);
    //     Xten::IOManager::GetThis()->addTimer(1000, [conn, i]()
    //                                          {
    //                 Xten::RockRequest::ptr req(new Xten::RockRequest);
    //                 req->SetCmd(100);

    //                 auto rt = conn->Request(req, 100);
    //                 std::cout << "[result="
    //                     << rt->resultCode << " response="
    //                     << (rt->response ? rt->response->ToString() : "null")
    //                     << "usedTIme="<<rt->usedTime <<"ms ]"<<std::endl; }, true);
    // }
    // iom.addTimer(1000, [=]()
    //              {
    //     Xten::RockRequest::ptr req(new Xten::RockRequest);
    //     static uint32_t s_sn = 0;
    //     req->SetSn(++s_sn);
    //     req->SetCmd(100);
    //     req->SetData("hello world sn=" + std::to_string(s_sn));

    //     auto rsp = conn->Request(req, 300);
    //     if(rsp)
    //         std::cout<<rsp->usedTime<<":"<<rsp->toString()<<std::endl;
    //              }, true);

    return 0;
}