#ifndef __XTEN_MSG_HANDLER_H__
#define __XTEN_MSG_HANDLER_H__
#include<memory>
#include"kcp_protocol.h"
namespace Xten
{
    namespace kcp
    {
        class MsgHandler
        {
            public:
            typedef std::shared_ptr<MsgHandler> ptr;
        };
    };
} // namespace Xten
#endif
