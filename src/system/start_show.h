#ifndef __XTEN_START_SHOW_H__
#define __XTEN_START_SHOW_H__
#include <iostream>
namespace Xten
{
    class StartShow
    {
    public:
        static void showBanner()
        {
            std::cout << R"(
    ██╗  ██╗████████╗███████╗███╗   ██╗   
    ╚██╗██╔╝╚══██╔══╝██╔════╝████╗  ██║             
     ╚███╔╝    ██║   █████╗  ██╔██╗ ██║               Xten Server Framework version 1.0.0
     ██╔██╗    ██║   ██╔══╝  ██║╚██╗██║               Github link: {https://github.com/California-817/Xten}
    ██╔╝ ██╗   ██║   ███████╗██║ ╚████║  
    ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═══╝  )"
                      << std::endl;
            std::cout << std::endl
                      << std::endl;
        }
    };
}
#endif