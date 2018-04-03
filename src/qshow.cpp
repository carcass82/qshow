/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#include "qshow.h"

static inline int clamp(int val, int min, int max) { return std::min(std::max(val, min), max); }

QShow::~QShow()
{
    FreeImage_Unload(original_image_);
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void QShow::ScanDirectory(const fs::path& filename)
{
    auto dir = filename.parent_path();
    if (!dir.empty())
    {
        auto dir_path = fs::directory_iterator(dir);
        for (auto& d_it : dir_path)
        {
            FREE_IMAGE_FORMAT format = FreeImage_GetFileType(d_it.path().generic_string().c_str());
            if (format != FIF_UNKNOWN)
            {
                filelist_.push_back(d_it.path());
                SDL_Log("Added '%s' to supported file list", d_it.path().filename().generic_string().c_str());
            }
        }
    }
}

bool QShow::Init(const std::string& filename)
{
    if (SDL_Init(SDL_INIT_VIDEO) >= 0)
    {
        fs::path selected_file(fs::absolute(filename));
        ScanDirectory(selected_file);

        if (!filelist_.empty())
        {
            current_file_ = std::find(filelist_.begin(), filelist_.end(), selected_file);
            if (LoadImage(*current_file_))
            {
                CreateWindow();
                OnSizeChanged(width_, height_);
                LoadTexture();
                SetTitle();

                return true;
            }
        }
    }

    return false;
}

void QShow::CreateWindow()
{
    SDL_CreateWindowAndRenderer((fullscreen_)? 0 : width_,
                                (fullscreen_)? 0 : height_,
                                (fullscreen_)? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE,
                                &window_,
                                &renderer_);

    SDL_assert(window_);
    SDL_assert(renderer_);

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xff);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
}

bool QShow::LoadImage(const fs::path& image_file)
{
    // reset zoom, movement and rotation
    image_zoom_ = 1.0f;
    image_rot_deg_ = 0.0f;
    image_move_ = { 0, 0 };

    // unload if already loaded
    FreeImage_Unload(original_image_);
    original_image_ = nullptr;

    // load bytes
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(image_file.generic_string().c_str());
    original_image_ = FreeImage_Load(format, image_file.generic_string().c_str());
    FIBITMAP* tmp = original_image_;
    original_image_ = FreeImage_ConvertTo32Bits(original_image_);
    FreeImage_Unload(tmp);

    SDL_assert(original_image_);

    // get useful properties
    width_ = FreeImage_GetWidth(original_image_);
    height_ = FreeImage_GetHeight(original_image_);
    bpp_ = FreeImage_GetBPP(original_image_);

    // update zoom factor
    image_fit_factor_ = std::min((float)window_width_ / width_, (float)window_height_ / height_);

    SDL_Log("Loaded %s (%dx%d), %dbpp", image_file.generic_string().c_str(), width_, height_, bpp_);
    return true;
}

void QShow::LoadTexture()
{
    SDL_DestroyTexture(texture_);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STATIC, width_, height_);
    SDL_UpdateTexture(texture_, nullptr, FreeImage_GetBits(original_image_), width_ * bpp_ / 8);
    SDL_assert(texture_);
}

void QShow::OnSizeChanged(int32_t w, int32_t h)
{
    image_fit_factor_ = std::min(static_cast<float>(w) / width_,
                                 static_cast<float>(h) / height_);
    window_width_ = w;
    window_height_ = h;
    SDL_Log("resize event: %dx%d, new fit factor %f", w, h, image_fit_factor_);
}

void QShow::Render()
{
    SDL_SetWindowFullscreen(window_, (fullscreen_? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    SDL_RenderClear(renderer_);

    SDL_Rect image_zoom = {0, 0, width_, height_};
    if (image_zoom_ != 1.0f)
    {
        float w2 = image_zoom.w / 2.0f;
        float h2 = image_zoom.h / 2.0f;

        // zoom
        image_zoom.x = w2 - (w2 / image_zoom_);
        image_zoom.y = (h2 - (h2 / image_zoom_));
        image_zoom.w = width_ / image_zoom_;
        image_zoom.h = height_ / image_zoom_;

        // scrolling
        image_zoom.y = clamp(image_zoom.y + image_move_.y, 0, height_ - image_zoom.h);
        image_zoom.x = clamp(image_zoom.x + image_move_.x, 0, width_ - image_zoom.w);
    }

    SDL_Rect image_fit = {(window_width_ - width_) / 2, (window_height_ - height_) / 2, width_, height_};
    if (image_fit_factor_ != 1.0f)
    {
        image_fit.x = (window_width_ - (width_ * image_fit_factor_)) / 2.0f;
        image_fit.y = (window_height_ - (height_ * image_fit_factor_)) / 2.0f;
        image_fit.w = width_ * image_fit_factor_;
        image_fit.h = height_ * image_fit_factor_;
    }

    SDL_RenderCopyEx(renderer_, texture_, &image_zoom, &image_fit, image_rot_deg_, nullptr, SDL_FLIP_VERTICAL);

    SDL_RenderPresent(renderer_);
}

void QShow::Run()
{
    while (!quit_)
    {
        SDL_WaitEvent(&sdl_event_);

        bool do_render = false;

        switch (sdl_event_.type)
        {
        case SDL_WINDOWEVENT:
            switch (sdl_event_.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                quit_ = true;
                break;

            case SDL_WINDOWEVENT_RESIZED:
                OnSizeChanged(sdl_event_.window.data1, sdl_event_.window.data2);
                do_render = true;
                break;

            case SDL_WINDOWEVENT_EXPOSED:
                do_render = true;
                break;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (alt_mousewheel)
            {
                image_zoom_ = std::max(1.0f, image_zoom_ + ((sdl_event_.wheel.y > 0)? 0.2f : -0.2f));
                SDL_Log("new zoom factor: %f", image_zoom_);
                do_render = true;
            }
            else
            {
                if (ChangeImage((sdl_event_.wheel.y > 0)? Browse::PREVIOUS : Browse::NEXT))
                {
                    LoadTexture();
                    do_render = true;
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (!mouse_move_ && sdl_event_.button.button == SDL_BUTTON_LEFT)
            {
                mouse_move_ = true;
                mouse_move_start_ = { sdl_event_.button.x, sdl_event_.button.y };
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (sdl_event_.button.button == SDL_BUTTON_LEFT)
            {
                mouse_move_ = false;
            }
            break;

        case SDL_MOUSEMOTION:
            if (mouse_move_ && (sdl_event_.motion.state & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK)
            {
                image_move_.x += (mouse_move_start_.x - sdl_event_.button.x);
                image_move_.y += (sdl_event_.button.y - mouse_move_start_.y);
                mouse_move_start_ = { sdl_event_.button.x, sdl_event_.button.y };
                SDL_Log("new move (%d, %d)", image_move_.x, image_move_.y);
                do_render = true;
            }
            break;

        case SDL_KEYUP:
            switch (sdl_event_.key.keysym.sym)
            {
            case SDLK_LCTRL:
                alt_mousewheel = false;
                break;
            }
            break;

        case SDL_KEYDOWN:
            switch (sdl_event_.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                quit_ = true;
                break;

            case SDLK_f:
                if (sdl_event_.key.repeat == 0)
                {
                    fullscreen_ = !fullscreen_;
                    do_render = true;
                }
                break;

            case SDLK_r:
                if (sdl_event_.key.repeat == 0)
                {
                    image_rot_deg_ += 90.0f * ((sdl_event_.key.keysym.mod & KMOD_SHIFT)? -1.0f : 1.0f);
                    image_rot_deg_ = std::fmod(image_rot_deg_, 360.0f);
                    do_render = true;
                }
                break;

            case SDLK_LCTRL:
                alt_mousewheel = true;
                break;

            case SDLK_PLUS:
                image_zoom_ = image_zoom_ + 0.2f;
                do_render = true;
                break;

            case SDLK_MINUS:
                image_zoom_ = std::max(1.0f, image_zoom_ - 0.2f);
                do_render = true;
                break;

            case SDLK_UP:
                image_move_.y = std::min(image_move_.y + 5, int(height_ / 2 - (height_ / 2 / image_zoom_)));
                do_render = true;
                break;

            case SDLK_DOWN:
                image_move_.y = std::max(image_move_.y - 5, -int(height_ / 2 - (height_ / 2 / image_zoom_)));
                do_render = true;
                break;

            case SDLK_RIGHT:
                image_move_.x = std::min(image_move_.x + 5, int(width_ / 2 - (width_ / 2 / image_zoom_)));
                do_render = true;
                break;

            case SDLK_LEFT:
                image_move_.x = std::max(image_move_.x - 5, -int(width_ / 2 - (width_ / 2 / image_zoom_)));
                do_render = true;
                break;

            case SDLK_PAGEUP:
            case SDLK_PAGEDOWN:
                if (ChangeImage((sdl_event_.key.keysym.sym == SDLK_PAGEUP)? Browse::PREVIOUS : Browse::NEXT))
                {
                    LoadTexture();
                    do_render = true;
                }
                break;
            }
            break;
        }

        if (do_render)
        {
            Render();
            SetTitle();
        }

        SDL_Delay(1);
    }
}

void QShow::SetTitle()
{
    std::snprintf(title_string, 256, "[%s] (%dx%d %d%%) - qShow",
                                     (*current_file_).filename().generic_string().c_str(),
                                     width_,
                                     height_,
                                     (int)(width_ *  image_fit_factor_ * 100.f / width_));

    SDL_SetWindowTitle(window_, title_string);
}

bool QShow::ChangeImage(Browse direction)
{
    switch (direction)
    {
    case Browse::NEXT:
        if (current_file_ + 1 == filelist_.end()) { return false; }
        ++current_file_;
        break;

    case Browse::PREVIOUS:
        if (current_file_ == filelist_.begin()) { return false; }
        --current_file_;
        break;
    }

    return LoadImage(*current_file_);
}
