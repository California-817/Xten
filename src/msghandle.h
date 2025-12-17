#ifndef __XTEN_MSGHANDLE_H__
#define __XTEN_MSGHANDLE_H__
#include"nocopyable.hpp"
#include<memory>
namespace Xten
{
    //消息处理器
    class MsgHandler : public NoCopyable
    {
    public:
        typedef std::shared_ptr<MsgHandler> ptr;
        MsgHandler() {}
        virtual ~MsgHandler() {}
        //处理消息的接口
        virtual void handleMessage(const char* msg, uint32_t len) = 0;
    };
} // namespace Xten

#endif
