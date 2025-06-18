#ifndef __XTEN_HOOK_H__
#define __XTEN_HOOK_H__
#include<unistd.h>
#include<fcntl.h>
// +------------------+       +------------------+
// | 应用调用 socket() |  -->  | 自定义 socket()  |
// +------------------+       +------------------+
//                                       ↓
//                           +----------------------+
//                           |    是否启用Hook?      |
//                           +----------+-----------+
//                                      |
//                        否            |           是
//                   +------------------+------------------+
//                   ↓                                     ↓
//     直接调用 socket_f(socket())                调用 socket_f() 并插入额外逻辑
namespace Xten
{
    //查看当前线程的接口是否是hook住的
    bool is_hook_enable();
    //设置当前线程接口是否hook
    void set_hook_enable(bool ishook);
}
//防止c++函数名修饰
extern "C"
{
    //sleep
    typedef  unsigned int (*sleep_func)(unsigned int seconds);
    extern sleep_func sleep_f;
    
    typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
    extern read_func read_f;


    typedef int (*fcntl_func)(int fd, int cmd, ... /* arg */ );
    extern fcntl_func fcntl_f;
}
#endif