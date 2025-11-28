#include "../src/Xten.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iomanip>
#include <bitset>
#include <iostream>
#include <thread>
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

    while (!server->Bind(addr))
    {
        continue;
    }
    server->Start();
}
void test_websocker_session()
{
    Xten::http::WSFrameHead head;
    head.fin = 1;
    head.rsv1 = 0;
    head.rsv2 = 1;
    head.rsv3 = 0;
    head.opcode = Xten::http::WSFrameHead::OPCODE::PING;
    head.mask = 1;
    head.payload = 126;
    std::cout << "size" << sizeof(head) << std::endl;
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&head);
    for (size_t i = 0; i < sizeof(head); ++i)
    {
        std::bitset<8> bits(bytes[i]);
        std::cout << "Byte " << i << ": " << bits << std::endl;
    }
}
void test_websocket_server()
{
    Xten::http::WSServer::ptr ws = std::make_shared<Xten::http::WSServer>();
    auto addr = Xten::IPv4Address::Create("0.0.0.0", 8080);

    while (!ws->Bind(addr))
    {
    }
    ws->Start();
}
static std::atomic<int> count = 0;
void test_queue()
{
    Xten::IOManager sche(5, false);
    std::thread t1 = std::thread([&sche]()
                                 {
 for (int i = 0; i < 300000; i++)
 {
     sche.Schedule([]()
                   { count++; });
 } });
    std::thread t2 = std::thread([&sche]()
                                 {
 for (int i = 0; i < 300000; i++)
 {
     sche.Schedule([]()
                   { count++; });
 } });
    std::thread t3 = std::thread([&sche]()
                                 {
 for (int i = 0; i < 400000; i++)
 {
     sche.Schedule([]()
                   {  count++; });
 } });
    t1.join();
    t2.join();
    t3.join();
    for (int i = 0; i < 3; i++)
    {
        std::cout << "queuesize: " << std::endl;
        std::cout << "local size " << std::endl;
    }
}
static Xten::Timer::ptr timer;
static int i=0;
int test(int argc, char **argv)
{
    Xten::Env::GetInstance()->AddHelp("s","sss");
    Xten::Env::GetInstance()->AddHelp("a","aaa");
    Xten::Env::GetInstance()->AddHelp("b","bbb");
    Xten::Env::GetInstance()->AddHelp("c","ccc");
    bool ret=Xten::Env::GetInstance()->Init(argc,argv);
    if(!ret)
    {
        //解析失败
        std::cout<<Xten::Env::GetInstance()->PrintHelps();
        return -1;
    }
    else
    {
        //成功
        std::cout<<"解析成功 Xten框架启动"<<std::endl;
        std::cout<<Xten::Env::GetInstance()->GetExe()<<std::endl;
        std::cout<<Xten::Env::GetInstance()->GetCwd()<<std::endl;
        std::cout<<Xten::Env::GetInstance()->GetConfigPath()<<std::endl;
    }
    return 0;
}
// 性能测试：递归并行快速排序 + 混合任务负载
class HighPerformanceTest {
private:
    std::atomic<uint64_t> m_taskCount{0};
    std::atomic<uint64_t> m_completedTasks{0};
    std::atomic<uint64_t> m_totalLatency{0};
    
public:
    // 测试1：递归分治算法（最适合工作窃取）
    void testParallelQuickSort(Xten::Scheduler& scheduler, std::vector<int>& data) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::atomic<bool> done{false};
        parallelQuickSort(scheduler, data, 0, data.size() - 1, 0, done);
        
        // 等待完成
        while(!done.load()) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        XTEN_LOG_INFO(g_logger) << "QuickSort completed in: " << duration.count() << "ms";
        XTEN_LOG_INFO(g_logger) << "Tasks created: " << m_taskCount.load();
    }
    
private:
    void parallelQuickSort(Xten::Scheduler& scheduler, std::vector<int>& arr, 
                          int left, int right, int depth, std::atomic<bool>& done) {
        if(left >= right) {
            if(depth == 0) done.store(true);
            return;
        }
        
        m_taskCount++;
        
        if(depth < 6) { // 前6层并行化，产生不均匀负载
            std::atomic<int> subTaskCount{2};
            
            int pivot = partition(arr, left, right);
            
            // 左子任务
            scheduler.Schedule([this, &scheduler, &arr, left, pivot, depth, &subTaskCount, &done](){
                parallelQuickSort(scheduler, arr, left, pivot - 1, depth + 1, done);
                if(subTaskCount.fetch_sub(1) == 1 && depth == 0) {
                    done.store(true);
                }
            });
            
            // 右子任务
            scheduler.Schedule([this, &scheduler, &arr, pivot, right, depth, &subTaskCount, &done](){
                parallelQuickSort(scheduler, arr, pivot + 1, right, depth + 1, done);
                if(subTaskCount.fetch_sub(1) == 1 && depth == 0) {
                    done.store(true);
                }
            });
        } else {
            // 深层递归转为串行，避免过度并行
            quickSortSerial(arr, left, right);
            if(depth == 0) done.store(true);
        }
    }
    
    int partition(std::vector<int>& arr, int left, int right) {
        int pivot = arr[right];
        int i = left - 1;
        
        for(int j = left; j < right; j++) {
            if(arr[j] <= pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[right]);
        return i + 1;
    }
    
    void quickSortSerial(std::vector<int>& arr, int left, int right) {
        if(left >= right) return;
        int pivot = partition(arr, left, right);
        quickSortSerial(arr, left, pivot - 1);
        quickSortSerial(arr, pivot + 1, right);
    }

public:
    // 测试2：混合负载场景（最能体现工作窃取优势）
    void testMixedWorkload(Xten::Scheduler& scheduler, int iterations) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::atomic<int> remainingTasks{iterations};
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> taskTypeDist(1, 4);
        std::uniform_int_distribution<> computeDist(1, 100);
        
        for(int i = 0; i < iterations; ++i) {
            int taskType = taskTypeDist(gen);
            int computeIntensity = computeDist(gen);
            
            switch(taskType) {
                case 1: // 快速任务 (1-5ms) - 70%概率
                    if(computeIntensity <= 70) {
                        scheduler.Schedule([this, &remainingTasks, i](){
                            auto taskStart = std::chrono::high_resolution_clock::now();
                            
                            // 轻量计算
                            volatile int sum = 0;
                            for(int j = 0; j < 10000; ++j) {
                                sum += j * j;
                            }
                            
                            auto taskEnd = std::chrono::high_resolution_clock::now();
                            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                                taskEnd - taskStart).count();
                            
                            m_totalLatency.fetch_add(latency);
                            m_completedTasks++;
                            remainingTasks--;
                        });
                    }
                    break;
                    
                case 2: // 中等任务 (10-50ms) - 20%概率  
                    if(computeIntensity > 70 && computeIntensity <= 90) {
                        scheduler.Schedule([this, &remainingTasks, i](){
                            auto taskStart = std::chrono::high_resolution_clock::now();
                            
                            // 中等计算 + 模拟IO
                            volatile int sum = 0;
                            for(int j = 0; j < 500000; ++j) {
                                sum += j * j;
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(5));
                            
                            auto taskEnd = std::chrono::high_resolution_clock::now();
                            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                                taskEnd - taskStart).count();
                            
                            m_totalLatency.fetch_add(latency);
                            m_completedTasks++;
                            remainingTasks--;
                        });
                    }
                    break;
                    
                case 3: // 重型任务 (100-200ms) - 8%概率
                    if(computeIntensity > 90 && computeIntensity <= 98) {
                        scheduler.Schedule([this, &remainingTasks, i](){
                            auto taskStart = std::chrono::high_resolution_clock::now();
                            
                            // 重型计算
                            volatile double result = 0.0;
                            for(int j = 0; j < 5000000; ++j) {
                                result += std::sin(j) * std::cos(j);
                            }
                            
                            auto taskEnd = std::chrono::high_resolution_clock::now();
                            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                                taskEnd - taskStart).count();
                            
                            m_totalLatency.fetch_add(latency);
                            m_completedTasks++;
                            remainingTasks--;
                        });
                    }
                    break;
                    
                case 4: // 超重型任务 (500ms+) - 2%概率
                    if(computeIntensity > 98) {
                        scheduler.Schedule([this, &remainingTasks, i](){
                            auto taskStart = std::chrono::high_resolution_clock::now();
                            
                            // 超重型计算
                            volatile double result = 0.0;
                            for(int j = 0; j < 20000000; ++j) {
                                result += std::sqrt(j) * std::log(j + 1);
                            }
                            
                            auto taskEnd = std::chrono::high_resolution_clock::now();
                            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
                                taskEnd - taskStart).count();
                            
                            m_totalLatency.fetch_add(latency);
                            m_completedTasks++;
                            remainingTasks--;
                        });
                    }
                    break;
            }
        }
        
        // 等待所有任务完成
        while(remainingTasks.load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        XTEN_LOG_INFO(g_logger) << "Mixed workload completed:";
        XTEN_LOG_INFO(g_logger) << "  Total time: " << totalTime.count() << "ms";
        XTEN_LOG_INFO(g_logger) << "  Completed tasks: " << m_completedTasks.load();
        XTEN_LOG_INFO(g_logger) << "  Average latency: " 
                                << (m_totalLatency.load() / m_completedTasks.load()) << "μs";
        XTEN_LOG_INFO(g_logger) << "  Throughput: " 
                                << (m_completedTasks.load() * 1000 / totalTime.count()) << " tasks/sec";
    }

    // 测试3：突发负载场景
    void testBurstWorkload(Xten::Scheduler& scheduler) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::atomic<int> remainingTasks{0};
        
        // 模拟3波突发负载
        for(int wave = 0; wave < 10; ++wave) {
            int burstSize = 10000 + wave * 5000;
            remainingTasks.fetch_add(burstSize);
            
            // 在短时间内提交大量任务
            for(int i = 0; i < burstSize; ++i) {
                scheduler.Schedule([this, &remainingTasks, wave, i](){
                    // 模拟不同复杂度的任务
                    int complexity = (wave * 1000 + i) % 100;
                    
                    volatile int sum = 0;
                    for(int j = 0; j < complexity * 1000; ++j) {
                        sum += j;
                    }
                    
                    m_completedTasks++;
                    remainingTasks--;
                });
            }
            
            // 波次间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 等待完成
        while(remainingTasks.load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        XTEN_LOG_INFO(g_logger) << "Burst workload completed in: " << duration.count() << "ms";
        XTEN_LOG_INFO(g_logger) << "Tasks processed: " << m_completedTasks.load();
    }
    
    void reset() {
        m_taskCount.store(0);
        m_completedTasks.store(0);
        m_totalLatency.store(0);
    }
};

// 性能对比测试
void performanceComparison() {
    const int THREAD_COUNT = std::thread::hardware_concurrency()*2;
    
    XTEN_LOG_INFO(g_logger) << "=== Performance Comparison (Threads: " << THREAD_COUNT << ") ===";
    
    // 测试工作窃取调度器
    {
        XTEN_LOG_INFO(g_logger) << "\n--- Work Stealing Scheduler ---";
        Xten::IOManager workStealScheduler(THREAD_COUNT, false, "work_steal");
        HighPerformanceTest test;
        
        // 测试1：快速排序
        // std::vector<int> data1(100000);
        // std::iota(data1.rbegin(), data1.rend(), 1); // 逆序数据，最坏情况
        // test.testParallelQuickSort(workStealScheduler, data1);
        
        // test.reset();
        
        // 测试2：混合负载
        // test.testMixedWorkload(workStealScheduler, 5);
        
        // test.reset();
        
        // 测试3：突发负载
        test.testBurstWorkload(workStealScheduler);
        
    }
}


Xten::RockConnection::ptr conn(new Xten::RockConnection);
void run()
{
    Xten::Address::ptr addr = Xten::Address::LookupAny("127.0.0.1:8061");
    if (!conn->Connect(addr))
    {
        XTEN_LOG_INFO(g_logger) << "connect " << *addr << " false";
    }

    Xten::IOManager::GetThis()->addTimer(1000, []()
                                         {
        // Ensure Request executes in a fiber: schedule the actual work
            Xten::RockRequest::ptr req(new Xten::RockRequest);
            static uint32_t s_sn = 0;
            req->SetCmd(100);
            req->SetData("hello world sn=" + std::to_string(s_sn++));

            auto rt = conn->Request(req,300);
            XTEN_LOG_INFO(g_logger) << "[result="
                        << rt->resultCode << " response="
                        << (rt->response ? rt->response->ToString() : "null")
                        << "usedTIme="<<rt->usedTime <<"ms ]";
    }, true);
    conn->Start();
}
void run2()
{
    Xten::Address::ptr addr = Xten::Address::LookupAny("127.0.0.1:8061");
    for (int i = 0; i < 500; ++i)
    {
        Xten::RockConnection::ptr conn(new Xten::RockConnection);
        conn->Connect(addr);
        Xten::IOManager::GetThis()->addTimer(2000, [conn, i]()
                                             {
                    Xten::RockRequest::ptr req(new Xten::RockRequest);
                    req->SetCmd(100);

                    auto rt = conn->Request(req, 100);
                    XTEN_LOG_INFO(g_logger) << "[result="
                        << rt->resultCode << " response="
                        << (rt->response ? rt->response->ToString() : "null")
                        << "usedTIme="<<rt->usedTime <<"ms ]"; }, true);
        conn->Start();
    }
}
int main(int argc, char **argv)
{
    // iom.addTimer(1000,[](){
    // std::cout<<"on timer"<<std::endl;
    // },true);
    // Xten::xten_start(argc,argv,&test,false);
     uint64_t begin = Xten::TimeUitl::GetCurrentMS();
     performanceComparison();
     uint64_t end = Xten::TimeUitl::GetCurrentMS();
     std::cout << "---------------------100万个任务总耗时----------------" << std::endl;
     std::cout << "---------------------" << end - begin << " ms--------------" << std::endl;
    //  test_assert();
    //  Xten::Config::LoadFromConFDir(".");
    //  Xten::IOManager iom(2);
    //  iom.Schedule(&test_websocket_server);
    //  test_byteArray();
    //  test_sslSocket();
    //  Xten::TimerManager mgr;
    //      XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"add";

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