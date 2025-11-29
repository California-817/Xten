#include "smtp.h"
#include "../log.h"

/************* stmp客户端与服务端交互流程

步骤 方向	原始消息（含换行）	            关键含义	
1	←	`220 smtp.example.com ESMTP Postfix ready\r\n`	        服务就绪	
2	→	`EHLO [192.168.1.10]\r\n`	                            客户端自报家门	
3	←	`250-smtp.example.com\r\n250-PIPELINING\r\n250-AUTH LOGIN PLAIN\r\n250-STARTTLS\r\n250 8BITMIME\r\n`	支持扩展列表	
4	→	`STARTTLS\r\n`	                                        升 TLS	
5	←	`220 Ready to start TLS\r\n`	                        同意升 TLS	
6	→	`EHLO [192.168.1.10]\r\n`	                            加密后再自报一次	
7	←	`250-smtp.example.com\r\n250-AUTH LOGIN PLAIN\r\n250 8BITMIME\r\n`	    加密后的扩展列表	
8	→	`AUTH LOGIN\r\n`	                                    选 LOGIN 认证	
9	←	`334 VXNlcm5hbWU6\r\n`	                                请发用户名（base64）	
10	→	`dXNlckBleGFtcGxlLmNvbQ==\r\n`	                        `user@example.com`	
11	←	`334 UGFzc3dvcmQ6\r\n`	                                请发密码	
12	→	`c3Ryb25ncGFzcw==\r\n`	                                `strongpass`	
13	←	`235 Authentication successful\r\n`                 	认证通过	
14	→	`MAIL FROM:<user@example.com>\r\n`	                    声明发件人	
15	←	`250 2.1.0 Ok\r\n`	                                    发件人地址合法	
16	→	`RCPT TO:<alice@example.com>\r\n`	                    声明收件人	
17	←	`250 2.1.5 Ok\r\n`	                                    收件人地址合法	
18	→	`DATA\r\n`	                                            请求投递正文	
19	←	`354 End data with <CR><LF>.<CR><LF>\r\n`	            可以发，以“单独点”结束	
20	→	下文「二、整封邮件源码」连续发送	
21	→	`\r\n.\r\n`	结束标记（单独点）	
下面可选：
22	←	`250 2.0.0 Ok: queued as A1B2C3\r\n`	已入队	
23	→	`QUIT\r\n`	礼貌退出	
24	←	`221 2.0.0 Bye\r\n`	同意关闭	
25	TCP 四次挥手		连接结束	



完整的一篇邮件源码示例：
Date: Fri, 29 Nov 2025 16:04:23 +0800       ← 本地时间
From: user@example.com                  ← 发件人
To: alice@example.com                       ← 收件人
Subject: =?utf-8?b?5p2O5Lq655Sf54mp?=          ← 标题 base64「会议纪要」
MIME-Version: 1.0                      ← MIME 版本
Content-Type: multipart/mixed; boundary="000000" ← 多部分混合类型，分隔符 000000

--000000                    ← 分隔符开始
Content-Type: text/plain; charset=utf-8 ← 正文类型与编码
Content-Transfer-Encoding: 7bit

Alice 你好，                        正文内容
请查收附件中的会议纪要。
--000000            ← 分隔符开始
Content-Type: application/octet-stream          附件内容
Content-Disposition: attachment; filename*=utf-8''%E4%BC%9A%E8%AE%AE%E7%BA%AA%E8%A6%81.txt
Content-Transfer-Encoding: base64

6K6w5b+G5L2T55qE5oiQ5Yqf
--000000--   

.     结束标记

*/







namespace Xten {

static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");

SmtpClient::SmtpClient(Socket::ptr sock)
    :Xten::SocketStream(sock) {
}

SmtpClient::ptr SmtpClient::Create(const std::string& host, uint32_t port, bool ssl) {
    Xten::IPAddress::ptr addr = Xten::Address::LookupAnyIPAddress(host);
    if(!addr) {
        XTEN_LOG_ERROR(g_logger) << "invalid smtp server: " << host << ":" << port
            << " ssl=" << ssl;
        return nullptr;
    }
    addr->setPort(port);
    Socket::ptr sock;
    if(ssl) {
        sock = Xten::SSLSocket::CreateTCP(addr);
    } else {
        sock = Xten::Socket::CreateTCP(addr);
    }
    if(!sock->Connect(addr)) {
        XTEN_LOG_ERROR(g_logger) << "connect smtp server: " << host << ":" << port
            << " ssl=" << ssl << " fail";
        return nullptr;
    }
    std::string buf;
    buf.resize(1024);

    SmtpClient::ptr rt = Xten::protected_make_shared<SmtpClient>(sock);
    int len = rt->Read(&buf[0], buf.size());
    if(len <= 0) {
        return nullptr;
    }
    buf.resize(len);
    if(Xten::TypeUtil::Atoi(buf) != 220) {
        return nullptr;
    }
    rt->m_host = host;
    return rt;
}


SmtpResult::ptr SmtpClient::doCmd(const std::string& cmd, bool debug) {
    if(WriteFixSize(cmd.c_str(), cmd.size()) <= 0) {
        return std::make_shared<SmtpResult>(SmtpResult::IO_ERROR, "write io error");
    }
    std::string buf;
    buf.resize(4096);
    auto len = Read(&buf[0], buf.size());
    if(len <= 0) {
        return std::make_shared<SmtpResult>(SmtpResult::IO_ERROR, "read io error");
    }
    buf.resize(len);
    if(debug) {
        m_ss << "C: " << cmd;
        m_ss << "S: " << buf;
    }

    int code = Xten::TypeUtil::Atoi(buf);
    if(code >= 400) {
        return std::make_shared<SmtpResult>(code,
                Xten::replace(
                    buf.substr(buf.find(' ') + 1)
                    , "\r\n", ""));
    }
    return nullptr;
}

SmtpResult::ptr SmtpClient::Send(EMail::ptr email, int64_t timeout_ms, bool debug) {
#define DO_CMD() \
    result = doCmd(cmd, debug); \
    if(result) {\
        return result; \
    }

    Socket::ptr sock = GetSocket();
    if(sock) {
        sock->SetRecvTimeOut(timeout_ms);
        sock->SetSendTimeOut(timeout_ms);
    }

    SmtpResult::ptr result;
    std::string cmd = "EHLO " + m_host + "\r\n";
    DO_CMD();
    if(!m_authed && !email->getFromEMailAddress().empty()) {
        cmd = "AUTH LOGIN\r\n";
        DO_CMD();
        auto pos = email->getFromEMailAddress().find('@');
        cmd = Xten::base64encode(email->getFromEMailAddress().substr(0, pos)) + "\r\n";
        DO_CMD();
        cmd = Xten::base64encode(email->getFromEMailPasswd()) + "\r\n";
        DO_CMD();

        m_authed = true;
    }

    cmd = "MAIL FROM: <" + email->getFromEMailAddress() + ">\r\n";
    DO_CMD();
    std::set<std::string> targets;
#define XX(fun) \
    for(auto& i : email->fun()) { \
        targets.insert(i); \
    } 
    XX(getToEMailAddress);
    XX(getCcEMailAddress);
    XX(getBccEMailAddress);
#undef XX
    for(auto& i : targets) {
        cmd = "RCPT TO: <" + i + ">\r\n";
        DO_CMD();
    }

    cmd = "DATA\r\n";
    DO_CMD();

    auto& entitys = email->getEntitys();

    std::stringstream ss;
    ss << "From: <" << email->getFromEMailAddress() << ">\r\n"
       << "To: ";
#define XX(fun) \
        do {\
            auto& v = email->fun(); \
            for(size_t i = 0; i < v.size(); ++i) {\
                if(i) {\
                    ss << ","; \
                } \
                ss << "<" << v[i] << ">"; \
            } \
            if(!v.empty()) { \
                ss << "\r\n"; \
            } \
        } while(0);
    XX(getToEMailAddress);
    if(!email->getCcEMailAddress().empty()) {
        ss << "Cc: ";
        XX(getCcEMailAddress);
    }
    ss << "Subject: " << email->getTitle() << "\r\n";
    std::string boundary;
    ss << "MIME-Version: 1.0\r\n";
    if(!entitys.empty()) {
        boundary = Xten::random_string(16);
        ss << "Content-Type: multipart/mixed;boundary="
           << boundary << "\r\n";
    }
    if(!boundary.empty()) {
        ss << "\r\n--" << boundary << "\r\n";
    }
    ss << "Content-Type: text/html;charset=\"utf-8\"\r\n"
       << "\r\n"
       << email->getBody() << "\r\n";
    for(auto& i : entitys) {
        ss << "\r\n--" << boundary << "\r\n";
        ss << i->toString();
    }
    if(!boundary.empty()) {
        ss << "\r\n--" << boundary << "--\r\n";
    }
    ss << "\r\n.\r\n";
    cmd = ss.str();
    DO_CMD();
#undef XX
#undef DO_CMD
    return std::make_shared<SmtpResult>(0, "ok");
}

std::string SmtpClient::GetDebugInfo() {
    return m_ss.str();
}

}
