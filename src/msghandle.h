#ifndef __XTEN_MSGHANDLE_H__
#define __XTEN_MSGHANDLE_H__
#include"nocopyable.hpp"
#include<memory>
namespace Xten
{
    //消息处理器
    class MsgHanler : public NoCopyable
    {
    public:
        typedef std::shared_ptr<MsgHanler> ptr;
        MsgHanler() {}
        virtual ~MsgHanler() {}
        //处理消息的接口
        virtual void handleMessage(const char* msg, uint32_t len) = 0;
    };
} // namespace Xten

#endif
