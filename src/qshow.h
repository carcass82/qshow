/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#pragma once
#include <cassert>
#include <string>
#include <list>
#include <algorithm>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>

class QShow
{
public:
	QShow(const std::string&);
    ~QShow();
	int Show();

private:
	enum ZoomParam { ZOOMIN, ZOOMOUT };
	enum RotParm { ROT_CW, ROT_CCW };
	enum ScrollDirection { SC_RIGHT, SC_LEFT, SC_UP, SC_DOWN, SC_CHECK };
	enum BrowseImg { IM_NEXT, IM_PREV };

	void InitSDL();
	void Shutdown();
    void LoadImage(const fs::path& image_file);
	bool ChangeImage(BrowseImg);
	void Reshape();
	void SetVideoMode();
	void ZoomImage(ZoomParam, int x = -1, int y = -1);
	void RotateImage(RotParm r) { rotDegrees += (r == ROT_CW)? 90.0 : -90.0; }
	bool Scroll(ScrollDirection);
	void CenterImage();
	void Draw();
	void DrawCheckerPattern();
    void SetTitle(const std::string& filename);

	bool quit;
	bool fullscreen;
	bool render;
	bool isScrolling;
	bool showCheckerBoard;
	ScrollDirection scrollDir;
	int bestWidth;
	int bestHeight;
	int width;
	int height;
	float zoomFactor;
	float rotDegrees;
	bool scrollEnable[4];

    //std::string fileName;
    std::list<fs::path> filelist;
    std::list<fs::path>::iterator curFile;

	SDL_Event event;
	SDL_Surface *screen;
	SDL_Surface *img;
	SDL_Surface *imgModified;
	SDL_Rect imgPosition;
};
