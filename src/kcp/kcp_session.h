#ifndef __XTEN_KCP_SESSION_H__
#define __XTEN_KCP_SESSION_H__
#include "third_part/ikcp.h"
#include <memory>
#include"kcp_listener.h"
namespace Xten
{
    namespace kcp
    {
        class KcpSession
        {   
            public:
                typedef std::shared_ptr<KcpSession> ptr;
            private:
            
        };
    } // namespace kcp
    
} // namespace Xten

 
#endif