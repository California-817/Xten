#include "../include/http/http_session.h"
namespace Xten
{
    namespace http
    {
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
        HttpSession::HttpSession(Socket::ptr socket, bool is_owner)
            : SocketStream(socket, is_owner)
        {
        }
        // 接受一个完整http请求并生成http请求结构体
        HttpRequest::ptr HttpSession::RecvRequest()
        {
            // 创建解析请求结构
            HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
            uint32_t buffer_size = HttpRequestParser::GetHttpReqMaxBufferSize();
            std::shared_ptr<char> buffer = std::shared_ptr<char>(new char[buffer_size], [](char *ptr)
                                                                 { delete[] ptr; });
            char *data = buffer.get();
            uint32_t offset = 0;
            // 解析http请求头
            do
            {
                int len = Read(data + offset, buffer_size - offset);
                if (len <= 0)
                {
                    Close();
                    return nullptr;
                }
                // 开始进行解析请求
                int std_len = GetLastRN(data, len + offset);
                if (std_len < 0)
                {
                    // 这次读取没有\r\n
                    offset = offset + len; // 更新offset
                    continue;
                }
                size_t nparse = parser->Execute(data, len + offset, std_len);
                if (parser->HasError())
                {
                    // 解析出错
                    Close();
                    return nullptr;
                }
                // 更新offset
                offset = offset + len - nparse;
                if (offset >= buffer_size)
                {
                    // 到了缓冲区末尾，不能再接受请求数据
                    Close();
                    return nullptr;
                }
                if (parser->IsFinished() == 1)
                {
                    // 解析完成
                    break;
                }
            } while (true);
            // 从头部字段获取body长度
            uint32_t body_size = parser->GetBodyLength();
            if (body_size > parser->GetHttpReqMaxBodySize())
            {
                // 恶意请求
                Close();
                return nullptr;
            }
            //请求头中是否有：Expect: 100-continue（客户端等待服务器确认后再发送请求体）
            std::string except=parser->GetRequest()->getHeader("Expect");
            if(strcasecmp(except.c_str(),"100-continue")==0)
            {
                //客户端先发请求头，待服务端确定后再决定是否发生请求体
                static const char* s_data="HTTP/1.1 100 Continue\r\n\r\n";
                ssize_t ret=WriteFixSize(s_data,strlen(s_data));
                if(ret<=0)
                {
                    Close();
                    return nullptr;
                }
                parser->GetRequest()->delHeader("Expect");
            }
            if (body_size > 0)
            {
                // 开始解析body
                std::string body;
                body.resize(body_size);
                int copyed = 0; // 已拷贝数据量
                // 请求解析后残留字段
                if (offset > body_size)
                {
                    //残留字段大于body长度
                    memcpy(&body[copyed], data, body_size);
                    copyed+=body_size;
                }
                else
                {
                    //残留字段小于body长度
                    memcpy(&body[copyed],data,offset);
                    copyed+=offset;
                }
                body_size-=copyed;
                if(body_size>0)
                {
                    //再次读取剩余长度body
                    ssize_t ret=ReadFixSize(&body[copyed],body_size);
                    if(ret<=0)
                    {
                        Close();
                        return nullptr;
                    }
                }
                //将body放入请求结构体
                parser->GetRequest()->setBody(body);
            }
            //读取一个完整请求
            parser->GetRequest()->init();
            parser->GetRequest()->initParam();
            return parser->GetRequest();
        }
        // 发送一个完整http响应
        int HttpSession::SendResponse(HttpResponse::ptr response)
        {
            std::string str = response->toString();
            return WriteFixSize(str.c_str(), str.size());
        }
    }
}