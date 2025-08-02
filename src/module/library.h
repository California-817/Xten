#ifndef __XTEN_LIBRARY_H__
#define __XTEN_LIBRARY_H__
#include<string>
#include"module.h"
namespace Xten
{
    class Library
    {
        public:
        static Module::ptr GetModule(const std::string& lib_path);
    };
}

#endif