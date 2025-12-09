#include "../src/iomanager.h"
#include "../src/address.h"
#include "../include/Xten.h"
int main()
{
    Xten::IOManager iom(2);
    Xten::Socket::ptr socket = Xten::Socket::CreateTCPSocket();
    socket->Connect(Xten::IPv4Address::Create("127.0.0.1", 8062));
    Xten::RockSession::ptr ss(std::make_shared<Xten::RockSession>(socket));

    iom.addTimer(1000, [=]()
                 {
        Xten::RockRequest::ptr req(new Xten::RockRequest);
        static uint32_t s_sn = 0;
        req->SetSn(++s_sn);
        req->SetCmd(100);
        req->SetData("hello world sn=" + std::to_string(s_sn));

        auto rsp = ss->Request(req, 300);
        if(rsp->response) {
            std::cout<<rsp->usedTime<<":"<<rsp->response->ToString()<<std::endl;
        } }, true);
    return 0;
}