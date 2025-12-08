#include "../src/xftp/xftp_protocol.h"
#include "../src/xftp/xftp_session.h"
#include "../src/iomanager.h"
#include "../src/address.h"
int main()
{
    Xten::IOManager iom(2);
    Xten::Socket::ptr socket = Xten::Socket::CreateTCPSocket();
    socket->Connect(Xten::IPv4Address::Create("127.0.0.1", 8083));
    Xten::SocketStream::ptr ss(std::make_shared<Xten::SocketStream>(socket));
    Xten::xftp::XftpMessageDecoder::ptr decoder = std::make_shared<Xten::xftp::XftpMessageDecoder>();
    iom.Schedule([ss, decoder]()
                 {
        //发送文件包
        Xten::xftp::XftpRequest::ptr req(new Xten::xftp::XftpRequest());
        req->SetCmd(-1);
        req->SetFileName("test");
        for(int i=0;i<10000;i++)
        {
            std::cout<<"i="<<i<<std::endl;
            req->SetSn(i);
            std::string data("file block sn=");
            data+=std::to_string(i);
            data+="\r\n";
            req->SetData(data);
            decoder->SerializeToStream(ss,req);
            usleep(100*1000);
        } });
    iom.Schedule([ss, decoder]()
                 {
        int sn=0;
        while(true)
        {
            auto msg=decoder->ParseFromStream(ss);
            Xten::xftp::XftpResponse::ptr rsp=std::dynamic_pointer_cast<Xten::xftp::XftpResponse>(msg);
            if(!rsp)
                break;
             if(rsp->GetSn()<sn)
             {
                std::cout<<"error"<<std::endl;
                break;
             }
             std::cout<<"rsp="<<rsp->ToString()<<rsp->GetResultStr()<<std::endl;
             sn=rsp->GetSn();
    } });
    return 0;
}