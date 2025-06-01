#pragma once
//不可拷贝对象封装
namespace Xten
{   
    class NoCopyable
    {
    public:
        NoCopyable()=default;
        /// @brief 
        // 作为基类使用 基类拷贝构造函数和赋值函数都删除 子类无法拷贝构造和赋值构造
        /// @param  
        NoCopyable(const NoCopyable&)=delete;  
        NoCopyable& operator=(const NoCopyable&)=delete;
        ~NoCopyable()=default;
    };
} 