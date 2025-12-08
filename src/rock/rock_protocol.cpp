#include "rock_protocol.h"
#include "../log.h"
#include "../config.h"
#include "../streams/zlib_stream.h"
#include <sstream>
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static ConfigVar<uint32_t>::ptr g_rock_protocol_max_body_length =
        Config::LookUp("rock.protocol.max_body_length", (uint32_t)(1024 * 1024 * 64), "rock protocol max body length");
    static ConfigVar<uint32_t>::ptr g_rock_protocol_min_gzip_body_length =
        Config::LookUp("rock.protocol.min_gzip_body_length", (uint32_t)(1024 * 4), "rock protocol min gzip body length");
    static uint32_t s_rock_protocol_max_body_length = 0;
    static uint32_t s_rock_protocol_min_gzip_body_length = 0;
    namespace
    {
        struct RockLengthInit
        {
            RockLengthInit()
            {
                s_rock_protocol_max_body_length = g_rock_protocol_max_body_length->GetValue();
                s_rock_protocol_min_gzip_body_length = g_rock_protocol_min_gzip_body_length->GetValue();
                g_rock_protocol_max_body_length->AddListener([](const uint32_t &old, const uint32_t &newval)
                                                             { s_rock_protocol_max_body_length = newval; });
                g_rock_protocol_min_gzip_body_length->AddListener([](const uint32_t &old, const uint32_t &newval)
                                                                  { s_rock_protocol_min_gzip_body_length = newval; });
            }
        };
        RockLengthInit __rocklenInit;
    }

    // 序列化到bytearray中
    bool
    RockBody::SerializeToByteArray(ByteArray::ptr ba) const
    {
        ba->WriteStringF32(_data);
        return true;
    }
    // 从bytearray中反序列化出string
    bool RockBody::ParseFromByteArray(ByteArray::ptr ba)
    {
        _data = ba->ReadStringF32();
        return true;
    }
    // 根据请求的字段创建对应的响应
    std::shared_ptr<RockResponse> RockRequest::CreateResponse()
    {
        std::shared_ptr<RockResponse> rsp = std::make_shared<RockResponse>();
        rsp->SetCmd(_cmd);
        rsp->SetSn(_sn);
        return rsp;
    }
    uint8_t RockRequest::GetMessageType() const
    {
        return MessageType::REQUEST;
    }
    // 将消息结构体序列化成bytearray
    bool RockRequest::SerializeToByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Request::SerializeToByteArray(ba);
            ret &= RockBody::SerializeToByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockRequest::SerializeToByteArray failed";
        }
        return ret;
    }
    // 从bytearray中反序列化出消息结构体
    bool RockRequest::ParseFromByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Request::ParseFromByteArray(ba);
            ret &= RockBody::ParseFromByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockRequest::ParseFromByteArray failed";
        }
        return ret;
    }
    // 转字符串
    std::string RockRequest::ToString() const
    {
        std::stringstream ss;
        ss << "[RockRequest sn=" << _sn
           << " cmd=" << _cmd
           << " body.length=" << _data.size()
           << "]";
        return ss.str();
    }
    // 获取name
    const std::string &RockRequest::GetName() const
    {
        static std::string name = "RockRequest";
        return name;
    }
    uint8_t RockResponse::GetMessageType() const
    {
        return MessageType::RESPONSE;
    }
    // 将消息结构体序列化成bytearray
    bool RockResponse::SerializeToByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Response::SerializeToByteArray(ba);
            ret &= RockBody::SerializeToByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockResponse::SerializeToByteArray failed";
        }
        return ret;
    }
    // 从bytearray中反序列化出消息结构体
    bool RockResponse::ParseFromByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Response::ParseFromByteArray(ba);
            ret &= RockBody::ParseFromByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockResponse::ParseFromByteArray failed";
        }
        return ret;
    }
    // 转字符串
    std::string RockResponse::ToString() const
    {
        std::stringstream ss;
        ss << "[RockResponse sn=" << _sn
           << " cmd=" << _cmd
           << " result=" << _result
           << " result_msg=" << _resultString
           << " body.length=" << _data.size()
           << "]";
        return ss.str();
    }
    // 获取name
    const std::string &RockResponse::GetName() const
    {
        static std::string name = "RockResponse";
        return name;
    }
    uint8_t RockNotify::GetMessageType() const
    {
        return MessageType::NOTIFY;
    }
    // 将消息结构体序列化成bytearray
    bool RockNotify::SerializeToByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Notify::SerializeToByteArray(ba);
            ret &= RockBody::SerializeToByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockNotify::SerializeToByteArray failed";
        }
        return ret;
    }
    // 从bytearray中反序列化出消息结构体
    bool RockNotify::ParseFromByteArray(ByteArray::ptr ba)
    {
        bool ret = true;
        try
        {
            ret = Notify::ParseFromByteArray(ba);
            ret &= RockBody::ParseFromByteArray(ba);
        }
        catch (...)
        {
            XTEN_LOG_INFO(g_logger) << "RockNotify::ParseFromByteArray failed";
        }
        return ret;
    }
    // 转字符串
    std::string RockNotify::ToString() const
    {
        std::stringstream ss;
        ss << "[RockNotify notify=" << _notify
           << " body.length=" << _data.size()
           << "]";
        return ss.str();
    }
    // 获取name
    const std::string &RockNotify::GetName() const
    {
        static std::string name = "RockNotify";
        return name;
    }
    static uint8_t s_magic[2] = {0x12, 0x21};
    RockMsgHead::RockMsgHead()
        : magic{s_magic[0], s_magic[1]},
          version(1),
          flag(0),
          length(0)
    {
    }
    // 将一个消息结构体直接发送到stream流中
    int32_t RockMessageDecoder::SerializeToStream(Stream::ptr stream, Message::ptr msg)
    {
        RockMsgHead head;
        ByteArray::ptr ba = msg->ToByteArray();
        if (!ba)
        {
            XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder body serializeTo bytearray error";
            return -1;
        }
        ba->SetPosition(0);
        head.length = ba->GetSize();
        if (head.length > s_rock_protocol_min_gzip_body_length)
        {
            // 需要压缩
            head.flag |= 0x1;
            ZlibStream::ptr zstream = ZlibStream::Creategzip(true);
            if (zstream->Write(ba, head.length) != Z_OK)
            {
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder serializeTo gizp error";
                return -2;
            }
            if (zstream->Flush() != Z_OK)
            {
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder serializeTo gizp flush error";
                return -3;
            }
            ba = zstream->GetByteArray();
            head.length = ba->GetSize();
        }
        head.length = htobe32(head.length);
        // 先写入rock协议头
        if (stream->WriteFixSize(&head, sizeof(head)) <= 0)
        {
            // 写入出错
            XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder write head to stream error";
            return -4;
        }
        // 写正文数据
        if (stream->WriteFixSize(ba, ba->GetReadSize()) <= 0)
        {
            XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder write body to stream error";
            return -5;
        }
        return sizeof(head) + ba->GetSize();
    }
    // 从stream流中读取一个消息体
    Message::ptr RockMessageDecoder::ParseFromStream(Stream::ptr stream)
    {
        RockMsgHead head;
        ByteArray::ptr ba = std::make_shared<ByteArray>();
        do
        {
            // 先读取rock头
            if (stream->ReadFixSize(&head, sizeof(head)) <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder read rockhead from stream error";
                break;
            }
            // 验证魔数字段
            if (memcmp(head.magic, s_magic, sizeof(s_magic)) != 0)
            {
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder compare rockhead.magic error";
                break;
            }
            if (head.version != 0x1)
            {
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder confire rockprotocol version error != 1";
                break;
            }
            head.length = be32toh(head.length);
            if (head.length > s_rock_protocol_max_body_length)
            {
                // body长度非法
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder parse head found body length is invaild";
                break;
            }
            // 长度合法，读取指定长度的数据
            if (stream->ReadFixSize(ba, head.length) <= 0)
            {
                // 读取失败
                XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder parse read body from stream error";
                break;
            }
            // 在读取数据到ba中position已经到了数据末尾处
            ba->SetPosition(0);
            if (head.flag & 0x1)
            {
                // 发送方进行了压缩body
                ZlibStream::ptr zstream = ZlibStream::Creategzip(false);
                if (zstream->Write(ba, ba->GetReadSize()) != Z_OK)
                {
                    XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder zlib parse from gzip error";
                    break;
                }
                if (zstream->Flush() != Z_OK)
                {
                    XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder zlib flush from gzip error";
                    break;
                }
                ba = zstream->GetByteArray();
            }
            // 此时ba中已经存放好了body数据
            // 先获取body首字段type判断message类型
            Message::MessageType type = (Message::MessageType)ba->ReadFUint8();
            Message::ptr msg;
            switch (type)
            {
            case Message::MessageType::REQUEST:
                msg = std::make_shared<RockRequest>();
                break;
            case Message::MessageType::RESPONSE:
                msg = std::make_shared<RockResponse>();
                break;
            case Message::MessageType::NOTIFY:
                msg = std::make_shared<RockNotify>();
                break;
            default:
                XTEN_LOG_ERROR(g_logger) << "invaild message type";
                break;
            }
            if(msg)
            {
                bool ret=msg->ParseFromByteArray(ba);
                if(!ret)
                {
                    XTEN_LOG_ERROR(g_logger)<<"body parse from bytearray error";
                    return nullptr;
                }
            }
            return msg;
        } while (false);
        XTEN_LOG_ERROR(g_logger) << "RockMessageDecoder parse from stream return message error";
        return nullptr;
    }
}