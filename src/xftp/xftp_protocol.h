#ifndef __XTEN_XFTP_PROTOCOL_H__
#define __XTEN_XFTP_PROTOCOL_H__
#include "../protocol.h"
#include <sstream>
namespace Xten
{
    namespace xftp
    {
        // 定长xftp协议头
        // ┌───────────────────────────────────────────────────────────────┐
        // │                      XftpkMessage Header                       │
        // ├─────────────┬─────────────┬─────────────┬─────────────────────┤
        // │   Magic     │   Version   │    Flag     │      Length         │
        // │  (2 bytes)  │  (1 byte)   │  (1 byte)   │    (4 bytes)        │
        // │  [0xab,cd]  │     1       │ bit0:gzip   │   Body Length       │
        // └─────────────┴─────────────┴─────────────┴─────────────────────┘
        struct XftpMsgHead
        {
            XftpMsgHead();
            uint8_t magic[2]; // 魔数(标识协议类型)
            uint8_t version;  // 版本
            uint8_t flag;     // 是否压缩body
            uint32_t length;  // body长度
        };

        class XftpResponse;
        // req
        // type--->消息类型 sn--->文件序列号 cmd--->请求方法[upload or download] time--->请求时间
        // md5--->文件的生成的md5值 fileName--->文件名 transSize--->已经发送大小 totalSize--->文件总大小
        // last--->是否是最后一个包 lastSn--->最后一个包的序列号   data--->正文数据
        class XftpRequest : public Request
        {
        public:
            XftpRequest() = default;
            ~XftpRequest() = default;
            typedef std::shared_ptr<XftpRequest> ptr;
            // 根据请求创建响应
            std::shared_ptr<XftpResponse> CreateResponse();

            // 将消息结构体序列化成bytearray
            virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
            // 从bytearray中反序列化出消息结构体
            virtual bool ParseFromByteArray(ByteArray::ptr ba) override;
            virtual uint8_t GetMessageType() const override
            {
                return Message::MessageType::REQUEST;
            }
            // 转字符串
            virtual std::string ToString() const override
            {
                std::stringstream ss;
                ss << "[XftpRequest sn=" << _sn
                   << " cmd=" << _cmd
                   << " body.length=" << _data.size()
                   << "]";
                return ss.str();
            }
            // 获取name
            virtual const std::string &GetName() const override
            {
                static std::string name = "XftpRequest";
                return name;
            }
            // 所有get，set方法
            std::string GetFileName() const { return _name; }
            std::string GetMd5() const { return _md5; }
            uint64_t GetTransSize() const { return _transSize; }
            uint64_t GetTotalSize() const { return _totalSize; }
            uint8_t IsLast() const { return _b_last; }
            uint32_t GetLastSn() const { return _lastSn; }
            std::string GetData() const { return _data; }
            void SetFileName(const std::string &name) { _name = name; }
            void SetMd5(const std::string &md5) { _md5 = md5; }
            void SetTransSize(uint64_t size) { _transSize = size; }
            void SetTotalSize(uint64_t size) { _totalSize = size; }
            void SetIsLast(uint8_t last) { _b_last = last; }
            void SetLastSn(uint32_t sn) { _lastSn = sn; }
            void SetData(const std::string &data) { _data = data; }

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
        // type 类型   cmd 操作  sn 与请求对应   result 响应码   resultstr 响应字符串
        // 其余的一致
        class XftpResponse : public Response
        {
        public:
            friend class XftpRequest;
            typedef std::shared_ptr<XftpResponse> ptr;
            XftpResponse() = default;
            virtual ~XftpResponse() = default;
            virtual uint8_t GetMessageType() const override
            {
                return Message::MessageType::RESPONSE;
            }
            // 将消息结构体序列化成bytearray
            virtual bool SerializeToByteArray(ByteArray::ptr ba) override;
            // 从bytearray中反序列化出消息结构体
            virtual bool ParseFromByteArray(ByteArray::ptr ba) override;
            // 转字符串
            virtual std::string ToString() const override
            {
                std::stringstream ss;
                ss << "[XftpResponse sn=" << _sn
                   << " cmd=" << _cmd
                   << " body.length=" << _data.size()
                   << "]";
                return ss.str();
            }
            // 获取name
            virtual const std::string &GetName() const override
            {
                static std::string name = "XftpResponse";
                return name;
            }
            // 所有get，set方法
            // 所有get，set方法
            std::string GetFileName() const { return _name; }
            std::string GetMd5() const { return _md5; }
            uint64_t GetTransSize() const { return _transSize; }
            uint64_t GetTotalSize() const { return _totalSize; }
            uint8_t IsLast() const { return _b_last; }
            uint32_t GetLastSn() const { return _lastSn; }
            std::string GetData() const { return _data; }
            void SetFileName(const std::string &name) { _name = name; }
            void SetMd5(const std::string &md5) { _md5 = md5; }
            void SetTransSize(uint64_t size) { _transSize = size; }
            void SetTotalSize(uint64_t size) { _totalSize = size; }
            void SetIsLast(uint8_t last) { _b_last = last; }
            void SetLastSn(uint32_t sn) { _lastSn = sn; }
            void SetData(const std::string &data) { _data = data; }

        private:
            std::string _md5;
            std::string _name;
            uint64_t _transSize;
            uint64_t _totalSize;
            uint8_t _b_last;
            uint32_t _lastSn;
            std::string _data;
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
    }
} // namespace Xten

#endif
