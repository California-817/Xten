#ifndef __XTEN_SOCKET_H__
#define __XTEN_SOCKET_H__
#include<memory>
#include"nocopyable.hpp"
namespace Xten
{
    /// @brief 封装socket结构
    class Socket:public std::enable_shared_from_this<Socket>,public NoCopyable
    {
    public:
        Socket();
        ~Socket();
    private:
        int _sockfd; //socket句柄
        
    };

}
#endif