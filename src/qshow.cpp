/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 * 2017 - updated to SDL2
 */

#include "qshow.h"

QShow::QShow(const std::string& arg)
{
    InitSDL();

    fs::path selectedFile(fs::current_path() / fs::path(arg));
    current_file_ = filelist_.begin();

    auto directory = fs::directory_iterator(selectedFile.parent_path());
    for (auto& path : directory) {

        FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.path().generic_string().c_str());
        if (format != FIF_UNKNOWN) {
            filelist_.push_back(path.path());
        }
    }

    current_file_ = std::find(filelist_.begin(), filelist_.end(), selectedFile);
    LoadImage(selectedFile);

    SetVideoMode();

    OnImageChanged();
}

QShow::~QShow()
{
    FreeImage_Unload(original_image_);

    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);

    SDL_Quit();
}

void QShow::InitSDL()
{
    int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_assert(res >= 0);
}

void QShow::LoadImage(const fs::path& image_file)
{
    // reset zoom and rotation
    image_zoom_ = 1.0f;
    image_rot_deg_ = 0.0f;

    FreeImage_Unload(original_image_);
    original_image_ = nullptr;

    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(image_file.generic_string().c_str());
    original_image_ = FreeImage_Load(format, image_file.generic_string().c_str());
    FIBITMAP* tmp = original_image_;
    original_image_ = FreeImage_ConvertTo32Bits(original_image_);
    FreeImage_Unload(tmp);

    SDL_assert(original_image_);

    width_ = FreeImage_GetWidth(original_image_);
    height_ = FreeImage_GetHeight(original_image_);
    bpp_ = FreeImage_GetBPP(original_image_);

    SDL_Log("Loaded %s (%dx%d), %dbpp", image_file.generic_string().c_str(), width_, height_, bpp_);
}

void QShow::SetVideoMode()
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

void QShow::OnImageChanged()
{
    SDL_DestroyTexture(texture_);

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STATIC, width_, height_);
    SDL_UpdateTexture(texture_, nullptr, FreeImage_GetBits(original_image_), width_ * bpp_ / 8);

    SDL_assert(texture_);
}

void QShow::OnSizeChanged(int32_t w, int32_t h)
{
    image_fit_factor_ = std::min(static_cast<float>(w) / width_, static_cast<float>(h) / height_);
    window_width_ = w;
    window_height_ = h;

    SDL_Log("resize event: %dx%d, new fit factor %f", w, h, image_fit_factor_);
}

void QShow::Render()
{
    SDL_SetWindowFullscreen(window_, (fullscreen_? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

    SDL_RenderClear(renderer_);

    SDL_Rect image_zoom{0, 0, width_, height_};
    if (image_zoom_ != 1.0f) {
        float w2 = image_zoom.w / 2.0f;
        float h2 = image_zoom.h / 2.0f;

        image_zoom.x = w2 - (w2 / image_zoom_);
        image_zoom.y = h2 - (h2 / image_zoom_);
        image_zoom.w = width_ / image_zoom_;
        image_zoom.h = height_ / image_zoom_;
    }

    SDL_Rect image_fit{(window_width_ - width_) / 2, (window_height_ - height_) / 2, width_, height_};
    if (image_fit_factor_ != 1.0f) {
        image_fit.x = (window_width_ - (width_ * image_fit_factor_)) / 2.0f;
        image_fit.y = (window_height_ - (height_ * image_fit_factor_)) / 2.0f;
        image_fit.w = width_ * image_fit_factor_;
        image_fit.h = height_ * image_fit_factor_;
    }

    SDL_RenderCopyEx(renderer_, texture_, &image_zoom, &image_fit, image_rot_deg_, nullptr, SDL_FLIP_VERTICAL);

    SDL_RenderPresent(renderer_);
}

void QShow::Show()
{
    while (!quit_) {

        SDL_WaitEvent(&sdl_event_);

        bool do_render = false;

        switch (sdl_event_.type) {

        case SDL_WINDOWEVENT:
            switch (sdl_event_.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                quit_ = true;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                OnSizeChanged(sdl_event_.window.data1, sdl_event_.window.data2);
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                do_render = true;
                break;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (ChangeImage((sdl_event_.wheel.y > 0)? IMG_PREV : IMG_NEXT)) {
                OnImageChanged();
                OnSizeChanged(window_width_, window_height_);
                do_render = true;
            }
            break;

        case SDL_KEYDOWN:
            switch (sdl_event_.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit_ = true;
                break;
            case SDLK_f:
                if (sdl_event_.key.repeat == 0) {
                    fullscreen_ = !fullscreen_;
                    do_render = true;
                }
                break;
            case SDLK_r:
                if (sdl_event_.key.repeat == 0) {
                    image_rot_deg_ += 90.0f * ((sdl_event_.key.keysym.mod & KMOD_SHIFT)? -1.0f : 1.0f);
                    image_rot_deg_ = std::fmod(image_rot_deg_, 360.0f);
                    do_render = true;
                }
                break;
            case SDLK_PLUS:
                image_zoom_ = image_zoom_ + 0.1f;
                do_render = true;
                break;
            case SDLK_MINUS:
                image_zoom_ = std::max(1.0f, image_zoom_ - 0.1f);
                do_render = true;
                break;
            }
            break;

        case SDL_KEYUP:
            break;
        }

        if (do_render)
            Render();

        SDL_Delay(1);
    }
}

void QShow::SetTitle(const std::string& filename)
{
    size_t title_length = std::snprintf(nullptr, 0, "qShow v1.0 [%s]", filename.c_str());
    std::vector<char> title_string(title_length + 1);
    std::snprintf(&title_string[0], title_length + 1, "qShow v1.0 [%s]", filename.c_str());

    SDL_SetWindowTitle(window_, &title_string[0]);
}

bool QShow::ChangeImage(BrowseImg direction)
{
    switch (direction) {

    case IMG_NEXT:
        if (++current_file_== filelist_.end()) {
            --current_file_;
            return false;
        }
        break;

    case IMG_PREV:
        if (current_file_ == filelist_.begin())
            return false;
        --current_file_;
        break;

    default:
        break;

    }

    SetTitle((*current_file_).filename().generic_string());
    LoadImage(*current_file_);

    return true;
}
