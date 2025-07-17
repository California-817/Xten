#ifndef __XTEN_HTTP_PARSER_H__
#define __XTEN_HTTP_PARSER_H__
#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
namespace Xten
{
    namespace http
    {
        // http请求头部解析：文本协议--->HttpRequest
        class HttpRequestParser
        {
        public:
            typedef std::shared_ptr<HttpRequestParser> ptr;
            HttpRequestParser();
            // 开始解析(返回值是本次解析长度)
            size_t Execute(char *data, size_t len);
            // 判断是否解析完成
            //  1--->解析完成 0--->解析未完成 -1--->解析出错
            int IsFinished();
            // 判断是否有解析错误
            int HasError();
            // 获取请求结构体
            HttpRequest::ptr GetRequest() const { return _request; }
            // 获取parser
            const http_parser &GetParser() const { return _parser; }
            // 获取正文长度
            size_t GetBodyLength();
            // 设置错误值
            void SetErrno(int error) { _errno = error; }
            ~HttpRequestParser() = default;
            // 获取请求包头最大长度
            static uint32_t GetHttpReqMaxBufferSize();
            // 获取请求正文最大长度
            static uint32_t GetHttpReqMaxBodySize();

        private:
            http_parser _parser;       // 请求解析结构体
            HttpRequest::ptr _request; // 请求机构体
            // 1000 : invaild method
            // 1001 : invaild version
            // 1002 : invaild faild  错误请求头字段kv
            int _errno; // 解析错误码
        };
        // http响应头部解析：文本协议--->HttpResponse
        class HttpResponseParser
        {
        public:
            HttpResponseParser();
            HttpResponse::ptr GetResponse() { return _response; };
            // 开始解析(返回值是本次解析长度)
            size_t Execute(char *data, size_t len, bool chunck);
            // 判断是否解析完成
            //  1--->解析完成 0--->解析未完成 -1--->解析出错
            int IsFinished();
            // 是否有错误
            int HasError();
            // 获取响应正文长度
            size_t GetBodyLength();
            ~HttpResponseParser() = default;
            void SetErrno(int error) { _errno = error; };
            // 获取parser
            const httpclient_parser &GetParser() const { return _parser; }
            static uint32_t GetHttpRspMaxBufferSize();
            static uint32_t GetHttpRspMaxBodySize();

        private:
            // 响应结构体
            HttpResponse::ptr _response;
            // 响应解析器
            httpclient_parser _parser;
            int _errno; // 错误码
        };
    }
}
#endif