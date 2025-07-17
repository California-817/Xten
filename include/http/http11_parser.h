#ifndef http11_parser_h
#define http11_parser_h

#include "http11_common.h"
//解析http请求的结构体
typedef struct http_parser { 
  int cs;
  size_t body_start;
  int content_len;
  size_t nread;
  size_t mark;
  size_t field_start;
  size_t field_len;
  size_t query_start;
  int xml_sent;
  int json_sent;

  void *data; //用户传入的参数

  int uri_relaxed;
  //各阶段解析后的回调函数
  field_cb http_field; //解析一个请求头中kv字段后执行该函数
  element_cb request_method;
  element_cb request_uri;
  element_cb fragment;
  element_cb request_path;
  element_cb query_string;
  element_cb http_version;
  element_cb header_done;
  
} http_parser;
//由rl实现的有限状态机解析http请求的函数
int http_parser_init(http_parser *parser);
int http_parser_finish(http_parser *parser);
size_t http_parser_execute(http_parser *parser, const char *data, size_t len, size_t off);
int http_parser_has_error(http_parser *parser);
int http_parser_is_finished(http_parser *parser);

#define http_parser_nread(parser) (parser)->nread 

#endif
