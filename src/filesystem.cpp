/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#include "filesystem.h"
using namespace std;

FileSystem::FileSystem(const string& filePath)
{
	SetFileName(filePath);
}

void FileSystem::SetFileName(const string& filePath)
{
	fileName = filePath;
	dirName = "";

	Init();
}

void FileSystem::Init()
{
	// first thing first check if file exists
	int fd = open(fileName.c_str(), O_RDONLY);
	if (fd == -1) {
		fileName.clear();
		return;
	}
	close(fd);

	// if path is relative, prepend current working dir
	if (fileName.at(0) != '/') {
		char *cwd = getcwd(NULL, 0);
		fileName.insert(0, "/");
		fileName.insert(0, cwd);
		delete cwd;
	}

	// exclude fileName to get directory
	int fnIdx = fileName.find_last_of('/');
	if (fnIdx != string::npos) {
		dirName = fileName.substr(0, fnIdx + 1);
		fileName = fileName.substr(fnIdx + 1);
	}

	// finally resolve path relative parts
	// (ie: /home/me/../you/../we/. ==> /home/we)
	list<string> pathSplitted;
	int idx = 1;                  // path is absolute: dirname(0) MUST BE '/'
	size_t found = string::npos;
	while ((found = dirName.find("/", idx)) != string::npos) {

		string part = dirName.substr(idx, found - idx);
		if (part == ".") {
			idx = found + 1;
			continue;
		} else if (part == "..") {
			pathSplitted.pop_back();
			idx = found + 1;
			continue;
		} else {
			pathSplitted.push_back(part);
			idx = found + 1;
		}
	}

	stringstream ss;
	list<string>::iterator it;
	for (it = pathSplitted.begin(); it != pathSplitted.end(); ++it)
		ss << "/" << *it;
	dirName = ss.str();
}

list<string> FileSystem::GetFileList(list<string> filter)
{
	list<string> dirContent;

	dir = opendir(dirName.c_str());
	if (dir != NULL) {
		while ((entry = readdir(dir)) != NULL) {
			string fEntry(entry->d_name);
			if (fEntry != "." && fEntry != "..") {
				for (list<string>::const_iterator cit = filter.begin(); cit != filter.end(); ++cit) {
					if (fEntry.size() >= (*cit).size() && equal((*cit).rbegin(), (*cit).rend(), fEntry.rbegin(), CaseInsensitiveComparer())) {
						dirContent.push_back(dirName + "/" + fEntry);
						break;
					}
				}
			}
		}
	}

	return dirContent;
}
