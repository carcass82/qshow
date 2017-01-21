/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <cassert>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <list>

class FileSystem
{
public:
	FileSystem(const std::string&);
	virtual ~FileSystem() { }
	std::string GetFilePath() { return dirName; }
	std::string GetFileName() { return fileName; }
	std::string GetAbsoluteFileName() { return dirName + "/" + fileName; }
	std::list<std::string> GetFileList(std::list<std::string> filter);
	void SetFileName(const std::string&);

private:
	void Init();

	std::string fileName;
	std::string dirName;
	DIR *dir;
	dirent *entry;
};

struct CaseInsensitiveComparer
{
	bool operator()(const char& a, const char& b) const { return ::tolower(a) == ::tolower(b); }
};

#endif /* FILESYSTEM_H_ */
