#ifndef __XTEN_KCP_PROTOCOL_H__
#define __XTEN_KCP_PROTOCOL_H__
#include <memory>
#include <sstream>
#include <arpa/inet.h>
#include "protobuf/msg_body.pb.h"
#include "protobuf/msg_id.pb.h"
namespace Xten
{
    namespace kcp
    {
        typedef unsigned short int KcpMsgLenType;
        typedef unsigned int KcpMsgIdType;
        // common game protocol
        class KcpMessage
        {
        public:
            typedef std::shared_ptr<KcpMessage> ptr;
            void SetMsgId(KcpMsgIdType id) { _id = id; }
            KcpMsgIdType GetMsgId() const { return _id; }
            /// @brief 获取body长度
            /// @return len
            KcpMsgLenType GetBodyLength() const {return _len;}
            template <class T>
            bool SetMsgBodyAsPB(T &pbData)
            {
                try
                {
                    pbData.SerializeToString(&_body);
                    _len=_body.size();
                    return true;
                }
                catch (...)
                {
                }
                return false;
            }
            template <class T>
            std::shared_ptr<T> GetMsgBodyAsPB()
            {
                if (_body.empty())
                {
                    return nullptr;
                }
                std::shared_ptr<T> pbData = std::make_shared<T>();
                pbData->ParseFromString(_body);
                return pbData;
            }
            /// @brief 序列化到外部提供的buffer中
            /// @param buffer 
            /// @param len 
            /// @return 空间不足-1 成功为真实大小
            size_t SerializeToBuffer(char *buffer, size_t len)
            {
                size_t rl_size = sizeof(KcpMsgLenType) + sizeof(KcpMsgIdType) + _len;
                if (rl_size >= len)
                    return -1;
                *(KcpMsgLenType *)buffer = htons(_len);
                buffer += sizeof(KcpMsgLenType);
                *(KcpMsgIdType *)buffer = htonl(_id);
                buffer += sizeof(KcpMsgIdType);
                memcpy(buffer, _body.c_str(), _body.size());
                return rl_size;
            }
            /// @brief 从buffer中反序列化出msg
            /// @param buf 
            /// @return 是否成功
            bool ParseFromBuffer(const char *buf)
            {
                _len = ntohs(*(KcpMsgLenType *)buf);
                buf += sizeof(KcpMsgLenType);
                _id = ntohl(*(KcpMsgIdType *)buf);
                const google::protobuf::EnumDescriptor *descriptor = Proto::MsgId_descriptor();
                if (descriptor->FindValueByNumber(_id) == nullptr)
                {
                    return false;
                }
                buf += sizeof(KcpMsgIdType);
                _body = std::string(buf);
                return true;
            }

        private:
            KcpMsgLenType _len;
            KcpMsgIdType _id;
            std::string _body;
        };
    }
} // namespace Xten

#endif