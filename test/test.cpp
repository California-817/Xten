#include "../include/Xten.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <sys/epoll.h>
static Xten::Logger::ptr g_logger = XTEN_LOG_ROOT();
int sock = 0;
void test_iomanager() // 也是在协程中调用
{
    XTEN_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    // sleep(3);

    // close(sock);
    // sylar::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, O_NONBLOCK | flags);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr *)&addr, sizeof(addr)))
    {
    }
    else if (errno == EINPROGRESS)
    {
        XTEN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        Xten::IOManager::GetThis()->AddEvent(sock, Xten::IOManager::Event::READ, []()
                                             { XTEN_LOG_INFO(g_logger) << "read callback"; });
        Xten::IOManager::GetThis()->AddEvent(sock, Xten::IOManager::Event::WRITE, []()
                                             {
            XTEN_LOG_INFO(g_logger) << "write callback";
            Xten::IOManager::GetThis()->CancelEvent(sock, Xten::IOManager::Event::READ);close(sock); });
    }
    else
    {
        XTEN_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}
void test_hook()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    XTEN_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));
    XTEN_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if (rt)
    {
        return;
    }

    const char data[] = "GET /register HTTP/1.1\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    XTEN_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    XTEN_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    buff.resize(rt);
    XTEN_LOG_INFO(g_logger) << buff;
}
void test_socket()
{
    Xten::IPAddress::ptr addr = Xten::Address::LookupAnyIPAddress("www.baidu.com:80");
    if (addr)
    {
        XTEN_LOG_DEBUG(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        XTEN_LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    Xten::Socket::ptr sock = Xten::Socket::CreateTCP(addr);
    if (!sock->Connect(addr))
    {
        XTEN_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    }
    else
    {
        XTEN_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = Xten::TimeUitl::GetCurrentMS();
    for (size_t i = 0; i < 10000000000ul; ++i)
    {
        if (int err = sock->GetError())
        {
            XTEN_LOG_INFO(g_logger) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        // struct tcp_info tcp_info;
        // if(!sock->getOption(IPPROTO_TCP, TCP_INFO, tcp_info)) {
        //     SYLAR_LOG_INFO(g_looger) << "err";
        //     break;
        // }
        // if(tcp_info.tcpi_state != TCP_ESTABLISHED) {
        //     SYLAR_LOG_INFO(g_looger)
        //             << " state=" << (int)tcp_info.tcpi_state;
        //     break;
        // }
        static int batch = 10000000;
        if (i && (i % batch) == 0)
        {
            uint64_t ts2 = Xten::TimeUitl::GetCurrentMS();
            XTEN_LOG_INFO(g_logger) << "10000000 use time:" << (ts2 - ts) << " ms";
            XTEN_LOG_INFO(g_logger) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " ms";
            ts = ts2;
            break;
        }
    }
}
void test_socket2()
{
    Xten::IPAddress::ptr addr = Xten::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr)
    {
        XTEN_LOG_INFO(g_logger) << "get address: " << addr->toString();
    }
    else
    {
        XTEN_LOG_ERROR(g_logger) << "get address fail";
        return;
    }
    int i = 0;
    while (true)
    {
        if ((i++) == 50)
        {
            break;
        }
        sleep(2);
        Xten::Socket::ptr sock = Xten::Socket::CreateTCP(addr);
        addr->setPort(80);
        XTEN_LOG_INFO(g_logger) << "addr=" << addr->toString();
        if (!sock->Connect(addr))
        {
            XTEN_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
            return;
        }
        else
        {
            XTEN_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
        }

        const char buff[] = "GET / HTTP/1.0\r\n\r\n";
        int rt = sock->Send(buff, sizeof(buff));
        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "send fail rt=" << rt;
            return;
        }

        std::string buffs;
        buffs.resize(4096);
        rt = sock->Recv(&buffs[0], buffs.size());

        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "recv fail rt=" << rt;
            return;
        }

        buffs.resize(rt);
        XTEN_LOG_INFO(g_logger) << buffs;
    }
}
void handle_client(Xten::Socket::ptr client)
{
    while (true)
    {
        std::string buffs;
        buffs.resize(4096);
        int rt = client->Recv(&buffs[0], buffs.size());
        if (rt <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "recv fail rt=" << rt;
            return;
        }
        // std::cout << "client:: " << buffs << std::endl;
        int i = 0;
        char ret[1024];
        snprintf(ret, 1024, "server recv:%d", i);
        int rt2 = client->Send(ret, 1024);
        if (rt2 <= 0)
        {
            XTEN_LOG_INFO(g_logger) << "send fail rt=" << rt;
            return;
        }
    }
}
void test_server()
{
    // Xten::Socket::ptr apt = Xten::Socket::CreateTCPSocket();
    auto addr = Xten::IPv4Address::Create("127.0.0.1", 8080);
    Xten::TcpServer::ptr tcp_server(new Xten::TcpServer());
    tcp_server->Bind(addr);
    tcp_server->Start();
    // apt->Bind(addr);
    // apt->Listen();
    // while (true)
    // {
    // Xten::Socket::ptr client = apt->Accept();
    // if(!client)
    // {
    // return;
    // }
    // Xten::Scheduler::GetThis()->Schedule(std::bind(&handle_client, client));
    // }
}
void test_assert()
{
    XTEN_ASSERT(false);
}
void test_byteArray()
{
#define XX(type, len, write_fun, read_fun, base_len)               \
    {                                                              \
        std::vector<type> vec;                                     \
        for (int i = 0; i < len; ++i)                              \
        {                                                          \
            vec.push_back(rand());                                 \
        }                                                          \
        Xten::ByteArray::ptr ba(new Xten::ByteArray(base_len));    \
        for (auto &i : vec)                                        \
        {                                                          \
            ba->write_fun(i);                                      \
        }                                                          \
        ba->SetPosition(0);                                        \
        for (size_t i = 0; i < vec.size(); ++i)                    \
        {                                                          \
            type v = ba->read_fun();                               \
            XTEN_ASSERT(v == vec[i]);                              \
        }                                                          \
        XTEN_ASSERT(ba->GetReadSize() == 0);                       \
        XTEN_LOG_INFO(g_logger) << #write_fun "/" #read_fun        \
                                              " (" #type " ) len=" \
                                << len                             \
                                << " base_len=" << base_len        \
                                << " size=" << ba->GetSize();      \
    }

    XX(int8_t, 100, WriteFint8, ReadFint8, 1);
    XX(uint8_t, 100, WriteFUint8, ReadFUint8, 1);
    XX(int16_t, 100, WriteFint16, ReadFint16, 1);
    XX(uint16_t, 100, WriteFUint16, ReadFUint16, 1);
    XX(int32_t, 100, WriteFint32, ReadFint32, 1);
    XX(uint32_t, 100, WriteFUint32, ReadFUint32, 1);
    XX(int64_t, 100, WriteFint64, ReadFint64, 1);
    XX(uint64_t, 100, WriteFUint64, ReadFUint64, 1);

    XX(int32_t, 100, WriteVarint32, ReadVarint32, 1);
    XX(uint32_t, 100, WriteVarUint32, ReadVarUint32, 1);
    XX(int64_t, 100, WriteVarint64, ReadVarint64, 1);
    XX(uint64_t, 100, WriteVarUint64, ReadVarUint64, 1);
#undef XX
#define XX(type, len, write_fun, read_fun, base_len)                                         \
    {                                                                                        \
        std::vector<type> vec;                                                               \
        for (int i = 0; i < len; ++i)                                                        \
        {                                                                                    \
            vec.push_back(rand());                                                           \
        }                                                                                    \
        Xten::ByteArray::ptr ba(new Xten::ByteArray(base_len));                              \
        for (auto &i : vec)                                                                  \
        {                                                                                    \
            ba->write_fun(i);                                                                \
        }                                                                                    \
        ba->SetPosition(0);                                                                  \
        for (size_t i = 0; i < vec.size(); ++i)                                              \
        {                                                                                    \
            type v = ba->read_fun();                                                         \
            XTEN_ASSERT(v == vec[i]);                                                        \
        }                                                                                    \
        XTEN_ASSERT(ba->GetReadSize() == 0);                                                 \
        XTEN_LOG_INFO(g_logger) << #write_fun "/" #read_fun                                  \
                                              " (" #type " ) len="                           \
                                << len                                                       \
                                << " base_len=" << base_len                                  \
                                << " size=" << ba->GetSize();                                \
        ba->SetPosition(0);                                                                  \
        XTEN_ASSERT(ba->WriteToFile("/tmp/" #type "_" #len "-" #read_fun ".dat.test"));      \
        Xten::ByteArray::ptr ba2(new Xten::ByteArray(base_len * 2));                         \
        XTEN_ASSERT(ba2->ReadFromFile("/tmp/" #type "_" #len "-" #read_fun ".dat.test"));    \
        ba2->SetPosition(0);                                                                 \
        XTEN_ASSERT(ba2->WriteToFile("/tmp/" #type "_" #len "-" #read_fun ".dat.bak.test")); \
        ba2->SetPosition(0);                                                                 \
        std::cout << "same begin" << std::endl;                                              \
        std::cout << ba->ToHexString() << std::endl;                                         \
        std::cout << ba2->ToHexString() << std::endl;                                        \
        XTEN_ASSERT(ba->ToString() == ba2->ToString());                                      \
        std::cout << "same end" << std::endl;                                                \
        XTEN_ASSERT(ba->GetPosition() == 0);                                                 \
        XTEN_ASSERT(ba2->GetPosition() == 0);                                                \
    }
    XX(int8_t, 100, WriteFint8, ReadFint8, 3);
    XX(uint8_t, 100, WriteFUint8, ReadFUint8, 3);
    XX(int16_t, 100, WriteFint16, ReadFint16, 3);
    XX(uint16_t, 100, WriteFUint16, ReadFUint16, 1);
    XX(int32_t, 100, WriteFint32, ReadFint32, 1);
    XX(uint32_t, 100, WriteFUint32, ReadFUint32, 1);
    XX(int64_t, 100, WriteFint64, ReadFint64, 1);
    XX(uint64_t, 100, WriteFUint64, ReadFUint64, 1);

    XX(int32_t, 100, WriteVarint32, ReadVarint32, 1);
    XX(uint32_t, 100, WriteVarUint32, ReadVarUint32, 5);
    XX(int64_t, 100, WriteVarint64, ReadVarint64, 1);
    XX(uint64_t, 100, WriteVarUint64, ReadVarUint64, 1);

#undef XX
}
void test_sslSocket()
{
    Xten::SSLSocket::ptr sslsk = Xten::SSLSocket::CreateTCPSocket();
    auto addr = Xten::IPv4Address::Create("0.0.0.0", 8080);
    sslsk->Bind(addr);
    sslsk->Listen();
    sslsk->Accept();
    std::cout << sslsk << std::endl;
}
const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                 "Host: www.sylar.top\r\n"
                                 "Content-Leng";

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                  "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
                                  "Server: Apache\r\n"
                                  "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Content-Length: 81\r\n"
                                  "Cache-Cont";
const char buffer[] =
    "th: 10\r\n\r\n"
    "1234567890\r\n";
// "rol: max-age=86400\r\n"
// "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
// "Connection: Close\r\n"
// "Content-Type: text/html\r\n\r\n"
// "<html>\r\n"
// "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
// "</html>\r\n";
// 通过这个函数限制读取的长度为\r\n结尾
static int GetLastRN(const char *begin, int len)
{
    // 找到这一段数据的最后的\r\n
    static const char *sub_str = "\r\n";
    if (len < 2)
    {
        return -1;
    }
    for (const char *i = begin + len - 2; i >= begin; i--)
    {
        if (strncmp(i, sub_str, (size_t)2) == 0)
        {
            // 找到了
            return i - begin + 2;
        }
        // 没找到继续向前查找
    }
    // 没有\r\n
    return -1;
}
void test_http_req()
{
    Xten::http::HttpRequestParser parser;
    int i = 2;
    std::string tmp = test_request_data;
    while (i--)
    {
        if (i == 0)
        {
            tmp += buffer;
            std::cout << tmp << std::endl;
        }
        std::cout << GetLastRN(&tmp[0], tmp.size()) << std::endl;
        size_t s = parser.Execute(&tmp[0], tmp.size(), GetLastRN(&tmp[0], tmp.size()));
        XTEN_LOG_ERROR(g_logger) << "execute rt=" << s
                                 << "has_error=" << parser.HasError()
                                 << " is_finished=" << parser.IsFinished()
                                 << " total=" << tmp.size()
                                 << " content_length=" << parser.GetBodyLength()
                                 << "tmp[s]" << tmp[s];
        // tmp.resize(tmp.size() - s);
        // size_t s2 = parser.Execute(&tmp2[0], tmp2.size());
        // XTEN_LOG_ERROR(g_logger) << "execute rt=" << s2
        //  << "has_error=" << parser.HasError()
        //  << " is_finished=" << parser.IsFinished()
        //  << " total=" << tmp2.size()
        //  << " content_length=" << parser.GetBodyLength();
        tmp.resize(tmp.size() - s);
        XTEN_LOG_INFO(g_logger) << parser.GetRequest()->toString();
        XTEN_LOG_INFO(g_logger) << tmp;
    }
}
void test_http_rsp()
{
    Xten::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    std::cout << sizeof(test_response_data) << test_response_data[tmp.size() - 1];
    std::string tmp2 = buffer;
    int std_len = GetLastRN(&tmp[0], tmp.size());
    std::cout << std_len << std::endl;
    size_t s = parser.Execute(&tmp[0], std_len, false);
    XTEN_LOG_ERROR(g_logger) << "execute rt=" << s
                             << " has_error=" << parser.HasError()
                             << " is_finished=" << parser.IsFinished()
                             << " total=" << tmp.size()
                             << " content_length=" << parser.GetBodyLength()
                             << " tmp[s]=" << tmp[s];
    std::cout << "nread:" << httpclient_parser_nread(&parser.GetParser()) << std::endl;
    size_t s2 = parser.Execute(&tmp2[0], tmp2.size(), false);
    XTEN_LOG_ERROR(g_logger) << "execute rt=" << s2
                             << " has_error=" << parser.HasError()
                             << " is_finished=" << parser.IsFinished()
                             << " total=" << tmp2.size()
                             << " content_length=" << parser.GetBodyLength()
                             << " tmp[s]=" << tmp2[s2];
    tmp.resize(tmp.size() - s);
    XTEN_LOG_INFO(g_logger) << parser.GetResponse()->toString();
    XTEN_LOG_INFO(g_logger) << tmp;
}
void test_http_session()
{
    auto addr = Xten::IPv4Address::Create("0.0.0.0", 8080);
    auto socket = Xten::Socket::CreateTCPSocket();
    socket->Bind(addr);
    socket->Listen();
    auto newsocket = socket->Accept();
    Xten::http::HttpSession session(newsocket);
    while (true)
    {
        std::cout << "loop" << std::endl;
        auto req = session.RecvRequest();
        std::cout << "end read" << std::endl;
        // std::cout<<Xten::http::HttpMethodToString(req->getMethod())<<std::endl;
        std::cout << req->toString() << std::endl;
        std::cout << "-----------------------" << std::endl;
        Xten::http::HttpResponse::ptr rsp = req->createResponse();
        rsp->setBody("hello Xten/http/1.1");
        std::cout << rsp->toString() << std::endl;
        std::cout << "-----------------------" << std::endl;
        int ret = session.SendResponse(rsp);
        std::cout << "send size" << ret << std::endl;
    }
}
void test_http_server()
{
    Xten::http::HttpServer::ptr server(new Xten::http::HttpServer());
    auto addr = Xten::IPv4Address::Create("0.0.0.0", 8080);

    while(!server->Bind(addr)){
        continue;
    }
    server->Start();
}
int main()
{
    // test_assert();
    // Xten::Config::LoadFromConFDir(".");
    Xten::IOManager iom(2);
    iom.Schedule(&test_http_server);
    // test_byteArray();
    // test_sslSocket();
    // Xten::TimerManager mgr;
    //     XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add";

    // tm=mgr.AddTimer(5000,[&mgr](){
    //     XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"hello";
    //     mgr.AddTimer(3000,[](){
    //         XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add other timer";

    //     },false);
    // },true);
    // int i=0;
    // while(true)
    // {
    //    usleep(10000);
    //     std::vector<std::function<void()>> cbs;
    //     mgr.ListAllExpireCb(cbs);
    //     for(auto& e:cbs)
    //     {
    //         e();
    //     }
    // }
    // std::cout<<"main"<<std::endl;
    // Xten::Thread::SetName("main_thread");
    // Xten::Fiber::GetThis();

    // XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello log";
    // XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fmt");
    // XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system";
    // Xten::Fiber::ptr fiber(Xten::NewFiber(0,
    //     [](){
    //         Xten::Thread::SetName("fiber_thread");
    //         XTEN_LOG_DEBUG(XTEN_LOG_ROOT())<<"hello fiber log";
    //         XTEN_LOG_FMT_DEBUG(XTEN_LOG_ROOT(),"hello %s","fiber fmt");
    //         XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber";
    //         Xten::Fiber::GetThis()->Back();
    //         XTEN_LOG_INFO(XTEN_LOG_NAME("system"))<<"system in fiber222222";
    //     }
    //     ,true
    // ),Xten::FreeFiber);
    // fiber->Call();
    // std::cout<<"retrun"<<std::endl;
    // fiber->Call();
    // std::cout<<"retrun again"<<std::endl;
    // std::cout<<Xten::LoggerManager::GetInstance()->GetLogger("system");
    return 0;
}