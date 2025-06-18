#include "../include/hook.h"
namespace Xten
{
    //表面 API 是同步风格（如 read()、write() 直接返回）但底层通过 hook 技术和协程调度
    //遇到阻塞时会让出当前协程，其他协程可以继续运行，线程不会被真正阻塞

    bool is_hook_enable()
    {}
    void set_hook_enable(bool ishook)
    {}
}
extern "C"
{
    //定义一个与系统库函数同名的函数.在链接阶段.链接器会优先使用自定义的函数.而不是 libc 中的版本

}


