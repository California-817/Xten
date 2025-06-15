#ifndef __XTEN_MACRO_H__
#define __XTEN_MACRO_H__
#include "log.h"
#include "util.h"
// 封装常用宏定义
#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define XTEN_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define XTEN_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define XTEN_LIKELY(x) (x)
#define XTEN_UNLIKELY(x) (x)
#endif

// 断言宏封装
#define XTEN_ASSERT(xx)                                                     \
    if (XTEN_UNLIKELY(!(xx)))                                               \
    {                                                                       \
        std::string btstring = Xten::BackTraceUtil::backtraceTostring(100); \
        XTEN_LOG_ERROR(XTEN_LOG_NAME("system")) << "Assert!!!\n"            \
                                                << "BackTrace:\n"           \
                                                << btstring;                \
        assert(0);                                                          \
    }
#define XTEN_ASSERTINFO(xx, info)                                             \
    if (XTEN_UNLIKELY(!(xx)))                                                 \
    {                                                                         \
        std::string btstring = Xten::BackTraceUtil::backtraceTostring(100);   \
        XTEN_LOG_ERROR(XTEN_LOG_NAME("system")) << "Assert!!! Info:" << #info \
                                                << "\nBackTrace:\n"           \
                                                << btstring;                  \
        assert(0);                                                            \
    }
#endif