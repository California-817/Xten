#ifndef __XTEN_HTTP_SESSION_H__
#define __XTEN_HTTP_SESSION_H__
#include "../streams/socket_stream.h"
#include "http.h"
#include "http_parser.h"
namespace Xten
{
    namespace http
    {
        class HttpSession : public SocketStream
        {
        public:
            typedef std::shared_ptr<HttpSession> ptr;
            HttpSession(Socket::ptr socket, bool is_owner = true);
            virtual ~HttpSession()=default;
            // 接受一个完整http请求并生成http请求结构体
            HttpRequest::ptr RecvRequest();
            // 发送一个完整http响应(ret<0失败)
            int SendResponse(HttpResponse::ptr response);
        };
    }
}
#endif