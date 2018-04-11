/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#pragma once
#include <string>
#include <vector>
#include <algorithm>
using std::vector;

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
using fs::path;

#include "SDL.h"
#include "FreeImage.h"


class QShow
{
public:
    bool Init(const std::string& arg);
    void Run();

private:
    void ScanDirectory(const path& filename);
    bool LoadImage(const path& image_file);
    void LoadTexture();
    bool NextImage();
    bool PrevImage();
    void CreateWindow();
    void SetTitle();
    void OnSizeChanged(int32_t w, int32_t h);
    void Render() const;
    void FilterImage() { /* stub */ }

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
    SDL_Point image_move_ = {};
    char title_string[256];
    bool alt_mousewheel = false;
    bool mouse_move_ = false;
    SDL_Point mouse_move_start_ = {};

    vector<path> filelist_;
    vector<path>::const_iterator current_file_;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    SDL_Event sdl_event_;
    FIBITMAP* original_image_ = nullptr;
};
