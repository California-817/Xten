#include "kcp_session.h"
#include "kcp_listener.h"
#include "third_part/ikcp.h"
#include "../iomanager.h"
#include "../macro.h"
#include "../util.h"
namespace Xten
{
    namespace kcp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        KcpSession::KcpSession(std::weak_ptr<Socket> udp_channel, std::weak_ptr<KcpListener> listener, Address::ptr remote_addr, uint32_t convid,
                               int nodelay,                                 // 0:disable(default), 1:enable  是否非延迟
                               int interval,                                // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                               int resend,                                  // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                               int nc, int mtxsize, int sndwnd, int rcvwnd) // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制)

            : _sendque_cond(_sendque_mtx),
              _kcpcb_cond(_kcpcb_mtx),
              _sem(1),
              _convid(convid),
              _udp_channel(udp_channel),
              _listener(listener),
              _remote_addr(remote_addr)
        {
            _kcp_cb = ikcp_create(_convid, this);
            XTEN_ASSERT(_kcp_cb);
            // 设置输出函数
            ikcp_setoutput(_kcp_cb, &KcpSession::kcp_output_func);
            // 设置kcp配置
            ikcp_nodelay(_kcp_cb, nodelay, interval, resend, nc);
            //  设置mtu
            ikcp_setmtu(_kcp_cb, mtxsize);
            // 设置窗口大小
            ikcp_wndsize(_kcp_cb, sndwnd, rcvwnd);
        }

        KcpSession::~KcpSession()
        {
            Close();
            if (_kcp_cb)
                ikcp_release(_kcp_cb); // 防止内存泄露
        }
        void KcpSession::Close()
        {
            std::call_once(_once_close, [this]()
                           {
                //关闭连接前最好进行一次数据的刷新----比如forceClose场景，写协程只是按顺序将包文放到kcpcb中，其实没有真正发送，直接关闭就导致包文丢失
                {
                    MutexType::Lock lock(_kcpcb_mtx);
                    ikcp_update(_kcp_cb,TimeUitl::GetCurrentMS());
                    ikcp_flush(_kcp_cb); //flush data in kcpcb buffer
                }
                auto listener=_listener.lock();
                if(listener)
                    listener->onSessionClose(_remote_addr->toString());
                _b_close=true;
                _sendque_cond.signal();
                _kcpcb_cond.signal(); });
        }
        void KcpSession::Start()
        {
            auto self = shared_from_this();
            auto iom = Xten::IOManager::GetThis();
            XTEN_ASSERT(iom);
            iom->Schedule(std::bind(&KcpSession::dopacketOutput, this, self));
        }

        // 读到一个message报文
        Message::ptr KcpSession::ReadMessage(KcpSession::READ_ERRNO &error)
        {
            Timer::ptr timer;
            if (_read_timeout_ms > 0)
                // 启动超时定时器
                timer = Xten::IOManager::GetThis()->addTimer(_read_timeout_ms,
                                                             std::bind(&KcpSession::notifyReadTimeout, this));
            do
            {
                MutexType::Lock lock(_kcpcb_mtx);
                while (ikcp_peeksize(_kcp_cb) <= 0 &&
                       !_b_close && !_b_read_error &&
                       !_b_read_timeout)
                {
                    // 提前获知接收队列中包文情况---没有完整包文
                    _kcpcb_cond.wait();
                    // XTEN_LOG_DEBUG(g_logger) <<"peeksize="<<ikcp_peeksize(_kcp_cb);
                }
                // XTEN_LOG_DEBUG(g_logger) << "read out";
                // 循环条件不满足---跳出
                if (_b_read_timeout)
                {
                    _b_read_timeout = false;
                    XTEN_LOG_DEBUG(g_logger) << "read time out";
                    error = READ_ERRNO::READ_TIMEOUT;
                    break;
                }
                else
                {
                    if (timer) // 还没有超时
                        timer->cancel();
                    if (_b_close)
                    {
                        error = READ_ERRNO::SESSION_CLOSE;
                        XTEN_LOG_DEBUG(g_logger) << "kcpsession has been closed";
                        break;
                    }
                    else if (_b_read_error)
                    {
                        error = READ_ERRNO::READ_ERROR;
                        XTEN_LOG_DEBUG(g_logger) << "kcpsession read error,errno=" << _read_error_code
                                                 << ",errstr=" << strerror(_read_error_code);
                        break;
                    }
                    else
                    {
                        // 读取数据并返回req
                        char *buf = (char *)malloc(1024 + 512);
                        //  char buf[1024+512]; // 固定缓冲区，避免ByteArray动态分配导致的崩溃---不直接在栈上开辟[因为是协程环境]
                        int ret = 0;
                        while (ret = ikcp_recv(_kcp_cb, buf, 1024 + 512))
                        {
                            if (ret == -3)
                            {
                                // 缓冲区不足---对方发来的包太大了[看作非法]
                                notifyReadError(ret);
                                free(buf);
                                error = READ_ERRNO::READ_ERROR;
                                return nullptr;
                            }
                            else if (ret > 0)
                                break;
                            else
                                continue;
                        }
                        buf[ret] = 0;
                        // XTEN_LOG_INFO(g_logger) << "data=" << buf;
                        // success---反序列化
                        ByteArray::ptr ba = std::make_shared<ByteArray>(ret);
                        ba->Write(buf, ret);
                        free(buf);
                        ba->SetPosition(0);
                        Message::MessageType type = (Message::MessageType)ba->ReadFUint8();
                        // XTEN_LOG_DEBUG(g_logger) << "type=" << type;
                        if (type == Message::MessageType::REQUEST)
                        {
                            KcpRequest::ptr req = std::make_shared<KcpRequest>();
                            req->ParseFromByteArray(ba);
                            // XTEN_LOG_DEBUG(g_logger) << "req=" << req->ToString();
                            return req;
                        }
                        else if (type == Message::MessageType::NOTIFY)
                        {
                            // todo
                        }
                        else
                        {
                            // todo
                        }
                    }
                }

            } while (false);
            return nullptr;
        }

        // 原始udp包处理 [data]
        void KcpSession::packetInput(const char *data, size_t len)
        {
            { // 对ikcpcb的访问需要加锁
                MutexType::Lock lock(_kcpcb_mtx);
                int ret = ikcp_input(_kcp_cb, data, len);
                if (ret < 0)
                {
                    notifyReadError(ret);
                    return;
                }
            }
            // success
            notifyReadEvent();
        }

        // 强制关闭连接
        void KcpSession::ForceClose()
        {
            SendMessage(nullptr);
        }
        // 发送一个包文--->into queue
        void KcpSession::SendMessage(Message::ptr msg)
        {
            {
                MutexType::Lock _lock(_sendque_mtx);
                _sendque.push_back(msg);
            }
            _sendque_cond.signal();
        }

        void KcpSession::dopacketOutput(std::shared_ptr<KcpSession> self)
        {
            XTEN_LOG_DEBUG(g_logger) << "KcpSession Send Fiber begin";
            _sem.wait(); // 1-0
            try
            {
                do
                {
                    std::list<Message::ptr> tmp;
                    {
                        MutexType::Lock lock(_sendque_mtx);
                        while (_sendque.empty() &&
                               !_b_close && !_b_read_error && !_b_write_error)
                        {
                            _sendque_cond.wait();
                        }
                        tmp.swap(_sendque);
                    }
                    // send
                    bool b_force_stop = false;
                    for (auto &rsp : tmp)
                    {
                        if (!rsp) // force close or timeout
                        {
                            b_force_stop = true;
                            break;
                        }
                        else
                        {
                            // send rsp
                            if (!write(rsp))
                            {
                                b_force_stop = true;
                                break;
                            }
                        }
                    }
                    if (b_force_stop)
                        break;
                } while (!_b_close && !_b_read_error && !_b_write_error);
                // std::cout << _b_close << _b_read_error << _b_write_error << "!!!!!!!!!!" << std::endl;
                Close(); // 关闭连接
            }
            catch (...)
            {
                XTEN_LOG_ERROR(g_logger) << "KcpSession Send Fiber catch Exception";
            }
            _sem.post(); // 0-1
            XTEN_LOG_DEBUG(g_logger) << "KcpSession Send Fiber end";
        }
        // 传给kcpcb的内部输出回调函数
        int KcpSession::kcp_output_func(const char *buf, int len, struct IKCPCB *kcp, void *user)
        {
            // 不加锁调用----因为这个函数实际上是update内部调用的一个函数
            //  将数据发送到socket中
            KcpSession *session = static_cast<KcpSession *>(user);
            auto udpChannel = session->_udp_channel.lock();
            if (udpChannel &&
                udpChannel->SendTo(buf, len, session->_remote_addr) > 0)
                // 非空发送
                return 0;
            // 发送出错误
            XTEN_LOG_DEBUG(g_logger) << "kcpsession sendto msg error remoteaddr="
                                     << session->_remote_addr->toString();
            // 通知错误
            session->notifyWriteError(errno);
            // 上层不会用到返回值
            return -1;
        }
        void KcpSession::update()
        {
            // 加锁调用
            MutexType::Lock lock(_kcpcb_mtx);
            ikcp_update(_kcp_cb, TimeUitl::GetCurrentMS());
        }

        // 通知read超时
        void KcpSession::notifyReadTimeout()
        {
            _b_read_timeout = true;
            _kcpcb_cond.signal();
        }
        // 通知read就绪
        void KcpSession::notifyReadEvent()
        {
            _kcpcb_cond.signal();
        }
        // 通知读错误
        void KcpSession::notifyReadError(int code)
        {
            std::call_once(_once_readError, [this, code]()
                           {
                _read_error_code=code;
                _b_read_error=true;
                _kcpcb_cond.signal();
                _sendque_cond.signal(); });
        }
        // 通知写错误
        void KcpSession::notifyWriteError(int code)
        {
            std::call_once(_once_writeError, [this, code]()
                           {
                _write_error_code=code;
                _b_write_error=true;
                _kcpcb_cond.signal();
                _sendque_cond.signal(); });
        }

        // 发送协程调用: [发送队列--->kcpcb发送缓冲区区]
        bool KcpSession::write(Message::ptr rsp)
        {
            // 对包的大小有限制
            ByteArray::ptr ba = std::make_shared<ByteArray>(1024 + 512);
            rsp->SerializeToByteArray(ba);
            ba->SetPosition(0); // 写操作移动了position
            // 涉及对kcpcb的访问---需要加锁
            MutexType::Lock lock(_kcpcb_mtx);
            // XTEN_LOG_DEBUG(g_logger)<<"send rsp size="<<ba->GetReadSize();
            int ret = ikcp_send(_kcp_cb, (const char *)ba->GetBeginNodePtr(), ba->GetReadSize());
            if (ret < 0)
            {
                // send error
                notifyWriteError(ret);
                return false;
            }
            return true;
        }
    } // namespace kcp
}