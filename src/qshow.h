/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#pragma once

#include <cassert>
#include <string>
#include <list>
#include <algorithm>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "SDL2/SDL.h"
#include "FreeImage.h"

class QShow
{
public:
    QShow(const std::string&);
    ~QShow();
    void Show();

private:
    enum BrowseImg { IMG_NEXT, IMG_PREV };

    void InitSDL();
    void LoadImage(const fs::path& image_file);
    bool ChangeImage(BrowseImg direction);
    void SetVideoMode();
    void Render();
    void SetTitle(const std::string& filename);
    void OnImageChanged();
    void OnSizeChanged(int32_t w, int32_t h);

    bool quit_ = false;
    bool fullscreen_ = false;
    int window_width_ = 0;
    int window_height_ = 0;
    int width_ = 0;
    int height_ = 0;
    int bpp_ = 0;
    float image_zoom_ = 1.0f;
    float image_rot_deg_ = 0.0f;
    float image_fit_factor_= 0.0f;

    std::list<fs::path> filelist_;
    std::list<fs::path>::iterator current_file_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    SDL_Event event;
    FIBITMAP* original_image_ = nullptr;
    SDL_Surface* image_ = nullptr;
    SDL_Rect image_position_;
};
