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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class QShow
{
public:
    QShow(const std::string&);
    ~QShow();
    void Show();

private:
    enum ZoomParam { ZOOMIN, ZOOMOUT };
    enum RotParm { ROT_CW, ROT_CCW };
    enum ScrollDirection { SC_RIGHT, SC_LEFT, SC_UP, SC_DOWN, SC_CHECK };
    enum BrowseImg { IM_NEXT, IM_PREV };

    void InitSDL();
    void LoadImage(const fs::path& image_file);
    bool ChangeImage(BrowseImg direction);
    void Reshape();
    void SetVideoMode();
    void ZoomImage(ZoomParam, int x = -1, int y = -1);
    bool Scroll(ScrollDirection s);
    void CenterImage();
    void Render();
    void DrawCheckerPattern();
    void SetTitle(const std::string& filename);
    void OnImageChanged();
    void OnSizeChanged(int32_t w, int32_t h);

    bool quit = false;
    bool fullscreen_ = false;
    bool render = false;
    bool isScrolling = false;
    bool showCheckerBoard = false;
    int window_width_ = 0;
    int window_height_ = 0;
    int width_ = 0;
    int height_ = 0;
    int bpp_ = 0;
    float image_zoom_ = 1.0f;
    float image_rot_deg_ = 0.0f;
    float image_fit_factor_= 0.0f;
    bool scrollEnable[SC_CHECK] = { false, false, false, false };

    std::list<fs::path> filelist_;
    std::list<fs::path>::iterator current_file_;

    ScrollDirection scrollDir;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    SDL_Event event;
    SDL_Surface* original_image_ = nullptr;
    SDL_Surface* image_ = nullptr;
    SDL_Rect image_position_;
};
