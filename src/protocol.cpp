#include "../include/protocol.h"
#include "../include/util.h"
namespace Xten
{
    // 直接转成bytearray
    ByteArray::ptr Message::ToByteArray()
    {
        ByteArray::ptr ba = std::make_shared<ByteArray>();
        if (SerializeToByteArray(ba))
        {
            return ba;
        }
        return nullptr;
    }
    Request::Request()
        : _sn(0),
          _cmd(0),
          _time(Xten::TimeUitl::GetCurrentUS())
    {
    }
    // 将消息结构体序列化成bytearray
    bool Request::SerializeToByteArray(ByteArray::ptr ba)
    {
        if (ba)
        {
            ba->WriteFUint8(GetMessageType());
            ba->WriteFUint32(GetSn());
            ba->WriteFUint32(GetCmd());
            return true;
        }
        return false;
    }
    // 从bytearray中反序列化出消息结构体
    bool Request::ParseFromByteArray(ByteArray::ptr ba)
    {
        if (ba)
        {
            _sn = ba->ReadFUint32();
            _cmd = ba->ReadFUint32();
            return true;
        }
        return false;
    }
    Response::Response()
        : _sn(0),
          _cmd(0),
          _result(404),
          _resultString("unhandle")
    {
    }
    // 将消息结构体序列化成bytearray
    bool Response::SerializeToByteArray(ByteArray::ptr ba)
    {
        if (ba)
        {
            ba->WriteFUint8(GetMessageType());
            ba->WriteFUint32(GetSn());
            ba->WriteFUint32(GetCmd());
            ba->WriteFUint32(GetResult());
            ba->WriteStringF32(GetResultStr());
            return true;
        }
        return false;
    }
    // 从bytearray中反序列化出消息结构体
    bool Response::ParseFromByteArray(ByteArray::ptr ba)
    {
        if (ba)
        {
            _sn = ba->ReadFUint32();
            _cmd = ba->ReadFUint32();
            _result = ba->ReadFUint32();
            _resultString = ba->ReadStringF32();
            return true;
        }
        return false;
    }
    Notify::Notify()
        : _notify(0)
    {
    }
    bool Notify::SerializeToByteArray(ByteArray::ptr bytearray)
    {
        if (bytearray)
        {
            bytearray->WriteFUint8(GetMessageType());
            bytearray->WriteFUint32(GetNotify());
            return true;
        }
        return false;
    }
    bool Notify::ParseFromByteArray(ByteArray::ptr bytearray)
    {
        if (bytearray)
        {
            _notify = bytearray->ReadFUint32();
            return true;
        }
        return false;
    }
}