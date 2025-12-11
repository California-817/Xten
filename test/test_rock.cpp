#include "../src/iomanager.h"
#include "../src/address.h"
#include "../include/Xten.h"
int main()
{
    Xten::IOManager iom(8);
    Xten::Socket::ptr socket = Xten::Socket::CreateTCPSocket();
    // Xten::RockConnection::ptr conn(std::make_shared<Xten::RockConnection>());
    // conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    Xten::RockConnection::ptr conn(new Xten::RockConnection);
    conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    conn->Start();
    for (int i = 0; i < 50; ++i)
    {
        // Xten::RockConnection::ptr conn(new Xten::RockConnection);
        // conn->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
        // conn->Connect(addr);
        Xten::IOManager::GetThis()->addTimer(1000, [conn, i]()
                                             {
                    Xten::RockRequest::ptr req(new Xten::RockRequest);
                    req->SetCmd(100);

                    auto rt = conn->Request(req, 100);
                    std::cout << "[result="
                        << rt->resultCode << " response="
                        << (rt->response ? rt->response->ToString() : "null")
                        << "usedTIme="<<rt->usedTime <<"ms ]"<<std::endl; }, true);
    }
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