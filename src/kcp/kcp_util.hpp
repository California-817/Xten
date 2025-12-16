#ifndef __XTEN_KCP_UTIL_H__
#define __XTEN_KCP_UTIL_H__
#include <string>
#include <memory.h>
namespace Xten
{
    namespace kcp
    {
// 发起kcp连接的udp数据包
#define XTEN_KCP_CONNECT_PACKET "xten_kcp_connect_packet#get_convid"
#define XTEN_KCP_CONNECT_BACKPACKET "xten_kcp_connect_backpacket#return_convid:"
        class KcpUtil
        {
        public:
            static const std::string &making_connect_packet()
            {
                static std::string conn_packet = XTEN_KCP_CONNECT_PACKET;
                return conn_packet;
            }
            static std::string making_connect_backpacket()
            {
                static std::string conn_backpacket = XTEN_KCP_CONNECT_BACKPACKET;
                return conn_backpacket;
            }
            static bool is_connect_packet(const char *data, size_t len)
            {
                return (memcmp(data, XTEN_KCP_CONNECT_PACKET, len) == 0) ? true : false;
            }
        };
    } // namespace kcp

} // namespace Xten

#endif