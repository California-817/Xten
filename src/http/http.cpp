#include "http.h"
#include"util.h"
#include<unicode/unistr.h>
namespace Xten
{
    namespace http
    {

        HttpMethod StringToHttpMethod(const std::string &m)
        {
#define XX(num, name, string)            \
    if (strcmp(#string, m.c_str()) == 0) \
    {                                    \
        return HttpMethod::name;         \
    }
            HTTP_METHOD_MAP(XX);
#undef XX
            return HttpMethod::INVALID_METHOD;
        }

        HttpMethod CharsToHttpMethod(const char *m)
        {
#define XX(num, name, string)                      \
    if (strncmp(#string, m, strlen(#string)) == 0) \
    {                                              \
        return HttpMethod::name;                   \
    }
            HTTP_METHOD_MAP(XX);
#undef XX
            return HttpMethod::INVALID_METHOD;
        }

        static const char *s_method_string[] = {
#define XX(num, name, string) #string,
            HTTP_METHOD_MAP(XX)
#undef XX
        };

        const char *HttpMethodToString(const HttpMethod &m)
        {
            uint32_t idx = (uint32_t)m;
            if (idx >= (sizeof(s_method_string) / sizeof(s_method_string[0])))
            {
                return "<unknown>";
            }
            return s_method_string[idx];
        }

        const char *HttpStatusToString(const HttpStatus &s)
        {
            switch (s)
            {
#define XX(code, name, msg) \
    case HttpStatus::name:  \
        return #msg;
                HTTP_STATUS_MAP(XX);
#undef XX
            default:
                return "<unknown>";
            }
        }

        bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const
        {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }

        HttpRequest::HttpRequest(uint8_t version, bool close)
            : m_method(HttpMethod::GET), m_version(version), m_close(close), m_websocket(false), m_parserParamFlag(0), m_streamId(0), m_path("/")
        {
        }

        std::string HttpRequest::getHeader(const std::string &key, const std::string &def) const
        {
            auto it = m_headers.find(key);
            return it == m_headers.end() ? def : it->second;
        }

        void HttpRequest::setUri(const std::string &uri)
        {
            auto pos = uri.find('?');
            if (pos == std::string::npos)
            {
                auto pos2 = uri.find('#');
                if (pos2 == std::string::npos)
                {
                    m_path = uri; //只有请求路径
                }
                else
                {
                    m_path = uri.substr(0, pos2); //请求路径
                    m_fragment = uri.substr(pos2 + 1); //片段标识符
                }
            }
            else
            {
                m_path = uri.substr(0, pos);

                auto pos2 = uri.find('#', pos + 1);
                if (pos2 == std::string::npos)
                {
                    m_query = uri.substr(pos + 1);
                }
                else
                {
                    m_query = uri.substr(pos + 1, pos2 - pos - 1);
                    m_fragment = uri.substr(pos2 + 1);
                }
            }
        }

        std::string HttpRequest::getUri()
        {
            //将三个部分进行拼接  path?query#fragment
            return m_path + (m_query.empty() ? "" : "?" + m_query) + (m_fragment.empty() ? "" : "#" + m_fragment);
        }

        std::shared_ptr<HttpResponse> HttpRequest::createResponse()
        {
            return std::make_shared<HttpResponse>(getVersion(), isClose());
        }

        std::string HttpRequest::getParam(const std::string &key, const std::string &def)
        {
            initQueryParam();
            initBodyParam();
            auto it = m_params.find(key);
            return it == m_params.end() ? def : it->second;
        }

        std::string HttpRequest::getCookie(const std::string &key, const std::string &def)
        {
            initCookies();
            auto it = m_cookies.find(key);
            return it == m_cookies.end() ? def : it->second;
        }

        void HttpRequest::setHeader(const std::string &key, const std::string &val)
        {
            m_headers[key] = val;
        }

        void HttpRequest::setParam(const std::string &key, const std::string &val)
        {
            m_params[key] = val;
        }

        void HttpRequest::setCookie(const std::string &key, const std::string &val)
        {
            m_cookies[key] = val;
        }

        void HttpRequest::delHeader(const std::string &key)
        {
            m_headers.erase(key);
        }

        void HttpRequest::delParam(const std::string &key)
        {
            m_params.erase(key);
        }

        void HttpRequest::delCookie(const std::string &key)
        {
            m_cookies.erase(key);
        }

        bool HttpRequest::hasHeader(const std::string &key, std::string *val)
        {
            auto it = m_headers.find(key);
            if (it == m_headers.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }

        bool HttpRequest::hasParam(const std::string &key, std::string *val)
        {
            initQueryParam();
            initBodyParam();
            auto it = m_params.find(key);
            if (it == m_params.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }

        bool HttpRequest::hasCookie(const std::string &key, std::string *val)
        {
            initCookies();
            auto it = m_cookies.find(key);
            if (it == m_cookies.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }

        std::string HttpRequest::toString() const
        {
            std::stringstream ss;
            dump(ss);
            return ss.str();
        }

        std::ostream &HttpRequest::dump(std::ostream &os) const
        {
            // GET /uri HTTP/1.1
            // Host: wwww.sylar.top
            //
            //
            //请求行
            os << HttpMethodToString(m_method) << " "
               << m_path
               << (m_query.empty() ? "" : "?")
               << m_query
               << (m_fragment.empty() ? "" : "#")
               << m_fragment
               << " HTTP/"
               << ((uint32_t)(m_version >> 4))
               << "."
               << ((uint32_t)(m_version & 0x0F))
               << "\r\n";
            //请求报头
            if (!m_websocket)
            {
                //不是websocket协议才填充 【长，短连接】
                os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
            }
            //当需要进行websocket协议升级时对应http升级报文
            // Upgrade: websocket
            // Connection: Upgrade
            // Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==
            // Sec-WebSocket-Version: 13
            for (auto &i : m_headers)
            {
                if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0)
                {
                    continue; //不是websocket协议则connection字段已经填充
                }
                if (!strcasecmp(i.first.c_str(), "content-length"))
                {
                    continue;
                }
                os << i.first << ": " << i.second << "\r\n";
            }

            if (!m_body.empty())
            {
                os << "content-length: " << m_body.size() << "\r\n\r\n" //空行
                   << m_body; //请求正文
            }
            else
            {
                os << "\r\n";
            }
            return os;
        }

        void HttpRequest::init()
        {
            std::string conn = getHeader("connection");
            if (!conn.empty())
            {
                if (strcasecmp(conn.c_str(), "keep-alive") == 0)
                {
                    //长连接
                    m_close = false;
                }
                else
                {
                    //短连接
                    m_close = true;
                }
            }
        }

        void HttpRequest::initParam()
        {
            initQueryParam();
            initBodyParam();
            initCookies();
        }

        void HttpRequest::initQueryParam()
        {
            if (m_parserParamFlag & 0x1)
            {
                return;
            }

#define PARSE_PARAM(str, m, flag, trim)                                                             \
    size_t pos = 0;                                                                                 \
    do                                                                                              \
    {                                                                                               \
        size_t last = pos;                                                                          \
        pos = str.find('=', pos);                                                                   \
        if (pos == std::string::npos)                                                               \
        {                                                                                           \
            break;                                                                                  \
        }                                                                                           \
        size_t key = pos;                                                                           \
        pos = str.find(flag, pos);                                                                  \
        m.insert(std::make_pair(trim(str.substr(last, key - last)),                                 \
                                Xten::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if (pos == std::string::npos)                                                               \
        {                                                                                           \
            break;                                                                                  \
        }                                                                                           \
        ++pos;                                                                                      \
    } while (true);

            PARSE_PARAM(m_query, m_params, '&',);
            m_parserParamFlag |= 0x1;
        }

        void HttpRequest::initBodyParam()
        {
            if (m_parserParamFlag & 0x2)
            {
                return;
            }
            std::string content_type = getHeader("content-type");
            if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr)
            {
                //body字段类型不是表单类型：无法使用key1=value1&key2=value2方式解析body
                m_parserParamFlag |= 0x2;
                return;
            }
            //body表单字段也解析到请求参数map中（表单类型请求正文也会进行url编码）
            PARSE_PARAM(m_body, m_params, '&',);
            m_parserParamFlag |= 0x2;
        }

        void HttpRequest::initCookies()
        {
            if (m_parserParamFlag & 0x4)
            {
                return;
            }
            std::string cookie = getHeader("cookie");
            if (cookie.empty())
            {
                m_parserParamFlag |= 0x4;
                return;
            }
            //cookies的间隔符是;  （也会进行url编码）
            PARSE_PARAM(cookie, m_cookies, ';', Xten::StringUtil::Trim);
            m_parserParamFlag |= 0x4;
        }

        void HttpRequest::paramToQuery()
        {
             m_query = Xten::MapJoin(m_params.begin(), m_params.end());
        }

        HttpResponse::HttpResponse(uint8_t version, bool close)
            : m_status(HttpStatus::OK), m_version(version), m_close(close), m_websocket(false)
        {
        }

        void HttpResponse::initConnection()
        {
            std::string conn = getHeader("connection");
            if (!conn.empty())
            {
                if (strcasecmp(conn.c_str(), "keep-alive") == 0)
                {
                    m_close = false;
                }
                else
                {
                    m_close = m_version == 0x10;
                }
            }
        }

        std::string HttpResponse::getHeader(const std::string &key, const std::string &def) const
        {
            auto it = m_headers.find(key);
            return it == m_headers.end() ? def : it->second;
        }

        void HttpResponse::setHeader(const std::string &key, const std::string &val)
        {
            m_headers[key] = val;
        }

        void HttpResponse::delHeader(const std::string &key)
        {
            m_headers.erase(key);
        }

        void HttpResponse::setRedirect(const std::string &uri)
        {
            m_status = HttpStatus::FOUND;
            setHeader("Location", uri);
        }

        void HttpResponse::setCookie(const std::string &key, const std::string &val,
                                     time_t expired, const std::string &path,
                                     const std::string &domain, bool secure)
        {
            std::stringstream ss;
            ss << key << "=" << val;
            if (expired > 0)
            {
                ss << ";expires=" << Xten::Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
            }
            if (!domain.empty())
            {
                ss << ";domain=" << domain;
            }
            if (!path.empty())
            {
                ss << ";path=" << path;
            }
            if (secure)
            {
                ss << ";secure";
            }
            m_cookies.push_back(ss.str());
        }

        std::string HttpResponse::toString() const
        {
            std::stringstream ss;
            dump(ss);
            return ss.str();
        }

        std::ostream &HttpResponse::dump(std::ostream &os) const
        {
            //响应行
            os << "HTTP/"
               << ((uint32_t)(m_version >> 4))
               << "."
               << ((uint32_t)(m_version & 0x0F))
               << " "
               << (uint32_t)m_status  //状态码
               << " "
               << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) //状态码描述
               << "\r\n";
            //响应报头
            bool has_content_length = false;
            for (auto &i : m_headers)
            {
                if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0)
                {
                    continue;
                }
                if (!has_content_length && strcasecmp(i.first.c_str(), "content-length") == 0)
                {
                    has_content_length = true;
                }
                os << i.first << ": " << i.second << "\r\n";
            }
            //设置所有cookie，一个cookie对应响应中一行
            //Set-Cookie: key=value; expires=时间; path=路径; domain=域名; secure;
            for (auto &i : m_cookies)
            {
                os << "Set-Cookie: " << i << "\r\n";
            }
            if (!m_websocket)
            {
                os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
            }
            if (!m_body.empty())
            {
                if (!has_content_length)  //没有手动设置
                {
                    os << "content-length: " << m_body.size() << "\r\n\r\n"  //响应包头中正文长度字段
                       << m_body;
                }
                else
                {
                    os << "\r\n"
                       << m_body; //响应正文  
                }
            }
            else
            {
                os << "\r\n";
            }
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
        {
            return req.dump(os);
        }

        std::ostream &operator<<(std::ostream &os, const HttpResponse &rsp)
        {
            return rsp.dump(os);
        }

    }
}
