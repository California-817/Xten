#ifndef __XTEN_WS_SERVLET_H__
#define __XTEN_WS_SERVLET_H__
#include "../http/servlet.h"
#include "ws_session.h"
namespace Xten
{
    namespace http
    {
        class WSServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<WSServlet> ptr;
            WSServlet(const std::string &name)
                : Servlet(name)
            {
            }
            virtual ~WSServlet() {}

            virtual int32_t handle(Xten::http::HttpRequest::ptr request, Xten::http::HttpResponse::ptr response,
                                   Xten::SocketStream::ptr session) override
            {
                return 0;
            }
            // 一个websocket链接建立时调用 （返回0成功，返回非0失败）
            virtual int32_t onConnect(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session) = 0;
            // 该websocket链接结束时调用
            virtual int32_t onClose(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session) = 0;
            // 返回0成功，返回非0失败
            virtual int32_t handle(Xten::http::HttpRequest::ptr header, Xten::http::WSFrameMessage::ptr msg,
                                   Xten::http::WSSession::ptr session) = 0;
            const std::string &getName() const { return m_name; }

        protected:
            std::string m_name;
        };

        class FunctionWSServlet : public WSServlet
        {
        public:
            typedef std::shared_ptr<FunctionWSServlet> ptr;
            typedef std::function<int32_t(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session)> on_connect_cb;
            typedef std::function<int32_t(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session)> on_close_cb;
            typedef std::function<int32_t(Xten::http::HttpRequest::ptr header, Xten::http::WSFrameMessage::ptr msg,
                                          Xten::http::WSSession::ptr session)>
                callback;

            FunctionWSServlet(callback cb, on_connect_cb connect_cb = nullptr, on_close_cb close_cb = nullptr);
            // 一个websocket链接建立时调用 （返回0成功，返回非0失败）
            virtual int32_t onConnect(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session) override;
            // 该websocket链接结束时调用
            virtual int32_t onClose(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session) override;
            // 返回0成功，返回非0失败
            virtual int32_t handle(Xten::http::HttpRequest::ptr header, Xten::http::WSFrameMessage::ptr msg,
                                   Xten::http::WSSession::ptr session) override;

        protected:
            callback m_callback;       // websocket链接成功的回调
            on_connect_cb m_onConnect; // 处理函数
            on_close_cb m_onClose;     // websocket链接关闭的回调
        };

        class WSServletDispatch : public ServletDispatch
        {
        public:
            typedef std::shared_ptr<WSServletDispatch> ptr;
            typedef RWMutex RWMutexType;

            WSServletDispatch();
            void addServlet(const std::string &uri, FunctionWSServlet::callback cb, FunctionWSServlet::on_connect_cb connect_cb = nullptr,
                            FunctionWSServlet::on_close_cb close_cb = nullptr);
            void addGlobServlet(const std::string &uri, FunctionWSServlet::callback cb, FunctionWSServlet::on_connect_cb connect_cb = nullptr,
                                FunctionWSServlet::on_close_cb close_cb = nullptr);
            WSServlet::ptr getWSServlet(const std::string &uri);
        };



        class TestServlet
        {
        public:
            static int32_t TestonConnect(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session)
            {
                std::cout << "on connected: " << header->toString() << std::endl;
                return 0;
            }
            // 该websocket链接结束时调用
            static int32_t TestonClose(Xten::http::HttpRequest::ptr header, Xten::http::WSSession::ptr session)
            {
                std::cout << "on close: " << header->toString() << std::endl;
                return 0;
            }
            // 返回0成功，返回非0失败
            static int32_t Testhandle(Xten::http::HttpRequest::ptr header, Xten::http::WSFrameMessage::ptr msg,
                                      Xten::http::WSSession::ptr session)
            {
                std::cout << "handle req: " << msg->GetData() << std::endl;
                session->SendMessage(msg->GetData() + "server", WSFrameHead::OPCODE::TEXT_FRAME, true);
                return 0;
            }
        };
    }

}
#endif