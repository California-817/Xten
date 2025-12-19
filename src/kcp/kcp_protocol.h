#ifndef __XTEN_KCP_PROTOCOL_H__
#define __XTEN_KCP_PROTOCOL_H__
#include "../protocol.h"
#include <sstream>
namespace Xten
{
    namespace kcp
    {
        // common game protocol
        // 只是临时测试逻辑正常，后面需要对协议进行完善....todo
        // kcp响应
        class KcpRequest;
        class KcpResponse : public Response
        {
        public:
            typedef std::shared_ptr<KcpResponse> ptr;
            friend class KcpRequest;
            // 将消息结构体序列化成bytearray
            virtual bool SerializeToByteArray(ByteArray::ptr ba)
            {
                Response::SerializeToByteArray(ba);
                ba->WriteFUint16(_magic);
                ba->WriteFUint8(_version);
                ba->WriteFUint16(_remain);
                ba->WriteStringF32(_data);
                return true;
            }
            // 从bytearray中反序列化出消息结构体
            virtual bool ParseFromByteArray(ByteArray::ptr ba)
            {

                Response::ParseFromByteArray(ba);
                _magic = ba->ReadFUint16();
                _version = ba->ReadFUint8();
                _remain = ba->ReadFUint16();
                _data = ba->ReadStringF32();
                return true;
            }
            // 转字符串
            virtual std::string ToString() const
            {
                std::stringstream ss;
                ss << "[sn=" << _sn << "][cmd=" << _cmd << "]"
                   << "[magic=" << _magic
                   << "][version" << _version << "][result="
                   << _result << "][resultstr=" << _resultString << "][data=" << _data;
                return ss.str();
            }
            // 获取name
            virtual const std::string &GetName() const
            {
                static std::string name = "KcpResponse";
                return name;
            }
            // 获取消息类型
            virtual uint8_t GetMessageType() const
            {
                return Message::MessageType::RESPONSE;
            }

            // data interface
            void SetData(const std::string &data) { _data = data; }
            std::string GetData() const { return _data; }

            // protobuf data
            template <class T>
            bool SetDataAsPB(const T &pbData)
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
            template <class T>
            std::shared_ptr<T> GetDataAsPB()
            {
                try
                {
                    if (_data.empty())
                        return nullptr;
                    std::shared_ptr<T> pbptr = std::make_shared<T>();
                    pbptr->ParseFromString(_data);
                    return pbptr;
                }
                catch (...)
                {
                }
                return nullptr;
            }

        private:
            // uint32_t _sn;              // 与请求对应的序列号
            // uint32_t _cmd;             // 与请求一致的操作类型
            // uint32_t _result;          // 响应码
            // std::string _resultString; // 响应码对应响应字符串

            uint16_t _magic;  // 魔数
            uint8_t _version; // 版本
            uint16_t _remain; // 保留字段

            std::string _data; // 正文数据
        };

        //  kcp请求
        class KcpRequest : public Request
        {
        public:
            typedef std::shared_ptr<KcpRequest> ptr;
            std::shared_ptr<KcpResponse> CreateKcpResponse()
            {
                auto rsp = std::make_shared<KcpResponse>();
                rsp->SetCmd(_cmd);
                rsp->SetSn(_sn);
                rsp->_magic = _magic;
                rsp->_version = _version;
                rsp->_remain = _remain;
                return rsp;
            }
            // 将消息结构体序列化成bytearray
            virtual bool SerializeToByteArray(ByteArray::ptr ba)
            {
                Request::SerializeToByteArray(ba);
                ba->WriteFUint16(_magic);
                ba->WriteFUint8(_version);
                ba->WriteFUint16(_remain);
                ba->WriteStringF32(_data);
                return true;
            }
            // 从bytearray中反序列化出消息结构体
            virtual bool ParseFromByteArray(ByteArray::ptr ba)
            {
                Request::ParseFromByteArray(ba);
                _magic = ba->ReadFUint16();
                _version = ba->ReadFUint8();
                _remain = ba->ReadFUint16();
                _data = ba->ReadStringF32();
                return true;
            }
            // 转字符串
            virtual std::string ToString() const
            {
                std::stringstream ss;
                ss << "[sn=" << _sn << "]"
                   << "[magic=" << _magic
                   << "][version" << _version
                   << "][cmd=" << _cmd << "][data=" << _data;
                return ss.str();
            }
            // 获取name
            virtual const std::string &GetName() const
            {
                static std::string name = "KcpRequest";
                return name;
            }
            // 获取消息类型
            virtual uint8_t GetMessageType() const
            {
                return Message::MessageType::REQUEST;
            }

            // data interface
            void SetData(const std::string &data) { _data = data; }
            std::string GetData() const { return _data; }

            // protobuf data
            template <class T>
            bool SetDataAsPB(const T &pbData)
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
            template <class T>
            std::shared_ptr<T> GetDataAsPB()
            {
                try
                {
                    if (_data.empty())
                        return nullptr;
                    std::shared_ptr<T> pbptr = std::make_shared<T>();
                    pbptr->ParseFromString(_data);
                    return pbptr;
                }
                catch (...)
                {
                }
                return nullptr;
            }

            void SetMagic(uint16_t magic) { _magic = magic; }
            uint16_t GetMagic() const { return _magic; }

            void SetVersion(uint8_t version) { _version = version; }
            uint8_t GetVersion() const { return _version; }

        private:
            // uint32_t _sn;   // 请求的唯一序列号
            // uint32_t _cmd;  // 用于标识这个请求的方法类型（不同方法类型对应不同处理）
            uint16_t _magic = 0x0817; // 魔数
            uint8_t _version = 1.0;   // 版本
            uint16_t _remain;         // 保留字段
            std::string _data;        // 正文数据
        };
    }
} // namespace Xten

#endif