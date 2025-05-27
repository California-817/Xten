#pragma once
#include "const.h"
#include "singleton.hpp"
#include "util.h"
// 协程网络库的日志模块
// 用宏函数的方式来简化日志的使用
//传入logger和level自动生成event并返回event的stringstream用于输入内容
//LogEventWrap在这一行结束自动析构进行日志的log操作
// \是宏续行符 表示宏替换之后当作一行去处理
#define XTEN_LOG_LEVEL(logger,level) \
        if(level>=logger->GetLevelLimit()) \
              Xten::LogEventWrap(std::make_shared<Xten::LogEvent>(logger,level,__FILE__,__LINE__,0, \
                        1,1,time(nullptr),"main thread")).GetSStream() 

#define XTEN_LOG_DEBUG(logger) XTEN_LOG_LEVEL(logger,Xten::LogLevel::DEBUG)  
#define XTEN_LOG_INFO(logger) XTEN_LOG_LEVEL(logger,Xten::LogLevel::INFO)  
#define XTEN_LOG_WARN(logger) XTEN_LOG_LEVEL(logger,Xten::LogLevel::WARN)  
#define XTEN_LOG_ERROR(logger) XTEN_LOG_LEVEL(logger,Xten::LogLevel::ERROR)  
#define XTEN_LOG_FATAL(logger) XTEN_LOG_LEVEL(logger,Xten::LogLevel::FATAL)  

//格式化输出日志
#define XTEN_LOG_FMT_LEVEL(logger,level,fmt,...) \
        if(level>=logger->GetLevelLimit())  \
                Xten::LogEventWrap(std::make_shared<Xten::LogEvent>(logger,level,__FILE__,__LINE__,0, \
                        1,1,time(nullptr),"main thread")).GetEvent()->format(fmt,__VA_ARGS__)

#define XTEN_LOG_FMT_DEBUG(logger,fmt,...) XTEN_LOG_FMT_LEVEL(logger,Xten::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define XTEN_LOG_FMT_INFO(logger,fmt,...) XTEN_LOG_FMT_LEVEL(logger,Xten::LogLevel::INFO,fmt,__VA_ARGS__)
#define XTEN_LOG_FMT_WARN(logger,fmt,...) XTEN_LOG_FMT_LEVEL(logger,Xten::LogLevel::WARN,fmt,__VA_ARGS__)
#define XTEN_LOG_FMT_ERROR(logger,fmt,...) XTEN_LOG_FMT_LEVEL(logger,Xten::LogLevel::ERROR,fmt,__VA_ARGS__)
#define XTEN_LOG_FMT_FATAL(logger,fmt,...) XTEN_LOG_FMT_LEVEL(logger,Xten::LogLevel::FATAL,fmt,__VA_ARGS__)

//获取root日志器
#define XTEN_LOG_ROOT() Xten::LoggerManager::GetInstance()->GetRootLogger()
//获取指定name的日志器
#define XTEN_LOG_NAME(name) Xten::LoggerManager::GetInstance()->GetLogger()
namespace Xten
{
    // 日志级别
    struct LogLevel
    {
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,
        };
        static const char *ToString(const LogLevel::Level &level); // level转字符串
        static LogLevel::Level ToLevel(const std::string &str);    // 字符串转level
    };
    // 日志事件 ---表示一条具体的日志
    class Logger;
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, std::string file, uint32_t line,
                 uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, std::string thread_name);
        std::shared_ptr<Logger> GetLogger();
        LogLevel::Level GetLevel();
        std::string FileName();
        uint32_t GetLine();
        uint64_t GetElapse();
        uint32_t GetThreaId();
        uint32_t GetFiberId();
        uint64_t GetTime();
        std::string GetThreadName();
        std::stringstream &GetSStream();   // 获取内容的流--用来保存日志内容
        std::string GetContent();          // 获取内容
        void format(const char *fmt, ...); // 格式化输入日志内容   format("log is %s",aaaaa);
    private:
        std::shared_ptr<Logger> _logger; // 输出依靠的日志器 ---有大用处
        LogLevel::Level _level;          // 日志级别
        std::string _file;               // 文件名
        uint32_t _line;                  // 文件行号
        uint32_t _elapse;                // 程序启动依赖的耗时(毫秒)
        uint32_t _thread_id;             // 线程id
        uint32_t _fiber_id;              // 协程id
        uint64_t _time;                  // 日志时间(秒)
        std::string _thread_name;        // 线程名称
        std::stringstream _ss;           // 日志的内容字段输出流
    };
    class Logsinker;
    // 格式化器
    //  *  %m 消息
    //  *  %p 日志级别
    //  *  %r 累计毫秒数
    //  *  %c 日志名称
    //  *  %t 线程id
    //  *  %n 换行
    //  *  %d 时间
    //  *  %f 文件名
    //  *  %l 行号
    //  *  %T 制表符
    //  *  %F 协程id
    //  *  %N 线程名称
    //  *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    class Formatter
    {
    public:
        typedef std::shared_ptr<Formatter> ptr;
        Formatter(const std::string &fmt_str = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
        void init();                                                                                                    // 初始化模板
        void format(std::ostream &out_stream, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr ev); // 向流中输出
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr ev);                    // 直接输出字符串
        // 格式化items
        class FormatterItem // 基类
        {
        public:
            typedef std::shared_ptr<FormatterItem> ptr;
            virtual void format(std::ostream &osm, std::shared_ptr<Logger> logger, LogLevel::Level lv, LogEvent::ptr ev) = 0;
            virtual ~FormatterItem() {} // 基类析构函数最好为虚函数
        protected:
        };
        std::string getFormatterPattern();
        bool isError();

    private:
        std::string _formatter_str;             // 格式化字符串
        std::vector<FormatterItem::ptr> _items; // 格式化items 根据_formatter_str生成
        bool _b_error;                          // 解析格式是否有错
    };

    // 日志器---管理落地类
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, LogEvent::ptr);
        void SetLevelLimit(LogLevel::Level);
        LogLevel::Level GetLevelLimit();
        void AddSinkers(const std::string &sink_name, std::shared_ptr<Logsinker> sinker);
        void DelSinkers(const std::string &sink_name);
        void ClearSinkers();
        std::string GetName();
        void info(LogEvent::ptr ev);
        void debug(LogEvent::ptr ev);
        void warn(LogEvent::ptr ev);
        void error(LogEvent::ptr ev);
        void fatal(LogEvent::ptr ev);
        Formatter::ptr GetFormatter();
        void SetFormatter(Formatter::ptr formatter); // 直接传入格式化器
        void SetFormatter(const char *fmt_str);      // 传入格式化字符串 %d{xxx} %s %g ...
        void SetRootLogger(Logger::ptr root_logger); // 设置主logger
    private:
        std::string _name;                                                    // 日志名称
        LogLevel::Level _level_limit;                                         // limit日志级别
        std::unordered_map<std::string, std::shared_ptr<Logsinker>> _sinkers; // 所有的日志输出器及其名字
        Formatter::ptr _formatter;                                            // 日志格式化器---logger级别的
        Logger::ptr _root_logger;                                             // 主日志器
    };
    // 日志器落地类
    class Logsinker
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<Logsinker> ptr;
        Logsinker(LogLevel::Level level = LogLevel::DEBUG) : _level_limit(level), _b_has_formatter(false) {}
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev) = 0;
        bool has_formatter() { return _b_has_formatter; }
        Formatter::ptr GetFormatter()
        {
            return _formatter;
        }
        void SetFormatter(Formatter::ptr formatter)
        {
            _formatter = formatter;
            _b_has_formatter = true;
        }
        LogLevel::Level GetLevelLimit()
        {
            return _level_limit;
        }
        void SetLevelLimit(LogLevel::Level level)
        {
            _level_limit = level;
        }
        virtual ~Logsinker() {}

    protected:
        LogLevel::Level _level_limit; // 每个sinks的日志级别
        bool _b_has_formatter;        // 是否有格式化器
        Formatter::ptr _formatter;    // 格式化器 --由所属的logger赋值 或者自定义
    };
    // 标准输出落地类
    class StdoutLogsinker : public Logsinker
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<StdoutLogsinker> ptr;
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev);
    };
    // 文件输出落地类---单文件
    class FileLogsinker : public Logsinker
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<FileLogsinker> ptr;
        FileLogsinker(const std::string &filename);
        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev);
        bool reopen(); // 重新打开
    private:
        std::string _log_filename;  // 文件名 路径+name
        std::ofstream _file_stream; // 文件流
        ino64_t _last_opentime;     // 上次操作时间
    };
    // 用于封装event 保证在构造出event的这一行结束的时候会自动进行输出日志 --RAII的思想 临时遍历生命周期为一行 自动析构
    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr event) : _event(event) {}
        ~LogEventWrap(){
            // std::cout<<"wrap 析构"<<std::endl;
            _event->GetLogger()->log(_event->GetLevel(),_event);
        }
        LogEvent::ptr GetEvent(){
            return _event;
        }
        std::ostream& GetSStream(){
            return _event->GetSStream();
        }
    private:
        LogEvent::ptr _event;
    };
    // 单例日志器管理类
    class LoggerManager : public singleton<LoggerManager>
    {
        friend class singleton<LoggerManager>;

    public:
        Logger::ptr GetLogger(const std::string &name);
        bool SetLogger(const std::string &name, Logger::ptr logger);
        Logger::ptr GetRootLogger(); // 获取root的logger
        void init();

    private:
        LoggerManager();
        LoggerManager(const LoggerManager &) = delete;
        LoggerManager &operator=(const LoggerManager &) = delete;

    private:
        std::unordered_map<std::string, Logger::ptr> _loggers_map; // 管理所有logger的map
        Logger::ptr _root_logger;                                  // 主logger日志器
    };
}
