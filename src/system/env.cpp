#include "env.h"
#include <unistd.h>
#include <iomanip>
#include <stdio.h>
namespace Xten
{
    // 解析命令行参数 ./program -key1 val -key2 -key3 ....
    bool Env::Init(int argc, char **argv)
    {
        char link[1024] = {0};
        char path[1024] = {0};
        sprintf(link, "/proc/%d/exe", getpid());
        readlink(link, path, 1024);
        _exe = path;
        int pos = _exe.find_last_of('/');
        _cwd = _exe.substr(0, pos) + '/';
        _program = argv[0];
        char *key = nullptr;
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                // 是一个key
                if (strlen(argv[i]) > 1)
                {
                    if (key)
                    {
                        Add(key, "");
                    }
                    key = argv[i] + 1;
                }
                else
                {
                    // 说明只有一个 -
                    return false;
                }
            }
            else
            {
                // 是一个val
                if (key == nullptr)
                {
                    return false;
                }
                Add(key, argv[i]);
                key = nullptr;
            }
        }
        // 解析完看最后是不是剩余一个key
        if (key)
        {
            Add(key, "");
        }
        return true;
    }
    // 添加命令行参数
    void Env::Add(const std::string &key, const std::string &val)
    {
        RWMutex::WriteLock wlock(_mtx);
        _cmdLineArgs[key] = val;
    }
    // 获取命令行参数(有默认值)
    std::string Env::Get(const std::string &key, const std::string &val)
    {
        RWMutex::ReadLock rlock(_mtx);
        auto iter = _cmdLineArgs.find(key);
        return iter == _cmdLineArgs.end() ? val : iter->second;
    }
    // 删除命令行参数
    void Env::Del(const std::string &key)
    {
        RWMutex::WriteLock wlock(_mtx);
        _cmdLineArgs.erase(key);
    }
    bool Env::Has(const std::string &key)
    {
        RWMutex::ReadLock rlock(_mtx);
        auto iter = _cmdLineArgs.find(key);
        return iter != _cmdLineArgs.end();
    }
    // 添加help
    void Env::AddHelp(const std::string &key, const std::string &help)
    {
        DelHelp(key);
        RWMutex::WriteLock wlock(_mtx);
        _helps.push_back(std::make_pair(key, help));
    }
    // 获取所有help
    std::string Env::PrintHelps()
    {
        RWMutex::ReadLock rlock(_mtx);
        std::stringstream ss;
        ss << "Usage: " << _program << " [options]" << std::endl;
        for (auto &help : _helps)
        {
            ss << std::setw(5) << "-" << help.first << " : " << help.second << std::endl;
        }
        return ss.str();
    }
    // 删除某个help
    void Env::DelHelp(const std::string &key)
    {
        RWMutex::WriteLock wlock(_mtx);
        auto iter = _helps.begin();
        for (iter; iter != _helps.end();)
        {
            if (iter->first == key)
            {
                // 找到了help
                iter = _helps.erase(iter);
            }
            else
            {
                // 当前不是
                iter++;
            }
        }
    }
    // 获取某个相对路径的绝对路径
    std::string Env::GetAbsolutePath(const std::string &path)
    {
        if(path.empty())
        {
            return _cwd;
        }
        if(path.front()=='/'){
            //本身就是绝对路径
            return path;
        }
        //相对路径
        return _cwd+path;
    }
    // 获取配置文件的绝对路径
    std::string Env::GetConfigPath()
    {
        return GetAbsolutePath(Get("c","./config"));
    }
}