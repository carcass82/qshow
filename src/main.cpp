/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#include <iostream>
#include "qshow.h"

void usage()
{
	std::cerr << "usage: qshow <image file>" << std::endl;
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		usage();
		return -1;
	}

	QShow mainw(argv[1]);
	return mainw.Show();
}
