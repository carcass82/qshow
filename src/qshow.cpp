/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#include "qshow.h"

QShow::QShow(const std::string& arg)
{
    InitSDL();

    std::list<std::string> supportedFileExts;
    supportedFileExts.push_back(".jpg");
    supportedFileExts.push_back(".jpeg");
    supportedFileExts.push_back(".png");
    supportedFileExts.push_back(".bmp");
    supportedFileExts.push_back(".gif");
    supportedFileExts.push_back(".tga");
    supportedFileExts.push_back(".tif");

    fs::path selectedFile(fs::current_path() / fs::path(arg));

    auto directory = fs::directory_iterator(selectedFile.parent_path());
    for (auto& path : directory) {

        const std::string& extension = path.path().extension().generic_string();

        if (std::find(supportedFileExts.begin(), supportedFileExts.end(), extension) != supportedFileExts.end()) {
            filelist_.push_back(path.path());
        }
    }

    current_file_ = filelist_.begin();
    LoadImage(selectedFile);

    SetVideoMode();
}

QShow::~QShow()
{
    SDL_FreeSurface(original_image_);
    SDL_FreeSurface(image_);

    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);

    IMG_Quit();
    SDL_Quit();
}

void QShow::InitSDL()
{
    int res = SDL_Init(SDL_INIT_VIDEO);
    SDL_assert(res >= 0);

    res = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
    SDL_assert(res != 0);
}

void QShow::LoadImage(const fs::path& image_file)
{
    // reset zoom and rotation
    image_zoom_ = 1.0f;
    image_rot_deg_ = 0.0f;

    SDL_FreeSurface(original_image_);

    original_image_ = IMG_Load(image_file.generic_string().c_str());
    SDL_assert(original_image_);

    width_ = original_image_->w;
    height_ = original_image_->h;
    bpp_ = original_image_->format->BitsPerPixel;

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

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    OnImageChanged();
}

void QShow::OnImageChanged()
{
    SDL_DestroyTexture(texture_);

    texture_ = SDL_CreateTextureFromSurface(renderer_, original_image_);

    SDL_assert(texture_);
    SDL_Log("created ARGB texture %dx%d", width_, height_);
}

void QShow::Render()
{
    SDL_RenderClear(renderer_);

    //SDL_GetWindowSize(window_, &win_width, &win_height);

    int w = original_image_->w;
    int h = original_image_->h;

    SDL_Rect imageZoom{0, 0, w, h};
    if (image_zoom_ > 1.0f) {
        float w2 = imageZoom.w / 2.0f;
        float h2 = imageZoom.h / 2.0f;

        imageZoom.x = w2 - (w2 / image_zoom_);
        imageZoom.y = h2 - (h2 / image_zoom_);
        imageZoom.w = w / image_zoom_;
        imageZoom.h = h / image_zoom_;

        SDL_Log("new image: (%d,%d - %d,%d)", imageZoom.x, imageZoom.y, imageZoom.w, imageZoom.h);
    }

    SDL_RenderCopyEx(renderer_, texture_, &imageZoom, nullptr, image_rot_deg_, nullptr, SDL_FLIP_NONE);

    SDL_RenderPresent(renderer_);
}

void QShow::Show()
{
    while (!quit) {

        SDL_WaitEvent(&event);

        switch (event.type) {

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                quit = true;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (ChangeImage((event.wheel.y > 0)? IM_PREV : IM_NEXT))
                OnImageChanged();
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_f:
                fullscreen_ = !fullscreen_;
                SDL_SetWindowFullscreen(window_, (fullscreen_? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
                break;
            case SDLK_r:
                image_rot_deg_ += 90.0f * ((event.key.keysym.mod & KMOD_SHIFT)? -1.0f : 1.0f);
                image_rot_deg_ = std::fmod(image_rot_deg_, 360.0f);
                break;
            case SDLK_PLUS:
                image_zoom_ = image_zoom_ + 0.1f;
                break;
            case SDLK_MINUS:
                image_zoom_ = std::max(1.0f, image_zoom_ - 0.1f);
                break;
            }
            break;

        case SDL_KEYUP:
            break;
        }

        Render();
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

    case IM_NEXT:
        if (++current_file_== filelist_.end()) {
            --current_file_;
            return false;
        }
        break;

    case IM_PREV:
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
