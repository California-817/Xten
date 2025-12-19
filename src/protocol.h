#ifndef __XTEN_PROTOCOL_H__
#define __XTEN_PROTOCOL_H__
#include "bytearray.h"
#include "stream.h"
namespace Xten
{
    class Message
    {
    public:
        typedef std::shared_ptr<Message> ptr;
        enum MessageType
        {
            REQUEST = 1,  // 请求消息
            RESPONSE = 2, // 响应消息
            NOTIFY = 3,   // 通知消息
            PING = 4,     // ping
            PONG = 5,     // pong
            CLOSE = 6,    // close
        };
        Message() = default;
        virtual ~Message() = default;
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) = 0;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) = 0;
        // 消息体直接转成bytearray
        ByteArray::ptr ToByteArray();
        // 转字符串
        virtual std::string ToString() const = 0;
        // 获取name
        virtual const std::string &GetName() const = 0;
        // 获取消息类型
        virtual uint8_t GetMessageType() const = 0;
    };
    // 消息编解码器
    class MessageDecoder
    {
    public:
        typedef std::shared_ptr<MessageDecoder> ptr;
        // 将一个消息结构体直接发送到stream流中
        virtual int32_t SerializeToStream(Stream::ptr stream, Message::ptr msg) = 0;
        // 从stream流中读取一个消息体
        virtual Message::ptr ParseFromStream(Stream::ptr stream) = 0;
    };
    // 请求消息
    class Request : public Message
    {
    public:
        typedef std::shared_ptr<Request> ptr;
        Request();
        virtual ~Request() = default;
        // 设置sn
        void SetSn(uint32_t sn)
        {
            _sn = sn;
        }
        // 设置cmd
        void SetCmd(uint32_t cmd)
        {
            _cmd = cmd;
        }
        // 设置超时时间
        void SetTime(uint64_t time_us)
        {
            _time = time_us;
        }
        // 获取sn
        uint32_t GetSn() const
        {
            return _sn;
        }
        // 设置cmd
        uint32_t GetCmd() const
        {
            return _cmd;
        }
        // 设置超时时间
        uint64_t GetTime() const
        {
            return _time;
        }
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;

    protected:
        uint32_t _sn;   // 请求的唯一序列号
        uint32_t _cmd;  // 用于标识这个请求的方法类型（不同方法类型对应不同处理）
        uint64_t _time; // 开始当前请求的的当前时间 us
    };
    // 响应消息
    class Response : public Message
    {
    public:
        typedef std::shared_ptr<Response> ptr;
        Response();
        uint32_t GetSn() const { return _sn; }
        uint32_t GetCmd() const { return _cmd; }
        uint32_t GetResult() const { return _result; }
        const std::string &GetResultStr() const { return _resultString; }
        void SetSn(uint32_t v) { _sn = v; }
        void SetCmd(uint32_t v) { _cmd = v; }
        void SetResult(uint32_t v) { _result = v; }
        void SetResultStr(const std::string &v) { _resultString = v; }
        // 将消息结构体序列化成bytearray
        virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
        // 从bytearray中反序列化出消息结构体
        virtual bool ParseFromByteArray(ByteArray::ptr ba) override;

    protected:
        uint32_t _sn;              // 与请求对应的序列号
        uint32_t _cmd;             // 与请求一致的操作类型
        uint32_t _result;          // 响应码
        std::string _resultString; // 响应码对应响应字符串
    };
    // 通知消息
    class Notify : public Message
    {
    public:
        typedef std::shared_ptr<Notify> ptr;
        Notify();
        uint32_t GetNotify() const { return _notify; }
        void GetNotify(uint32_t v) { _notify = v; }

        virtual bool SerializeToByteArray(ByteArray::ptr bytearray) override;
        virtual bool ParseFromByteArray(ByteArray::ptr bytearray) override;

    protected:
        uint32_t _notify; // 通知类型
    };
}
#endif