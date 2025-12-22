#ifndef __XTEN_MSGHANDLE_H__
#define __XTEN_MSGHANDLE_H__
#include"nocopyable.hpp"
#include<memory>
#include<unordered_map>
#include<functional>
#include"protocol.h"
namespace Xten
{
    //消息处理器
    class MsgHandler : public NoCopyable
    {
    public:
        typedef std::shared_ptr<MsgHandler> ptr;
        // typedef std::function<int()
        // MsgHandler() {}
        // virtual ~MsgHandler() {}
        //处理消息的接口
        virtual void handleMessage(Message::ptr msg) = 0;
    private:
        //路由
        // std::unordered_map<uint32_t,>
    };
    // class KcpMsgHandler
} // namespace Xten

#endif
