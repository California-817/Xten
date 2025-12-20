#ifndef __XTEN_MSGHANDLE_H__
#define __XTEN_MSGHANDLE_H__
#include"nocopyable.hpp"
#include<memory>
#include"protocol.h"
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
        virtual void handleMessage(Message::ptr msg) = 0;
    };
} // namespace Xten

#endif
