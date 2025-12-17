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
                               int nodelay,  // 0:disable(default), 1:enable  是否非延迟
                               int interval, // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                               int resend,   // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                               int nc)       // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制)

        {
            _kcp_cb = ikcp_create(_convid, this);
            // 设置输出函数
            ikcp_setoutput(_kcp_cb, &KcpSession::kcp_output_func);
            // 设置kcp配置
            ikcp_nodelay(_kcp_cb, nodelay, interval, resend, nc);
        }

        KcpSession::~KcpSession()
        {
            if (_kcp_cb)
                ikcp_release(_kcp_cb);
        }

        void KcpSession::Start()
        {
            auto self = shared_from_this();
            // 启动定时器定期update
            auto iom = Xten::IOManager::GetThis();
            XTEN_ASSERT(iom);
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
    } // namespace kcp

}