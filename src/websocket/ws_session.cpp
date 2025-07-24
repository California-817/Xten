#include "ws_session.h"
#include "log.h"
#include <endian.h>
#include "config.h"
namespace Xten
{
    namespace http
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        static ConfigVar<uint32_t>::ptr g_websocket_message_max_size =
            Xten::Config::LookUp("websocket.message.max_size",
                                 (uint32_t)(32 * 1024 * 1024),
                                 "websocket message max size");
        static uint32_t s_websocket_message_max_size = 0;
        namespace
        {
            struct WSMessageSizeInit
            {
                WSMessageSizeInit()
                {
                    s_websocket_message_max_size = g_websocket_message_max_size->GetValue();
                    g_websocket_message_max_size->AddListener([](const uint32_t &oldval, const uint32_t &newval)
                                                              { s_websocket_message_max_size = newval; });
                }
            };
            WSMessageSizeInit __wsSizeInit;
        }
        WSSession::WSSession(Socket::ptr socket, bool is_owner)
            : HttpSession(socket, is_owner)
        {
        }
        // 进行http协议升级websocket握手
        HttpRequest::ptr WSSession::HandleShake()
        {
            HttpRequest::ptr req;
            // 接收来自客户端的升级请求
            do
            {
                req = RecvRequest();
                if (!req)
                {
                    // 接收请求失败
                    XTEN_LOG_INFO(g_logger) << "invaild http request";
                    break;
                }
                // 检查是否是一个合法的协议升级http请求
                if (strcasecmp(req->getHeader("Upgrade").c_str(), "websocket"))
                {
                    XTEN_LOG_INFO(g_logger) << "http header Upgrade != websocket";
                    break;
                }
                if (strcasecmp(req->getHeader("Connection").c_str(), "Upgrade"))
                {
                    XTEN_LOG_INFO(g_logger) << "http header Connection != Upgrade";
                    break;
                }
                if (req->getHeaderAs<int>("Sec-webSocket-Version") != 13)
                {
                    XTEN_LOG_INFO(g_logger) << "http header Sec-webSocket-Version != 13";
                    break;
                }
                std::string key = req->getHeader("Sec-WebSocket-Key");
                if (key.empty())
                {
                    XTEN_LOG_INFO(g_logger) << "http header Sec-WebSocket-Key = null";
                    break;
                }
                // 服务端密钥
                std::string v = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                // 将这个v生成hash摘要
                std::string digest = Xten::sha1sum(v);
                // 进行base64编码可视化
                std::string base64_digest_key = Xten::base64encode(digest);
                req->setWebsocket(true);
                // 创建响应
                HttpResponse::ptr rsp = req->createResponse();
                rsp->setStatus(HttpStatus::SWITCHING_PROTOCOLS);
                rsp->setWebsocket(true);
                rsp->setReason("Switching Protocols");
                rsp->setHeader("Upgrade", "websocket");
                rsp->setHeader("Connection", "Upgrade");
                rsp->setHeader("Sec-WebSocket-Accept", base64_digest_key);
                // send
                if (SendResponse(rsp) <= 0)
                {
                    XTEN_LOG_INFO(g_logger) << "Send webscket handleshake response failed";
                    break;
                }
                XTEN_LOG_DEBUG(g_logger) << *req;
                XTEN_LOG_DEBUG(g_logger) << *rsp;
                return req;
            } while (false);
            // 握手失败
            if (req)
            {
                XTEN_LOG_DEBUG(g_logger) << *req;
            }
            return nullptr;
        }
        // 发送websocket消息体结构
        int32_t WSSession::SendMessage(WSFrameMessage::ptr msg, bool fin)
        {
            return WSSendMessage(this, msg, false, fin);
        }
        // 直接发送数据
        int32_t WSSession::SendMessage(const std::string &data,
                                       int32_t opcode, bool fin)
        {
            return WSSendMessage(this, std::make_shared<WSFrameMessage>(opcode,data), false, fin);
        }
        // 接收消息,返回消息体
        WSFrameMessage::ptr WSSession::RecvMessage()
        {
            return WSRecvMessage(this, false);
        }
        // 发送心跳ping帧
        int32_t WSSession::Ping()
        {
            return WSPing(this);
        }
        // 发送心跳pong帧
        int32_t WSSession::Pong()
        {
            return WSPong(this);
        }
        extern int32_t WSSendMessage(Stream *stream, WSFrameMessage::ptr msg, bool client, bool fin)
        {
            do
            {
                WSFrameHead head;
                std::cout << "websocket head size: " << sizeof(head) << std::endl;
                memset(&head, 0, sizeof(head));
                int opcode = msg->GetOpCode();
                head.opcode = opcode;
                head.fin = fin;
                head.mask = client; // 只有客户端才会发送mask掩码
                int payloadlen = msg->GetData().size();
                if (payloadlen < 126)
                {
                    head.payload = payloadlen;
                }
                else if (payloadlen < 65535)
                {
                    head.payload = 126;
                }
                else
                {
                    head.payload = 127;
                }
                // 头部填充完毕---进行发送
                if (stream->WriteFixSize(&head, sizeof(head)) <= 0)
                {
                    break;
                }
                // 发送长度扩展字段
                if (head.payload == 126)
                {
                    uint16_t expend = payloadlen;
                    expend = htons(expend);
                    if (stream->WriteFixSize(&expend, sizeof(expend)) <= 0)
                    {
                        break;
                    }
                }
                else if (head.payload == 127)
                {
                    uint64_t expend = htobe64(payloadlen);
                    if (stream->WriteFixSize(&expend, sizeof(expend)) <= 0)
                    {
                        break;
                    }
                }
                // 发送掩码值(客户端)
                if (client)
                {
                    // 客户端
                    char mask[4];
                    uint32_t rand_val = rand();
                    memcpy(mask, &rand_val, sizeof(mask));
                    std::string &data = msg->GetData();
                    for (int i = 0; i < data.size(); i++)
                    {
                        data[i] = data[i] ^ mask[i % 4];
                    }
                    if (stream->WriteFixSize(mask, sizeof(mask)) <= 0)
                    {
                        break;
                    }
                }
                // 发送数据
                if (stream->WriteFixSize(msg->GetData().c_str(), payloadlen) <= 0)
                {
                    break;
                }
                return payloadlen + sizeof(head);
            } while (false);
            // 发送失败
            stream->Close();
            return -1;
        }
        WSFrameMessage::ptr WSRecvMessage(Stream *stream, bool client)
        {
            std::string data; // 数据
            int opcode = 0;   // 操作码
            int cur_len = 0;  // 当前数据位置
            do
            {
                WSFrameHead head;
                // 读取一个完整头部
                if (stream->ReadFixSize(&head, sizeof(head)) <= 0)
                {
                    break;
                }
                XTEN_LOG_DEBUG(g_logger) << "ws head:" << head.toString();
                if (head.opcode == WSFrameHead::OPCODE::PING)
                {
                    // 发送心跳应答包
                    XTEN_LOG_INFO(g_logger) << "PING";
                    if (WSPong(stream) <= 0)
                    {
                        break;
                    }
                    // 发送完心跳响应包PONG后继续尝试接受数据包
                }
                else if (head.opcode == WSFrameHead::OPCODE::PONG)
                {
                    // nothing todo
                }
                else if (head.opcode == WSFrameHead::OPCODE::TEXT_FRAME ||
                         WSFrameHead::OPCODE::BIN_FRAME ||
                         WSFrameHead::OPCODE::CONTINUE)
                {
                    if (!client && !head.mask)
                    {
                        // 服务端接受客户端消息，客户端发送时必须用mask加密
                        XTEN_LOG_INFO(g_logger) << "ws head mask != 1";
                        break;
                    }
                    // 获取真正消息长度
                    uint64_t len = 0;
                    if (head.payload == 126)
                    {
                        uint16_t real_len = 0;
                        if (stream->ReadFixSize(&real_len, sizeof(real_len)) <= 0)
                        {
                            break;
                        }
                        len = ntohs(real_len);
                    }
                    else if (head.payload == 127)
                    {
                        uint64_t real_len = 0;
                        if (stream->ReadFixSize(&real_len, sizeof(real_len) <= 0))
                        {
                            break;
                        }
                        len = be64toh(real_len);
                    }
                    else
                    {
                        len = head.payload;
                    }
                    // 看长度是否超过了最大长度
                    if (cur_len + (uint32_t)len >= s_websocket_message_max_size)
                    {
                        XTEN_LOG_WARN(g_logger) << "WSFrameMessage length > "
                                                << s_websocket_message_max_size
                                                << " (" << (cur_len + len) << ")";
                        break;
                    }
                    char mask[4];
                    if (head.mask)
                    {
                        // 服务端接受客户端消息
                        if (stream->ReadFixSize(mask, sizeof(mask)) <= 0)
                        {
                            break;
                        }
                    }
                    data.resize(cur_len + len);
                    // 接受消息体
                    if (stream->ReadFixSize(&data[cur_len], len) <= 0)
                    {
                        break;
                    }
                    // 接受完后进行mask解码
                    if (head.mask)
                    {
                        for (int i = 0; i < len; i++)
                        {
                            data[cur_len + i] = data[cur_len + i] ^ mask[i % 4];
                        }
                    }
                    cur_len += len;
                    if (!opcode && head.opcode != WSFrameHead::OPCODE::CONTINUE)
                    {
                        opcode = head.opcode;
                    }
                    // 是继续帧
                    if (head.fin)
                    {
                        // 最后一个继续帧
                        XTEN_LOG_DEBUG(g_logger) << data;
                        return std::make_shared<WSFrameMessage>(opcode, data);
                    }
                    // 不是最后一个继续帧---继续读取
                }
                else
                {
                    // 链接关闭帧
                    XTEN_LOG_DEBUG(g_logger) << "opcode==CLOSE";
                    break;
                }

            } while (true);
            stream->Close();
            return nullptr;
        }
        // 发送心跳帧
        int32_t WSPing(Stream *stream)
        {
            WSFrameHead head;
            memset(&head, 0, sizeof(head));
            head.opcode = WSFrameHead::OPCODE::PING;
            head.fin = true;
            if (stream->WriteFixSize(&head, sizeof(head)) <= 0)
            {
                stream->Close();
                return -1;
            }
            return sizeof(head);
        }
        int32_t WSPong(Stream *stream)
        {
            WSFrameHead head;
            memset(&head, 0, sizeof(head));
            head.opcode = WSFrameHead::OPCODE::PONG;
            head.fin = true;
            if (stream->WriteFixSize(&head, sizeof(head)) <= 0)
            {
                stream->Close();
                return -1;
            }
            return sizeof(head);
        }
    }
}