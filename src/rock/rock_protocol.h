#ifndef __XTEN_ROCK_PROTOCOL_H__
#define __XTEN_ROCK_PROTOCOL_H__
#include "../protocol.h"
namespace Xten
{
    // rock协议正文(集成了protobuf序列化方案)
    class RockBody
    {
    public:
        RockBody() = default;
        // 设置正文
        void SetData(const std::string &data) { _data = data; }
        // 获取正文
        const std::string &GetData() const { return _data; }
        // 设置protobuf消息结构体并序列化
        template <class T>
        bool SetDataAsProtoBuf(const T &pbData)
        {
            try
            {
                pbData.SerializeToString(&_data);
                return true;
            }
            catch (...)
            {
            }
            return false;
        }
        // 获取并返回protobuf消息结构体
        template <class T>
        std::shared_ptr<T> GetDataAsProtoBuf()
        {
            if (_data.empty())
            {
                return nullptr;
            }
            std::shared_ptr<T> pbData = std::make_shared<T>();
            pbData->ParseFromString(_data);
            return pbData;
        }
        // 序列化到bytearray中
        virtual bool SerializeToByteArray(ByteArray::ptr ba) const;
        // 从bytearray中反序列化出string
        virtual bool ParseFromByteArray(ByteArray::ptr ba);
        virtual ~RockBody() = default;

    protected:
        std::string _data;
    };
    // 定长rock协议头
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
    // rock请求
    // ┌─────────────────────────────────────────────────────────────┐
    // │                         RockRequest                         │
    // ├─────────────┬─────────────┬─────────────────────────────────┤
    // │    Type     │     SN      │     CMD     │      Body         │
    // │  (1 byte)   │  (4 bytes)  │  (4 bytes)  │   (Variable)      │
    // │             │             │             │  protobufData     │
    // └─────────────┴─────────────┴─────────────┴───────────────────┘
    class RockResponse;
    class RockRequest : public Request, public RockBody
    {
    public:
        typedef std::shared_ptr<RockRequest> ptr;
        RockRequest() = default;
        virtual ~RockRequest() = default;
        // 根据请求的字段创建对应的响应
        std::shared_ptr<RockResponse> CreateResponse();
        virtual uint8_t GetMessageType() const override;
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;
        // 转字符串
        virtual std::string ToString() const override;
        // 获取name
        virtual const std::string &GetName() const override;
    };
    // rock响应
    // ┌─────────────────────────────────────────────────────────────┐
    // │                       RockResponse                          │
    // ├─────────────┬─────────────┬─────────────┬─────────────┬─────┤
    // │    Type     │     SN      │     CMD     │   Result    │ Str │
    // │  (1 byte)   │  (4 bytes)  │  (4 bytes)  │  (4 bytes)  │(Var)│
    // ├─────────────┴─────────────┴─────────────┴─────────────┴─────┤
    // │                    Result String                            │
    // │              Length(4 bytes) + String Content               │
    // └─────────────────────────────────────────────────────────────┘
    class RockResponse : public Response, public RockBody
    {
    public:
        typedef std::shared_ptr<RockResponse> ptr;
        RockResponse() = default;
        virtual ~RockResponse() = default;
        virtual uint8_t GetMessageType() const override;
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;
        // 转字符串
        virtual std::string ToString() const override;
        // 获取name
        virtual const std::string &GetName() const override;
    };
    // rock通知
    // ┌─────────────────────────────────────────────────────────────┐
    // │                         RockNotify                          │
    // ├─────────────┬─────────────┬─────────────────────────────────┤
    // │    Type     │  NotifyType |             Body                |
    // │  (1 byte)   │  (4 bytes)  |          (Variable)             │
    // └─────────────┴─────────────┴─────────────────────────────────┘
    class RockNotify : public Notify, public RockBody
    {
    public:
        typedef std::shared_ptr<RockNotify> ptr;
        RockNotify() = default;
        virtual ~RockNotify() = default;
        virtual uint8_t GetMessageType() const override;
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;
        // 转字符串
        virtual std::string ToString() const override;
        // 获取name
        virtual const std::string &GetName() const override;
    };

    // Rock协议消息的编解码器 ( stream <---- bytearray ----> Message )
    class RockMessageDecoder : public MessageDecoder
    {
    public:
        typedef std::shared_ptr<RockMessageDecoder> ptr;
        // 将一个消息结构体直接发送到stream流中(负数出错，整数为发送数据长度)
        virtual int32_t SerializeToStream(Stream::ptr stream, Message::ptr msg) override;
        // 从stream流中读取一个消息体
        virtual Message::ptr ParseFromStream(Stream::ptr stream) override;
    };
}
#endif