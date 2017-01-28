/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#include <iostream>
#include "qshow.h"

int usage()
{
    std::cerr << "usage: qshow <image file>" << std::endl;
    return -1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {

        return usage();

    } else {

        QShow mainw(argv[1]);
        mainw.Show();

    }
}
