#ifndef __XTEN_STATUS_SERVLET_H__
#define __XTEN_STATUS_SERVLET_H__
#include "../servlet.h"
namespace Xten
{
    namespace http
    {
        class StatusServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<StatusServlet> ptr;
            StatusServlet();
            virtual int32_t handle(Xten::http::HttpRequest::ptr request, Xten::http::HttpResponse::ptr response,
                                   Xten::SocketStream::ptr session) override;
        };
    }
}
#endif