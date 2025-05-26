#include "../include/log.h"
namespace Xten
{
    const char *LogLevel::ToString(const LogLevel::Level &level) // level转字符串
    {
        switch (level)
        { // 定义宏函数来简化代码 #是用来转成字符串
#define XX(LEV)                        \
    \ 
            case LogLevel::Level::LEV: \
    return #LEV;                       \
    \    
                break;
            XX(INFO);
            XX(DEBUG);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            break;
        }
        return "UNKNOW";
    }
    LogLevel::Level LogLevel::ToLevel(const std::string &str) // 字符串转level
    {
#define XX(Lv, Cmp)  \
    if (#Cmp == str) \
        return LogLevel::Level::Lv;
        XX(INFO, info);
        XX(DEBUG, debug);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);
        XX(UNKNOW, unknow);
        XX(INFO, INFO);
        XX(DEBUG, DEBUG);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        XX(UNKNOW, UNKNOW);
#undef XX
        return LogLevel::Level::UNKNOW;
    }
    LogEvent::LogEvent(Logger::ptr logger, LogLevel::Level level, std::string file, uint32_t line,
                       uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time, std::string thread_name)
        : _logger(logger),
          _level(level),
          _file(file),
          _line(line),
          _elapse(elapse),
          _thread_id(thread_id),
          _fiber_id(fiber_id),
          _time(time),
          _thread_name(thread_name) {}
    Logger::ptr LogEvent::GetLogger()
    {
        return _logger;
    }
    LogLevel::Level LogEvent::GetLevel()
    {
        return _level;
    }
    std::string LogEvent::FileName()
    {
        return _file;
    }
    uint32_t LogEvent::GetLine()
    {
        return _line;
    }
    uint64_t LogEvent::GetElapse()
    {
        return _elapse;
    }
    uint32_t LogEvent::GetThreaId()
    {
        return _thread_id;
    }
    uint64_t LogEvent::GetTime()
    {
        return _time;
    }
    std::string LogEvent::GetContent() // 获取内容
    {
        return _ss.str();
    }

    uint32_t LogEvent::GetFiberId()
    {
        return _fiber_id;
    }
    std::string LogEvent::GetThreadName()
    {
        return _thread_name;
    }
    std::stringstream &LogEvent::GetSStream() // 获取内容的流--用来保存日志内容
    {
        return _ss;
    }
    void LogEvent::format(const char *fmt, ...) // 格式化输入日志内容   format("log is %s",aaaaa);
    {
        va_list args;
        va_start(args, fmt);
        // 第一次调用 vsnprintf 计算需要的缓冲区大小
        int length = vsnprintf(NULL, 0, fmt, args);
        va_end(args);
        if (length < 0)
        {
            return;
        }
        // 分配足够的内存
        char *buffer = (char *)malloc(length + 1);
        if (!buffer)
        {
            return;
        }
        // 第二次调用 vsnprintf 填充缓冲区
        va_start(args, fmt);
        vsnprintf(buffer, length + 1, fmt, args);
        va_end(args);
        // 输出日志格式化处理后信息到ss
        _ss << std::string(buffer);
        // 释放分配的内存
        free(buffer);
    }
    Logger::Logger(const std::string &name)
        : _name(name), _level_limit(LogLevel::DEBUG), _root_logger(nullptr)
    {
        // 初始化一个logger时分配一个默认格式Formatter
        _formatter = std::make_shared<Formatter>();
    }
    void Logger::log(LogLevel::Level level, LogEvent::ptr ev)
    {
        auto self = shared_from_this();
        if (level >= _level_limit)
        { // 到达指定日志级别则输出
            if (!_sinkers.empty())
            { // 这个logger有落地对象
                for (auto &sink : _sinkers)
                {
                    sink.second->log(self, level, ev); // 每个sinks又有自己的日志级别
                }
            }
            // 无落地对象 让主logger进行输出
            else
            {
                if (_root_logger)
                {
                    _root_logger->log(level, ev);
                }
            }
        }
    }
    void Logger::SetLevelLimit(LogLevel::Level lv)
    {
        _level_limit = lv;
    }
    LogLevel::Level Logger::GetLevelLimit()
    {
        return _level_limit;
    }
    void Logger::AddSinkers(const std::string &sink_name, Logsinker::ptr sinker)
    {
        _sinkers.insert(std::make_pair(sink_name, sinker));
    }
    void Logger::DelSinkers(const std::string &sink_name)
    {
        for (auto iter = _sinkers.begin(); iter != _sinkers.end(); iter++)
        {
            if (iter->first == sink_name)
            { // 找到了目标sink
                _sinkers.erase(iter);
            }
        }
    }
    void Logger::ClearSinkers()
    {
        _sinkers.clear();
    }
    std::string Logger::GetName()
    {
        return _name;
    }
    void Logger::info(LogEvent::ptr ev)
    {
        log(LogLevel::INFO, ev);
    }
    void Logger::debug(LogEvent::ptr ev)
    {
        log(LogLevel::DEBUG, ev);
    }
    void Logger::warn(LogEvent::ptr ev)
    {
        log(LogLevel::WARN, ev);
    }
    void Logger::error(LogEvent::ptr ev)
    {
        log(LogLevel::ERROR, ev);
    }
    void Logger::fatal(LogEvent::ptr ev)
    {
        log(LogLevel::FATAL, ev);
    }
    Formatter::ptr Logger::GetFormatter()
    {
        return _formatter;
    }
    void Logger::SetFormatter(Formatter::ptr formatter) // 直接传入格式化器
    {
        for (auto &sink : _sinkers)
        {
            if (!sink.second->has_formatter())
            { // sink没有格式化器
                sink.second->SetFormatter(formatter);
            }
        }
        _formatter = formatter;
    }
    void Logger::SetFormatter(const char *fmt_str) // 传入格式化字符串 %d{xxx} %s %g ...
    {
        _formatter = std::make_shared<Formatter>(fmt_str);
        for (auto &sink : _sinkers)
        {
            if (!sink.second->has_formatter())
            { // sink没有格式化器
                sink.second->SetFormatter(_formatter);
            }
        }
    }
    void Logger::SetRootLogger(Logger::ptr root_logger) // 设置主logger
    {
        _root_logger = root_logger;
    }
    // 标准输出
    void StdoutLogsinker::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev)
    {
        if (level >= _level_limit)
        { // 允许输出
            if (!_b_has_formatter)
            {
                // 还没初始化formatter
                SetFormatter(logger->GetFormatter()); // 用所属的logger的formatter给sink的formatter赋值
            }
            // 让Formatter格式化器来进行格式化event形成字符串直接格式化到这个sink对应的流中 --标准输出流
            _formatter->format(std::cout, logger, level, ev);
        }
    }
    FileLogsinker::FileLogsinker(const std::string &filename)
        : _log_filename(filename)
    {
        _last_opentime = TimeUitl::NowTime_to_uint64();
        reopen(); // 打开文件流
    }
    // 文件输出
    void FileLogsinker::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev)
    {
        if (level >= _level_limit)
        { // 允许输出
            if (!_b_has_formatter)
            {
                // 还没初始化formatter
                SetFormatter(logger->GetFormatter()); // 用所属的logger的formatter给sink的formatter赋值
            }
            // 让Formatter格式化器来进行格式化event形成字符串直接格式化到这个sink对应的流中 --文件流
            _formatter->format(_file_stream, logger, level, ev);
        }
    }
    bool FileLogsinker::reopen()
    {
        if (_file_stream)
        {
            _file_stream.close();
        }
        return FileUtil::OpenForWrite(_file_stream, _log_filename, std::ios_base::out | std::ios_base::app); // 以追加方式写入文件
    }
    // 格式化items
    // 1.日志内容输出
    class MessageFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<MessageFormatterItem> ptr;
        MessageFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetContent();
        }

    private:
    };
    // 2.日志级别输出
    class LevelFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<LevelFormatterItem> ptr;
        LevelFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << LogLevel::ToString(lv);
        }

    private:
    };
    // 3.离程序启动时间输出
    class ElapseFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<ElapseFormatterItem> ptr;
        ElapseFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << std::to_string(ev->GetElapse());
        }

    private:
    };
    // 4.日志器名字输出
    class LoggerNameFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<LoggerNameFormatterItem> ptr;
        LoggerNameFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetLogger()->GetName();
        }

    private:
    };
    // 5.线程id输出
    class ThreadIdFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<ThreadIdFormatterItem> ptr;
        ThreadIdFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetThreaId();
        }

    private:
    };
    // 6.协程id输出
    class FiberIdFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<FiberIdFormatterItem> ptr;
        FiberIdFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetFiberId();
        }

    private:
    };
    // 7.线程名字输出
    class ThreadNameFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<ThreadNameFormatterItem> ptr;
        ThreadNameFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetThreadName();
        }

    private:
    };
    // 8.时间日期输出
    class DateTimeFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<DateTimeFormatterItem> ptr;
        DateTimeFormatterItem(const std::string &str = "") : _date_time_pattern(str) {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            // 对日期时间格式做处理
            struct tm tm;
            time_t time = ev->GetTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), _date_time_pattern.c_str(), &tm);
            osm << buf; // buf是格式化处理后的时间日期
        }

    private:
        std::string _date_time_pattern; // 日期时间输出的格式
    };
    // 9.file名字输出
    class FileNameFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<FileNameFormatterItem> ptr;
        FileNameFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->FileName();
        }

    private:
    };
    // 10.行号输出
    class LineFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<LineFormatterItem> ptr;
        LineFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << ev->GetLine();
        }

    private:
    };
    // 11.换行输出
    class NewLineFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<NewLineFormatterItem> ptr;
        NewLineFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << std::endl;
        }

    private:
    };
    // 12.普通字符输出
    class StringFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<StringFormatterItem> ptr;
        StringFormatterItem(const std::string &str = "") : _string(str) {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << _string;
        }

    private:
        std::string _string;
    };
    // 13.Table输出
    class TabFormatterItem : public Formatter::FormatterItem // 基类
    {
    public:
        typedef std::shared_ptr<TabFormatterItem> ptr;
        TabFormatterItem(const std::string &str = "") {}
        virtual void format(std::ostream &osm, Logger::ptr logger, LogLevel::Level lv, LogEvent::ptr ev)
        {
            osm << "\t";
        }

    private:
    };
    Formatter::Formatter(const std::string &fmt_str)
        : _formatter_str(fmt_str), _b_error(false)
    {
    }
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
    // 默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    void Formatter::init() // 初始化模板 根据fmt_str填入items
    {
        // str, format, type --普通字符是0  格式字符是1
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr; // 存放普通字符
        for (size_t i = 0; i < _formatter_str.size(); ++i)
        { // 遍历字符串
            if (_formatter_str[i] != '%')
            {
                // 不是%
                nstr.append(1, _formatter_str[i]);
                continue;
            }
            // 是%

            if ((i + 1) < _formatter_str.size())
            {
                if (_formatter_str[i + 1] == '%')
                { // 是%%
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while (n < _formatter_str.size())
            {
                if (!fmt_status && (!isalpha(_formatter_str[n]) && _formatter_str[n] != '{' && _formatter_str[n] != '}'))
                {
                    str = _formatter_str.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (_formatter_str[n] == '{')
                    {
                        str = _formatter_str.substr(i + 1, n - i - 1);
                        // std::cout << "*" << str << std::endl;
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (_formatter_str[n] == '}')
                    {
                        fmt = _formatter_str.substr(fmt_begin + 1, n - fmt_begin - 1);
                        // std::cout << "#" << fmt << std::endl;
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == _formatter_str.size())
                {
                    if (str.empty())
                    {
                        str = _formatter_str.substr(i + 1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1)); // str表示d s这种格式化项 fmt表示{}中的内容 1表示是格式化项
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << _formatter_str << " - " << _formatter_str.substr(i) << std::endl;
                _b_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }
        // 格式化解析完毕
        //  std::vector<std::tuple<std::string, std::string, int> > vec;
        //  std::string nstr; //存放普通字符
        // 根据解析结果的vec和nstr生成items并填入
        if (!nstr.empty())
        {
            // 还有普通字符
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
        // 遍历所有vec中的tuple生成items
        // 存放str和生成对应item的映射关系
        static std::unordered_map<std::string, std::function<FormatterItem::ptr(const std::string &)>> item_map = {
#define XX(str, item)                                                          \
    {                                                                          \
        #str,                                                                  \
            [](const std::string &fmt) { return std::make_shared<item>(fmt); } \
    }
            XX(m, MessageFormatterItem),
            XX(p, LevelFormatterItem),
            XX(r, ElapseFormatterItem),
            XX(c, LoggerNameFormatterItem),
            XX(t, ThreadIdFormatterItem),
            XX(n, NewLineFormatterItem),
            XX(d, DateTimeFormatterItem),
            XX(f, FileNameFormatterItem),
            XX(l, LineFormatterItem),
            XX(T, TabFormatterItem),
            XX(F, FiberIdFormatterItem),
            XX(N, ThreadNameFormatterItem)
#undef XX
        };
        for (auto &ele : vec)
        {
            if (std::get<2>(ele) == 0) // 说明是普通字符
            {
                _items.push_back(std::make_shared<StringFormatterItem>(std::get<0>(ele)));
            }
            else
            { // 不是普通字符
                auto iter = item_map.find(std::get<0>(ele));
                if (iter != item_map.end())
                {
                    // 找到了这个对应的item构造
                    _items.push_back(item_map[std::get<0>(ele)](std::get<1>(ele)));
                }
                else
                {
                    // 没找到
                    _items.push_back(std::make_shared<StringFormatterItem>("<<error_format %" + std::get<0>(ele) + " >>"));
                    _b_error = true;
                }
            }
        }
    }
    void Formatter::format(std::ostream &out_stream, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr ev) // 向流中输出
    {
        for (auto &ele : _items)
        {
            ele->format(out_stream, logger, level, ev);
        }
    }
    std::string Formatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr ev) // 直接输出字符串
    {
        std::stringstream strsm;
        for (auto &ele : _items)
        {
            ele->format(strsm, logger, level, ev);
        }
        return strsm.str();
    }
    std::string Formatter::getFormatterPattern()
    {
        return _formatter_str;
    }
    bool Formatter::isError()
    {
        return _b_error;
    }

}