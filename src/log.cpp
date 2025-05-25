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
        : _name(name), _level_limit(LogLevel::DEBUG), _root_logger(nullptr), _formatter(nullptr)
    {
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
        reopen(); //打开文件流
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
    void Formatter::format(std::ostream &out_stream, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr ev)
    {
    }

}