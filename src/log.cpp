#include "../include/log.h"
#include "../include/config.h"
// 包含头文件后，编译器能“看到”模板声明和特化之间的关系
// 这意味着：
// 编译器在处理 log.cpp时，会先看到 LexicalCast 模板的原始声明。
// 然后再看到你在 log.cpp 中提供的两个特化版本。
// 因此，这两个特化版本会被视为对原始模板的合法特化。
// 如果 log.cpp 没有包含 config.h，
// 那么你写的特化将被视为一个新的类模板定义，而不是对已有模板的特化
// ，这会导致链接错误或调用不到预期的特化版本。
namespace Xten
{
    const char *LogLevel::ToString(const LogLevel::Level &level) // level转字符串
    {
        switch (level)
        { // 定义宏函数来简化代码 #是用来转成字符串
#define XX(LEV)                \
                               \
    case LogLevel::Level::LEV: \
        return #LEV;           \
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
        // std::cout<<_sinkers.size()<<std::endl;
        if (level >= _level_limit)
        { // 到达指定日志级别则输出
            if (!_sinkers.empty())
            { // 这个logger有落地对象
                for (auto &sink : _sinkers)
                {
                    sink.second->log(self, level, ev); // 每个sinks又有自己的日志级别
                    // std::cout<<sink.second->toYamlString();
                    // std::cout<<"logger 的log调用"<<std::endl;
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
        // 1.添加一个sinker需要判断这个sinker是否有格式化器 没有的话用logger的格式化器
        // 保证每一个sink都有格式化器
        if (!sinker->has_formatter())
        {
            sinker->SetFormatter(_formatter);
        }
        //如果 unordered_map 中的 key 已经存在，调用 insert 操作时将不会插入新的键值对
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
            // 覆盖sink本身的格式化器--即使已经有了格式化器
            sink.second->SetFormatter(formatter);
        }
        _formatter = formatter;
    }
    void Logger::SetFormatter(const char *fmt_str) // 传入格式化字符串 %d{xxx} %s %g ...
    {
        _formatter = std::make_shared<Formatter>(fmt_str);
        for (auto &sink : _sinkers)
        {
            // 覆盖sink本身的格式化器--即使已经有了格式化器
            sink.second->SetFormatter(_formatter);
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
            // std::cout<<"stdout log调用"<<std::endl;
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
            bool is_open = true;
            // 让Formatter格式化器来进行格式化event形成字符串直接格式化到这个sink对应的流中 --文件流
            if (TimeUitl::NowTime_to_uint64() - _last_opentime > 3)
            {
                // 上次打开文件的时间比较长 重新打开文件防止文件被删除
                is_open = reopen();
                _last_opentime = TimeUitl::NowTime_to_uint64(); // 更新时间戳
            }
            if (is_open)
            {
                _formatter->format(_file_stream, logger, level, ev);
            }
            else
            {
                std::cout << "log file open failed" << std::endl;
            }
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
        init();
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
    LoggerManager::LoggerManager()
    { // 构造函数默认生成一个主logger和一个system logger
        auto _root_logger = std::make_shared<Logger>();
        auto system_logger = std::make_shared<Logger>("system");
        _root_logger->AddSinkers("stdout", std::make_shared<StdoutLogsinker>());
        system_logger->AddSinkers("stdout", std::make_shared<StdoutLogsinker>());
        // std::cout<<"root 日志添加sinl=k" <<std::endl;
        _loggers_map.insert(std::make_pair(_root_logger->GetName(), _root_logger));
        _loggers_map.insert(std::make_pair(system_logger->GetName(), system_logger));
        init();
    }
    Logger::ptr LoggerManager::GetLogger(const std::string &name)
    {
        auto iter = _loggers_map.find(name);
        if (iter == _loggers_map.end())
        {
            // 没找到
            return Logger::ptr();
        }
        return iter->second;
    }
    bool LoggerManager::SetLogger(const std::string &name, Logger::ptr logger)
    { // 如果有重复会进行覆盖
        _loggers_map[name] = logger;
        return true;
    }
    Logger::ptr LoggerManager::GetRootLogger() // 获取root的logger
    {
        return _loggers_map["root"];
    }
    void LoggerManager::init()
    {
    }
    void LoggerManager::ClearLogger() // 清除所有logger
    {
        _loggers_map.clear();
    }
    void LoggerManager::DelLogger(const std::string &name) // 删除指定logger
    {
        _loggers_map.erase(name);
    }
    std::string Logger::toYamlString() // 将配置转化成yaml格式的string
    {
        YAML::Node node;
        node["name"] = _name;
        if (_level_limit != LogLevel::UNKNOW)
        {
            node["level"] = LogLevel::ToString(_level_limit);
        }
        if (_formatter)
        {
            node["formatter"] = _formatter->getFormatterPattern();
        }

        for (auto &i : _sinkers)
        {
            // std::cout<<"toYamlString"<<_sinkers.size()<<std::endl;
            // std::cout<<"toYamlString"<<i.second->toYamlString()<<std::endl;
            node["sinkers"].push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    std::string StdoutLogsinker::toYamlString() // 将配置转化成yaml格式的string
    {
        YAML::Node node;
        node["type"] = "StdoutLogSinker";
        if (_level_limit != LogLevel::UNKNOW)
        {
            node["level"] = LogLevel::ToString(_level_limit);
        }
        if (_b_has_formatter && _formatter)
        {
            node["formatter"] = _formatter->getFormatterPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    std::string FileLogsinker::toYamlString() // 将配置转化成yaml格式的string
    {
        YAML::Node node;
        node["type"] = "FileLogSinker";
        node["file"] = _log_filename;
        if (_level_limit != LogLevel::UNKNOW)
        {
            node["level"] = LogLevel::ToString(_level_limit);
        }
        if (_b_has_formatter && _formatter)
        {
            node["formatter"] = _formatter->getFormatterPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    std::string LoggerManager::toYamlString() // 将配置转化成yaml格式的string
    {
        YAML::Node node;
        for (auto &i : _loggers_map)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    // 定义log配置的特有的T用于配置日志模块的var
    struct LogSinkerDefine
    {
        std::string _name;                               // 用于再logger的sinks中进行使用
        LogLevel::Level _level_limit = LogLevel::UNKNOW; // limit日志级别
        std::string _formatter;                          // 日志格式化器字符串---sink级别的
        int _type = SinkType::STDOUT;                    // 日志器的类型
        std::string _file;                               // 文件的话 文件名
        bool operator==(const LogSinkerDefine &old) const
        {
            return _name == old._name &&
                   _level_limit == old._level_limit &&
                   _formatter == old._formatter &&
                   _type == old._type &&
                   _file == old._file;
        }
    };
    struct LoggerDefine
    {
        std::string _name;                               // 日志名称
        LogLevel::Level _level_limit = LogLevel::UNKNOW; // limit日志级别
        std::vector<LogSinkerDefine> _sinkers;           // 所有的日志输出器
        std::string _formatter;                          // 日志格式化器字符串---logger级别的
        bool operator==(const LoggerDefine &old) const
        {
            return _name == old._name &&
                   _level_limit == old._level_limit &&
                   _sinkers == old._sinkers &&
                   _formatter == old._formatter;
        }
        // 这个小于的重载用于在set中进行find操作时的<的规则 使find时是按照name进行查找 name一致则是同一个日志器
        bool operator<(const LoggerDefine &old) const
        {
            return _name < old._name;
        }
    };

    // 这些特化版本 只在 log.cpp 文件中可见，因为它们没有在头文件（如 log.h）中声明。
    // 因此，只有在 log.cpp 编译时，编译器才知道这些特化版本的存在，并用于相关的类型转换。
    // 其他源文件（例如 main.cpp 或 config_test.cpp）如果尝试对 LogDefine 做类似的LexicalCast 操作，
    // 会因找不到特化版本而调用原始模板（如果有默认实现），或者直接报错（如果没有合适的匹配）。
    // 特化类型转换的仿函数---LoggerDefine和LogSinkerDefine 全特化
    template <>
    class lexicalCast<std::string, LoggerDefine>
    {
        public:
        // logs:
        //  - name: root
        //    level: info
        //    appenders:
        //        - type: FileLogAppender
        //          name: fileout
        //          file: /apps/logs/sylar/root.txt
        //        - type: StdoutLogAppender
        //          name: stdout
        //  - name: system
        //    level: info
        //    appenders:
        //        - type: FileLogAppender
        //          file: /apps/logs/sylar/system.txt
        //        - type: StdoutLogAppender
        LoggerDefine operator()(const std::string &v)
        {
            YAML::Node n = YAML::Load(v);
            LoggerDefine ld; // 要返回的日志配置类型
            if (!n["name"].IsDefined())
            { // logger的name
                std::cout << "log config error: name is null, " << n
                          << std::endl;
                throw std::logic_error("log config name is null");
            }
            ld._name = n["name"].as<std::string>();
            // logger的日志级别
            ld._level_limit = LogLevel::ToLevel(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if (n["formatter"].IsDefined())
            { // logger的格式化器
                ld._formatter = n["formatter"].as<std::string>();
            }
            // logger的落地类
            if (n["sinkers"].IsDefined())
            {
                // std::cout << "==" << ld.name << " = " << n["appenders"].size() << std::endl;
                for (size_t x = 0; x < n["sinkers"].size(); ++x)
                {
                    auto a = n["sinkers"][x];
                    if (!a["type"].IsDefined())
                    {
                        std::cout << "log config error: appender type is null, " << a
                                  << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogSinkerDefine lsd; // 日志落地类的配置类型
                    if (type == "FileLogAppender")
                    {
                        lsd._type = SinkType::FILE;
                        if (!a["file"].IsDefined())
                        {
                            std::cout << "log config error: fileappender file is null, " << a
                                      << std::endl;
                            continue;
                        }
                        lsd._file = a["file"].as<std::string>();
                        if (a["formatter"].IsDefined())
                        {
                            lsd._formatter = a["formatter"].as<std::string>();
                        }
                        if (a["name"].IsDefined())
                        {
                            lsd._name = a["name"].as<std::string>();
                        }
                        if (a["level"].IsDefined())
                        {
                            lsd._level_limit = LogLevel::ToLevel(a["level"].as<std::string>());
                        }
                    }
                    else if (type == "StdoutLogAppender")
                    {
                        lsd._type = SinkType::STDOUT;
                        if (a["formatter"].IsDefined())
                        {
                            lsd._formatter = a["formatter"].as<std::string>();
                        }
                        if (a["name"].IsDefined())
                        {
                            lsd._name = a["name"].as<std::string>();
                        }
                        if (a["level"].IsDefined())
                        {
                            lsd._level_limit = LogLevel::ToLevel(a["level"].as<std::string>());
                        }
                    }
                    else
                    {
                        std::cout << "log config error: appender type is invalid, " << a
                                  << std::endl;
                        continue;
                    }

                    ld._sinkers.push_back(lsd);
                }
            }
            return ld;
        }
    };
    template <>
    class lexicalCast<LoggerDefine, std::string>
    {
        public:
        std::string operator()(const LoggerDefine &i)
        {
            YAML::Node n;
            n["name"] = i._name;
            if (i._level_limit != LogLevel::UNKNOW)
            {
                n["level"] = LogLevel::ToString(i._level_limit);
            }
            if (!i._formatter.empty())
            {
                n["formatter"] = i._formatter;
            }
            // 转每一个sinker
            for (auto &a : i._sinkers)
            {
                YAML::Node na;
                if (a._type == SinkType::FILE)
                {
                    na["type"] = "FileLogSinker";
                    na["file"] = a._file;
                }
                else if (a._type == SinkType::STDOUT)
                {
                    na["type"] = "StdoutLogSinker";
                }
                if (a._level_limit != LogLevel::UNKNOW)
                {
                    na["level"] = LogLevel::ToString(a._level_limit);
                }

                if (!a._formatter.empty())
                {
                    na["formatter"] = a._formatter;
                }
                if (!a._name.empty())
                {
                    na["name"] = a._name;
                }
                n["sinkers"].push_back(na);
            }
            std::stringstream ss;
            ss << n;
            return ss.str();
        }
    };
    //这个变量的存在用来保证config文件中的类静态成员变量要先于下面此文件全局变量访问时创建
    //同一个编译单元（translation unit）内的全局变量初始化顺序是 按照它们在代码中的出现顺序进行的。
    //也就是说，在一个 .cpp 文件中定义的所有全局变量和静态局部变量，
    //其初始化顺序与它们在源文件中声明的位置一致。
    struct ConfigInitializer {
        ConfigInitializer() {
            // 强制访问静态成员变量，触发其构造
            (void)Config::_configvars_map;
            (void)Config::_configfile_modifytimes;
        }
    };
    static ConfigInitializer s_configInit; // 静态变量，构造早于 main
    // 创建一个日志的Configvar到config模块中
    // 这是动态库的一个全局变量
    // 在程序启动后 main函数执行前 动态连接器加载动态库到可执行程序时 会对库中的全局变量和静态变量进行初始化工作
    // 也就是说这个变量在main之前已经定义好了 并调用了LookUp内部也会对静态局部变量 ConfigVarMap 进行创建
    // 其他文件没有使用extern ConfigVar<LoggerDefine>::ptr g_logs_defines 因此这个全局变量只在这个此文件可见
    // 日志配置模块的创建地方
    //日志模块的配置数据类型是 std::set<LoggerDefine>
    Xten::ConfigVar<std::set<LoggerDefine>>::ptr g_logs_defines = Xten::Config::LookUp("logs", std::set<LoggerDefine>(), "logs config");
    struct LogIniter
    {
        // 在构造函数中进行日志配置变更函数的创建
        LogIniter()
        {
            // 每次重新读取配置文件都会触发setval 而setval判断值改变会进行触发变更回调函数的调用来更改配置实体
            g_logs_defines->AddListener([](const std::set<LoggerDefine> &old_val, const std::set<LoggerDefine> &new_val)
                                        {
                
                XTEN_LOG_INFO(XTEN_LOG_ROOT())<<"on_logs_config_changed";
                //先遍历新的值
                for(auto& logdef:new_val)
                {
                    // std::cout<<logdef._sinkers.size()<<std::endl;
                    //查找的依据是日志配置结构中的name
                    auto iter=old_val.find(logdef);
                    if(iter==old_val.end())
                    { //说明没找到这个日志器 是新增加的日志器
                        //最开始是logDefine是空 但是有日志器 需要进行覆盖
                        //创建日志器并放入管理类
                        Xten::Logger::ptr logger=std::make_shared<Logger>(logdef._name);
                        logger->SetLevelLimit(logdef._level_limit);
                        if(!logdef._formatter.empty()){
                            //有一个默认的formatter 这个会覆盖所有sink的formatter
                            logger->SetFormatter(logdef._formatter.c_str());
                        }
                        for(auto& sink:logdef._sinkers){
                            //一个logger默认是没有sink的
                            //根据加载的配置添加sink
                            Xten::Logsinker::ptr p_sink; //基类sink指针
                            switch(sink._type)
                            {
                            case SinkType::STDOUT:
                                p_sink=std::make_shared<Xten::StdoutLogsinker>();
                                    break;
                            case SinkType::FILE:
                                p_sink=std::make_shared<Xten::FileLogsinker>(sink._file);
                                    break;
                                    //....可扩展
                            default:
                                    break;
                            }
                            p_sink->SetLevelLimit(sink._level_limit);
                            if(!sink._formatter.empty()){
                                p_sink->SetFormatter(std::make_shared<Xten::Formatter>(sink._formatter.c_str())); //覆盖logger的formatter
                            }
                           logger->AddSinkers(sink._name,p_sink);
                        }
                        //将这个新配置日志器放入mgr 有的话也会覆盖
                        Xten::LoggerManager::GetInstance()->SetLogger(logger->GetName(),logger);
                    }else{
                        //找到了 新的这个日志器和老的一样
                        if(logdef==*iter){
                            continue;
                        }
                        //同一个日志器进行了修改
                        Xten::Logger::ptr logger=std::make_shared<Logger>(logdef._name);
                        logger->SetLevelLimit(logdef._level_limit);
                        if(!logdef._formatter.empty()){
                            //有一个默认的formatter 这个会覆盖所有sink的formatter
                            logger->SetFormatter(logdef._formatter.c_str());
                        }
                        for(auto& sink:logdef._sinkers){
                            //一个logger默认是没有sink的
                            //根据加载的配置添加sink
                            Xten::Logsinker::ptr p_sink; //基类sink指针
                            switch(sink._type)
                            {
                            case SinkType::STDOUT:
                                p_sink=std::make_shared<Xten::StdoutLogsinker>();
                                    break;
                            case SinkType::FILE:
                                p_sink=std::make_shared<Xten::FileLogsinker>(sink._file);
                                    break;
                                    //....可扩展
                            default:
                                    break;
                            }
                            p_sink->SetLevelLimit(sink._level_limit);
                            if(!sink._formatter.empty()){
                                p_sink->SetFormatter(std::make_shared<Xten::Formatter>(sink._formatter.c_str())); //覆盖logger的formatter
                            }
                           logger->AddSinkers(sink._name,p_sink);
                        }
                        //将这个新配置日志器放入mgr 有的话也会覆盖
                        Xten::LoggerManager::GetInstance()->SetLogger(logger->GetName(),logger);                       
                    }
                    //遍历旧的值
                    //确保在新的配置logs中删除的日志器不被遗漏
                    for(auto& oldlog:old_val)
                    {
                        auto iter=new_val.find(oldlog);
                        if(iter==new_val.end()){
                            //没找到 说明需要进行删除
                            XTEN_LOG_DEL(iter->_name);
                        }
                        //找到了不做处理 在上面已经进行处理
                    }
                    //更新完对logger进行输出
                } });
        }
    };
    // 这个库的静态局部变量在main函数之前创建 在构造函数中会进行变更回调函数的注册
    static LogIniter __log__init;
    Logger::ptr LoggerManager::GetAndSetLogger(const std::string &name)//获取logger 不存在则设置
    {
        auto iter=_loggers_map.find(name);
        if(iter!=_loggers_map.end())
        {
            return iter->second;
        }
        //没找到设置
        Logger::ptr new_logger=std::make_shared<Logger>(name);
        new_logger->AddSinkers("stdout",std::make_shared<StdoutLogsinker>());
        _loggers_map[name]=new_logger;
        return new_logger;
    }

}