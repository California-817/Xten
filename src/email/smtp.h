#ifndef __XTEN_EMAIL_SMTP_H__
#define __XTEN_EMAIL_SMTP_H__

#include "../streams/socket_stream.h"
#include "email.h"
#include <sstream>

namespace Xten {

//smtp命令执行结果
struct SmtpResult {
    typedef std::shared_ptr<SmtpResult> ptr;
    enum Result {
        OK = 0,
        IO_ERROR = -1
    };

    SmtpResult(int r, const std::string& m)
        :result(r)
        ,msg(m) {
    }

    int result;
    std::string msg;
};

//smtp客户端实现
class SmtpClient : public Xten::SocketStream {
public:
    typedef std::shared_ptr<SmtpClient> ptr;
    //静态工厂方法创建smtp客户端对象
    static SmtpClient::ptr Create(const std::string& host, uint32_t port, bool ssl= false);
    //发送邮件接口
    SmtpResult::ptr Send(EMail::ptr email, int64_t timeout_ms = 1000, bool debug = false);
    //获取邮件发送cmd的调试信息
    std::string GetDebugInfo();
private:
    //出错返回非空结果，正常返回nullptr---->与服务端的一次请求响应
    SmtpResult::ptr doCmd(const std::string& cmd, bool debug);
protected:
    //构造函数保护化，防止外部任意创建，只可以根据静态工厂方法创建
    SmtpClient(Socket::ptr sock);
private:
    std::string m_host; //smtp服务器地址
    std::stringstream m_ss; //debug info
    bool m_authed = false; //是否已经认证通过
};

}

#endif
