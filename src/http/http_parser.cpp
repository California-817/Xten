#include "http/http_parser.h"
#include "log.h"
#include "config.h"
namespace Xten
{
    namespace http
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        static ConfigVar<uint32_t>::ptr g_http_max_request_size = Config::LookUp("http.request.buffer_size",
                                                                                 (uint32_t)(4 * 1024), "http request buffer size");
        static ConfigVar<uint32_t>::ptr g_http_max_request_body_size = Config::LookUp("http.request.body_size",
                                                                                      (uint32_t)(64 * 1024 * 1024), "http request body size");
        static ConfigVar<uint32_t>::ptr g_http_max_response_size = Config::LookUp("http.response.buffer_size",
                                                                                  (uint32_t)(4 * 1024), "http response buffer size");
        static ConfigVar<uint32_t>::ptr g_http_max_response_body_size = Config::LookUp("http.response.body_size",
                                                                                       (uint32_t)(64 * 1024 * 1024), "http response body size");
        // 考虑到配置项直接getvalue需要加锁，而需要频繁获取这个size
        static uint32_t s_http_max_request_size = 0;
        static uint32_t s_http_max_request_body_size = 0;
        static uint32_t s_http_max_response_size = 0;
        static uint32_t s_http_max_response_body_size = 0;
        namespace
        {
            struct HttpSizeInit
            {
                HttpSizeInit()
                {
                    s_http_max_request_size = g_http_max_request_size->GetValue();
                    s_http_max_request_body_size = g_http_max_request_body_size->GetValue();
                    s_http_max_response_size = g_http_max_response_size->GetValue();
                    s_http_max_response_body_size = g_http_max_response_body_size->GetValue();
                    g_http_max_request_size->AddListener([](const uint32_t &oldval, const uint32_t &newval)
                                                         { s_http_max_request_size = newval; });
                    g_http_max_request_body_size->AddListener([](const uint32_t &oldval, const uint32_t &newval)
                                                              { s_http_max_request_body_size = newval; });
                    g_http_max_response_size->AddListener([](const uint32_t &oldval, const uint32_t &newval)
                                                          { s_http_max_response_size = newval; });
                    g_http_max_response_body_size->AddListener([](const uint32_t &oldval, const uint32_t &newval)
                                                               { s_http_max_response_body_size = newval; });
                }
            };
            HttpSizeInit __httpSizeInit;
        }
        // 获取请求包头最大长度
        uint32_t HttpRequestParser::GetHttpReqMaxBufferSize()
        {
            return s_http_max_request_size;
        }
        // 获取请求正文最大长度
        uint32_t HttpRequestParser::GetHttpReqMaxBodySize()
        {
            return s_http_max_request_body_size;
        }

        // 请求方法解析回调
        void on_request_method(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            HttpMethod mtd = CharsToHttpMethod(at);
            if (mtd == HttpMethod::INVALID_METHOD)
            {
                // 非法的请求方法
                XTEN_LOG_WARN(g_logger) << "invalid http request method: "
                                        << std::string(at, length);
                parser->SetErrno(1000);
            }
            parser->GetRequest()->setMethod(mtd);
        }
        // 请求uri解析回调
        void on_request_uri(void *data, const char *at, size_t length)
        {
        }
        // 请求路径解析回调
        void on_request_path(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->GetRequest()->setPath(std::string(at, length));
        }
        // 请求参数解析回调
        void on_query_string(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->GetRequest()->setQuery(std::string(at, length));
        }
        // 请求fragment解析回调
        void on_fragment(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->GetRequest()->setFragment(std::string(at, length));
        }
        // 请求版本解析回调
        void on_http_version(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            uint8_t version = 0;
            if (strncmp(at, "HTTP/1.1", length) == 0)
            {
                version = 0x11;
            }
            else if (strncmp(at, "HTTP/1.0", length) == 0)
            {
                version = 0x10;
            }
            else
            {
                // 非法版本
                XTEN_LOG_WARN(g_logger) << "invalid http request version: "
                                        << std::string(at, length);
                parser->SetErrno(1001);
            }
            parser->GetRequest()->setVersion(version);
        }
        // 请求完成解析回调
        void on_header_done(void *data, const char *at, size_t length)
        {
        }
        // 请求字段kv解析回调
        void on_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
        {
            // std::cout << "field:" << std::string(field, flen) << "  value:" << std::string(value, vlen) << std::endl;
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            if (flen == 0)
            {
                XTEN_LOG_WARN(g_logger) << "invalid http request field length == 0";
                // parser->setError(1002);
                return;
            }
            parser->GetRequest()->setHeader(std::string(field, flen), std::string(value, vlen));
        }
        HttpRequestParser::HttpRequestParser()
            : _errno(0)
        {
            _request = std::make_shared<HttpRequest>();
            http_parser_init(&_parser);
            _parser.request_uri = on_request_uri;
            _parser.request_path = on_request_path;
            _parser.request_method = on_request_method;
            _parser.query_string = on_query_string;
            _parser.fragment = on_fragment;
            _parser.header_done = on_header_done;
            _parser.http_field = on_http_field;
            _parser.http_version = on_http_version;
            _parser.data = (void *)this; // 传入一个data指针，这个指针会在回调中作为参数传入
        }
        // 开始解析
        size_t HttpRequestParser::Execute(char *data, size_t len,size_t std_len)
        {
            size_t offset = http_parser_execute(&_parser, data, std_len, 0);
            if (offset >= 0)
            {
                memmove(data, data + offset, len - offset);
            }
            return offset;
        }
        // 判断是否解析完成
        int HttpRequestParser::IsFinished()
        {
            return http_parser_finish(&_parser);
        }
        // 判断是否有解析错误
        int HttpRequestParser::HasError()
        {
            return _errno || http_parser_has_error(&_parser);
        }
        // 获取正文长度
        size_t HttpRequestParser::GetBodyLength()
        {
            return _request->getHeaderAs<size_t>("content-length", 0);
        }
        void on_response_reason(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            parser->GetResponse()->setReason(std::string(at, length));
        }
        void on_response_status(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            HttpStatus status = (HttpStatus)(atoi(at));
            parser->GetResponse()->setStatus(status);
        }
        void on_response_chunk(void *data, const char *at, size_t length)
        {
        }
        void on_response_version(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            uint8_t version = 0;
            if (strncmp(at, "HTTP/1.1", length) == 0)
            {
                version = 0x11;
            }
            else if (strncmp(at, "HTTP/1.0", length) == 0)
            {
                version = 0x10;
            }
            else
            {
                // 非法版本
                XTEN_LOG_WARN(g_logger) << "invalid http request version: "
                                        << std::string(at, length);
                parser->SetErrno(1001);
            }
            parser->GetResponse()->setVersion(version);
        }
        void on_response_header_done(void *data, const char *at, size_t length)
        {
        }

        void on_response_last_chunk(void *data, const char *at, size_t length)
        {
        }
        void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            if (flen == 0)
            {
                XTEN_LOG_WARN(g_logger) << "invalid http response field length == 0";
                // parser->setError(1002);
                return;
            }
            parser->GetResponse()->setHeader(std::string(field, flen), std::string(value, vlen));
        }
        HttpResponseParser::HttpResponseParser()
            : _errno(0)
        {
            _response = std::make_shared<HttpResponse>();
            httpclient_parser_init(&_parser);
            _parser.data = (void *)this;
            _parser.reason_phrase = on_response_reason;
            _parser.status_code = on_response_status;
            _parser.chunk_size = on_response_chunk;
            _parser.http_version = on_response_version;
            _parser.header_done = on_response_header_done;
            _parser.last_chunk = on_response_last_chunk;
            _parser.http_field = on_response_http_field;
        }
        uint32_t HttpResponseParser::GetHttpRspMaxBufferSize()
        {
            return s_http_max_response_size;
        }
        uint32_t HttpResponseParser::GetHttpRspMaxBodySize()
        {
            return s_http_max_response_body_size;
        }
        // 开始解析
        size_t HttpResponseParser::Execute(char *data, size_t len, bool chunck)
        {
            if (chunck)
            {
                httpclient_parser_init(&_parser);
            }
            size_t offset = httpclient_parser_execute(&_parser, data, len, 0);
            if (offset >= 0)
            {
                memmove(data, data + offset, len - offset);
            }
            return offset;
        }
        // 是否完成
        int HttpResponseParser::IsFinished()
        {
            return httpclient_parser_finish(&_parser);
        }
        // 是否有错误
        int HttpResponseParser::HasError()
        {
            return _errno || httpclient_parser_has_error(&_parser);
        }
        // 获取响应正文长度
        size_t HttpResponseParser::GetBodyLength()
        {
            return _response->getHeaderAs<size_t>("content-length", 0);
        }
    }

};