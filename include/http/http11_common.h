#ifndef _http11_common_h
#define _http11_common_h

#include <sys/types.h>
//第一个参数data就是在解析结构体中的data成员
typedef void (*element_cb)(void *data, const char *at, size_t length);
typedef void (*field_cb)(void *data, const char *field, size_t flen, const char *value, size_t vlen);

#endif
