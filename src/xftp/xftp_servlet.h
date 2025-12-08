#ifndef __XTEN_XFTP_SERVLET_H__
#define __XTEN_XFTP_SERVLET_H__
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "xftp_protocol.h"
#include "xftp_session.h"
namespace Xten
{
    namespace xftp
    {
        //文件任务操作码
        enum XftpOptCmd
        {
            UPLOAD_TEST=-1,
            UPLOAD_COMMON_FILE=0,
            DOWNLOAD_COMMON_FILE=1,
        };
        /**
         * @brief XftpServlet封装
         */
        class XftpServlet
        {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<XftpServlet> ptr;

            /**
             * @brief 构造函数
             * @param[in] name 名称
             */
            XftpServlet(const std::string &name)
                : m_name(name) {}

            /**
             * @brief 析构函数
             */
            virtual ~XftpServlet() {}

            /**
             * @brief 处理请求
             * @param[in] request xftp请求
             * @param[in] response xftp响应
             * @param[in] session xftp连接
             * @return 是否处理成功
             */
            virtual int32_t handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                   Xten::SocketStream::ptr session) = 0;

            /**
             * @brief 返回XftpServlet名称
             */
            const std::string &getname() const { return m_name; }

        protected:
            /// 名称
            std::string m_name;
        };

        /**
         * @brief 函数式XftpServlet
         */
        class FunctionXftpServlet : public XftpServlet
        {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<FunctionXftpServlet> ptr;
            /// 函数回调类型定义
            typedef std::function<int32_t(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                          Xten::SocketStream::ptr session)>
                callback;

            /**
             * @brief 构造函数
             * @param[in] cb 回调函数
             */
            FunctionXftpServlet(callback cb, std::string name);
            // convenience ctor when only a callback is provided
            FunctionXftpServlet(callback cb);
            virtual int32_t handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                   Xten::SocketStream::ptr session) override;

        private:
            /// 回调函数
            callback m_cb;
        };

        class IXftpServletCreator
        {
        public:
            typedef std::shared_ptr<IXftpServletCreator> ptr;
            virtual ~IXftpServletCreator() {}
            virtual XftpServlet::ptr get() const = 0;
            virtual std::string getname() const = 0;
        };

        class HoldXftpServletCreator : public IXftpServletCreator
        {
        public:
            typedef std::shared_ptr<HoldXftpServletCreator> ptr;
            HoldXftpServletCreator(XftpServlet::ptr slt)
                : m_Xftpservlet(slt)
            {
            }

            XftpServlet::ptr get() const override
            {
                return m_Xftpservlet;
            }

            std::string getname() const override
            {
                return m_Xftpservlet->getname();
            }

        private:
            XftpServlet::ptr m_Xftpservlet;
        };

        template <class T>
        class XftpServletCreator : public IXftpServletCreator
        {
        public:
            typedef std::shared_ptr<XftpServletCreator> ptr;

            XftpServletCreator()
            {
            }

            XftpServlet::ptr get() const override
            {
                return std::make_shared<T>();
            }

            std::string getname() const override
            {
                return "TypeUtil::TypeToName<T>()"; // bug
            }
        };

        /**
         * @brief XftpServlet分发器
         */
        class XftpServletDispatch : public XftpServlet
        {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<XftpServletDispatch> ptr;
            /// 读写锁类型定义
            typedef RWMutex RWMutexType;

            /**
             * @brief 构造函数
             */
            XftpServletDispatch();
            virtual int32_t handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                   Xten::SocketStream::ptr session) override;

            /**
             * @brief 添加Xftpservlet
             * @param[in] name name
             * @param[in] slt serlvet
             */
            void addXftpServlet(const uint32_t &cmd, XftpServlet::ptr slt);

            /**
             * @brief 添加Xftpservlet
             * @param[in] name name
             * @param[in] cb FunctionXftpServlet回调函数
             */
            void addXftpServlet(const uint32_t &cmd, FunctionXftpServlet::callback cb);


            void addXftpServletCreator(const uint32_t &name, IXftpServletCreator::ptr creator);

            template <class T>
            void addXftpServletCreator(const uint32_t &name)
            {
                addXftpServletCreator(name, std::make_shared<XftpServletCreator<T>>());
            }


            /**
             * @brief 删除Xftpservlet
             * @param[in] name name
             */
            void delXftpServlet(const uint32_t &name);



            /**
             * @brief 通过name获取Xftpservlet
             * @param[in] name name
             * @return 返回对应的Xftpservlet
             */
            XftpServlet::ptr getXftpServlet(const uint32_t &name);


            /**
             * @brief 通过name获取Xftpservlet
             * @param[in] name name
             * @return 优先精准匹配,其次模糊匹配,最后返回默认
             */
            XftpServlet::ptr getMatchedXftpServlet(const uint32_t &name);

            void listAllXftpServletCreator(std::map<uint32_t, IXftpServletCreator::ptr> &infos);

        private:
            /// 读写互斥量
            RWMutexType m_mutex;
            /// 精准匹配Xftpservlet MAP
            std::unordered_map<uint32_t, IXftpServletCreator::ptr> m_datas;
        };

        // testservlet
        class TestServlet : public XftpServlet
        {
        public:
            typedef std::shared_ptr<TestServlet> ptr;
            TestServlet()
                : XftpServlet("test")
            {
            }
            /**
             * @brief 处理请求
             * @param[in] request xftp请求
             * @param[in] response xftp响应
             * @param[in] session xftp连接
             * @return 是否处理成功
             */
            virtual int32_t handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                   Xten::SocketStream::ptr session) override;

        };
        // 辅助方法
        int32_t DispatchReq2Worker(Xten::xftp::XftpRequest::ptr request, Xten::SocketStream::ptr session);

    } // namespace xftp

} // namespace Xten

#endif