#include"../include/Xten.h"
#include"../src/kcp/kcp_util.hpp"

using namespace Xten;
using namespace Xten::kcp;
int main()
{
    auto socket=Socket::CreateUDPSocket();
    Address::ptr to=IPv4Address::Create("127.0.0.1",8080);
    socket->SendTo(KcpUtil::making_connect_packet().c_str(),KcpUtil::making_connect_packet().length(),to);
    char buffer[1024];
    Address::ptr from=std::make_shared<IPv4Address>();
    socket->RecvFrom(buffer,1023,from);
    std::cout<<"ret="<<buffer<<std::endl;
    return 0;
}