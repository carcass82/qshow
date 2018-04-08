/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#include <iostream>
#include "qshow.h"

int main(int argc, char **argv)
{
    QShow mainw;
    if (mainw.Init((argc > 1)? argv[1] : "."))
    {
        mainw.Run();
    }
    else
    {
        std::cerr << "unable to start qshow" << std::endl;
        return -1;
    }

    return 0;
}
