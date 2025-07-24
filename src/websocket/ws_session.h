#ifndef __XTEN_WS_SESSION_H__
#define __XTEN_WS_SESSION_H__
#include "../http/http.h"
#include "../http/http_session.h"
//-------------------webssocket协议格式-----------------------------
//  0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |     Extended payload length continued, if payload len == 127  |
// +-------------------------------+-------------------------------+
// | Masking-key, if MASK set to 1 |          Payload Data         |
// +-------------------------------+-------------------------------+
// |         Payload Data continued ...                     |
// +--------------------------------------------------------+
namespace Xten
{
    namespace http
    {
        // 定义websocket协议头结构(使用的是位域，保证1字节结构体对齐，防止未定义行为)
#pragma pack(1)
        struct WSFrameHead
        {
            enum OPCODE
            {
                /// 数据分片帧
                CONTINUE = 0,
                /// 文本帧
                TEXT_FRAME = 1,
                /// 二进制帧
                BIN_FRAME = 2,
                /// 断开连接
                CLOSE = 8,
                /// PING
                PING = 0x9,
                /// PONG
                PONG = 0xA
            };
            //位域的排列方式是从一个字节的低位开始向高位分配
            uint32_t opcode : 4;
            bool rsv3 : 1;
            bool rsv2 : 1;
            bool rsv1 : 1;
            bool fin : 1;
            uint32_t payload : 7;
            bool mask : 1;
            std::string toString() const
            {
                std::stringstream ss;
                ss << "[WSFrameHead fin=" << fin
                   << " rsv1=" << rsv1
                   << " rsv2=" << rsv2
                   << " rsv3=" << rsv3
                   << " opcode=" << opcode
                   << " mask=" << mask
                   << " payload=" << payload
                   << "]";
                return ss.str();
            }
        };
#pragma pack()
        // websocket消息体
        class WSFrameMessage
        {
        public:
            typedef std::shared_ptr<WSFrameMessage> ptr;
            WSFrameMessage(const int &opcode = 0, const std::string &data = "")
                : _opcode(opcode), _data(data)
            {
            }
            ~WSFrameMessage() = default;
            void SetOpCode(const int &code) { _opcode = code; }
            int GetOpCode() { return _opcode; }
            void SetData(const std::string &data) { _data = data; }
            const std::string &GetData() const { return _data; }
            std::string &GetData() { return _data; }

        private:
            int _opcode;
            std::string _data;
        };
        // 定义websocketsession结构
        class WSSession : public HttpSession
        {
        public:
            typedef std::shared_ptr<WSSession> ptr;
            WSSession(Socket::ptr socket, bool is_owner = true);
            ~WSSession() = default;
            // 进行http协议升级websocket握手
            HttpRequest::ptr HandleShake();
            // 发送websocket消息体结构（对于全双工的websocket协议，不是线程安全的）
            int32_t SendMessage(WSFrameMessage::ptr msg, bool fin = true);
            // 直接发送数据
            int32_t SendMessage(const std::string &data,
                                int32_t opcode = WSFrameHead::OPCODE::TEXT_FRAME, bool fin = true);
            // 接收消息,返回消息体
            WSFrameMessage::ptr RecvMessage();
            // 发送心跳ping帧
            int32_t Ping();
            // 发送心跳pong帧
            int32_t Pong();
        };
        // 作为客户端，服务端均能使用的方法
        extern int32_t WSSendMessage(Stream *stream, WSFrameMessage::ptr msg, bool client, bool fin);
        extern WSFrameMessage::ptr WSRecvMessage(Stream *stream, bool client);
        // 发送心跳帧
        extern int32_t WSPing(Stream *stream);
        extern int32_t WSPong(Stream *stream);
    }
}
#endif