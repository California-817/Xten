#ifndef __XTEN_XFTP_PROTOCOL_H__
#define __XTEN_XFTP_PROTOCOL_H__
#include "../protocol.h"
namespace Xten
{
    // 定长xftp协议头
    // ┌───────────────────────────────────────────────────────────────┐
    // │                      RockMessage Header                       │
    // ├─────────────┬─────────────┬─────────────┬─────────────────────┤
    // │   Magic     │   Version   │    Flag     │      Length         │
    // │  (2 bytes)  │  (1 byte)   │  (1 byte)   │    (4 bytes)        │
    // │  [0xab,cd]  │     1       │ bit0:gzip   │   Body Length       │
    // └─────────────┴─────────────┴─────────────┴─────────────────────┘
    struct RockMsgHead
    {
        RockMsgHead();
        uint8_t magic[2]; // 魔数(标识协议类型)
        uint8_t version;  // 版本
        uint8_t flag;     // 是否压缩body
        uint32_t length;  // body长度
    };

    // req
    // type--->消息类型 sn--->文件序列号 cmd--->请求方法[upload or download] time--->请求时间
    // md5--->文件的生成的md5值 fileName--->文件名 transSize--->已经发送大小 totalSize--->文件总大小
    // last--->是否是最后一个包 lastSn--->最后一个包的序列号   data--->base64编码后文件本体
    class XftpRequest : public Request
    {
    public:
        XftpRequest()=default;
        ~XftpRequest()=default;
        typedef std::shared_ptr<XftpRequest> ptr;
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;

    private:
        std::string _md5;
        std::string _name;
        uint64_t _transSize;
        uint64_t _totalSize;
        uint8_t _b_last;
        uint32_t _lastSn;
        std::string _data;
    };
    // rsp
    class XftpResponse : public Response
    {
        
    };
    // decoder
    class XftpMessageDecoder : public MessageDecoder
    {
    public:
        typedef std::shared_ptr<XftpMessageDecoder> ptr;
        // 将一个消息结构体直接发送到stream流中(负数出错，整数为发送数据长度)
        virtual int32_t SerializeToStream(Stream::ptr stream, Message::ptr msg) override;
        // 从stream流中读取一个消息体
        virtual Message::ptr ParseFromStream(Stream::ptr stream) override;
    };
} // namespace Xten

#endif
