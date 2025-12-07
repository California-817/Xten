#include "xftp_protocol.h"
#include "../log.h"
#include "../config.h"
#include "../streams/zlib_stream.h"
namespace Xten
{
    namespace xftp
    {

        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        static ConfigVar<uint32_t>::ptr g_xftp_protocol_min_gzip_body_length =
            Config::LookUp("xftp.protocol.min_gzip_body_length", (uint32_t)(1024 * 1024 * 4), "xftp protocol min gzip body length");
        static uint32_t s_xftp_protocol_min_gzip_body_length = 0;
        namespace
        {
            struct xftpLengthInit
            {
                xftpLengthInit()
                {
                    s_xftp_protocol_min_gzip_body_length = g_xftp_protocol_min_gzip_body_length->GetValue();
                    g_xftp_protocol_min_gzip_body_length->AddListener([](const uint32_t &old, const uint32_t &newval)
                                                                      { s_xftp_protocol_min_gzip_body_length = newval; });
                }
            };
            xftpLengthInit __xftplenInit;
        }

        // 将消息结构体序列化成bytearray
        bool XftpRequest::SerializeToByteArray(ByteArray::ptr ba)
        {

            bool ret = Request::SerializeToByteArray(ba);
            ba->WriteStringF32(_md5);
            ba->WriteStringF32(_name);
            ba->WriteFUint64(_transSize);
            ba->WriteFUint64(_totalSize);
            ba->WriteFUint8(_b_last);
            ba->WriteFUint32(_lastSn);
            ba->WriteStringF32(_data);
            return ret;
        }
        // 从bytearray中反序列化出消息结构体
        bool XftpRequest::ParseFromByteArray(ByteArray::ptr ba)
        {
            bool ret = Request::ParseFromByteArray(ba);
            _md5 = ba->ReadStringF32();
            _name = ba->ReadStringF32();
            _transSize = ba->ReadFUint64();
            _totalSize = ba->ReadFUint64();
            _b_last = ba->ReadFUint8();
            _lastSn = ba->ReadFUint32();
            _data = ba->ReadStringF32();
            return ret;
        }

        // 将消息结构体序列化成bytearray
        bool XftpResponse::SerializeToByteArray(ByteArray::ptr ba)
        {
            bool ret = Response::SerializeToByteArray(ba);
            ba->WriteStringF32(_md5);
            ba->WriteStringF32(_name);
            ba->WriteFUint64(_transSize);
            ba->WriteFUint64(_totalSize);
            ba->WriteFUint8(_b_last);
            ba->WriteFUint32(_lastSn);
            ba->WriteStringF32(_data);
            return ret;
        }
        // 从bytearray中反序列化出消息结构体
        bool XftpResponse::ParseFromByteArray(ByteArray::ptr ba)
        {
            bool ret = Response::ParseFromByteArray(ba);
            _md5 = ba->ReadStringF32();
            _name = ba->ReadStringF32();
            _transSize = ba->ReadFUint64();
            _totalSize = ba->ReadFUint64();
            _b_last = ba->ReadFUint8();
            _lastSn = ba->ReadFUint32();
            _data = ba->ReadStringF32();
            return ret;
        }
        static uint8_t s_magic[2] = {0x12, 0x21};
        XftpMsgHead::XftpMsgHead()
            : magic{s_magic[0], s_magic[1]},
              version(1),
              flag(0),
              length(0)
        {
        }
        int32_t XftpMessageDecoder::SerializeToStream(Stream::ptr stream, Message::ptr msg)
        {
            XftpMsgHead head;
            ByteArray::ptr ba = msg->ToByteArray();
            if (!ba)
            {
                XTEN_LOG_ERROR(g_logger) << "xftpMessageDecoder body serializeTo bytearray error";
                return -1;
            }
            ba->SetPosition(0);
            head.length = ba->GetSize();
            if (head.length > s_xftp_protocol_min_gzip_body_length)
            {
                // 需要压缩
                head.flag |= 0x1;
                ZlibStream::ptr zstream = ZlibStream::Creategzip(true);
                if (zstream->Write(ba, head.length) != Z_OK)
                {
                    XTEN_LOG_ERROR(g_logger) << "xftpMessageDecoder serializeTo gizp error";
                    return -2;
                }
                if (zstream->Flush() != Z_OK)
                {
                    XTEN_LOG_ERROR(g_logger) << "xftpMessageDecoder serializeTo gizp flush error";
                    return -3;
                }
                ba = zstream->GetByteArray();
                head.length = ba->GetSize();
            }
            head.length = htobe32(head.length);
            // 先写入xftp协议头
            if (stream->WriteFixSize(&head, sizeof(head)) <= 0)
            {
                // 写入出错
                XTEN_LOG_ERROR(g_logger) << "xftpMessageDecoder write head to stream error";
                return -4;
            }
            // 写正文数据
            if (stream->WriteFixSize(ba, ba->GetReadSize()) <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "xftpMessageDecoder write body to stream error";
                return -5;
            }
            return sizeof(head) + ba->GetReadSize();
        }
        // 从stream流中读取一个消息体
        Message::ptr ParseFromStream(Stream::ptr stream)
        {
            XftpMsgHead head;
            ByteArray::ptr ba = std::make_shared<ByteArray>();
            do
            {
                // 先读取Xftp头
                if (stream->ReadFixSize(&head, sizeof(head)) <= 0)
                {
                    XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder read Xftphead from stream error";
                    break;
                }
                // 验证魔数字段
                if (memcmp(head.magic, s_magic, sizeof(s_magic)) != 0)
                {
                    XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder compare Xftphead.magic error";
                    break;
                }
                if (head.version != 0x1)
                {
                    XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder confire Xftpprotocol version error != 1";
                    break;
                }
                head.length = be32toh(head.length);
                // 长度合法，读取指定长度的数据
                if (stream->ReadFixSize(ba, head.length) <= 0)
                {
                    // 读取失败
                    XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder parse read body from stream error";
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
                        XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder zlib parse from gzip error";
                        break;
                    }
                    if (zstream->Flush() != Z_OK)
                    {
                        XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder zlib flush from gzip error";
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
                    msg = std::make_shared<XftpRequest>();
                    break;
                case Message::MessageType::RESPONSE:
                    msg = std::make_shared<XftpResponse>();
                    break;
                default:
                    XTEN_LOG_ERROR(g_logger) << "invaild message type";
                    break;
                }
                if (msg)
                {
                    bool ret = msg->ParseFromByteArray(ba);
                    if (!ret)
                    {
                        XTEN_LOG_ERROR(g_logger) << "body parse from bytearray error";
                        return nullptr;
                    }
                }
                return msg;
            } while (false);
            XTEN_LOG_ERROR(g_logger) << "XftpMessageDecoder parse from stream return message error";
            return nullptr;
        }
    } // namespace xftp

} // namespace Xten
