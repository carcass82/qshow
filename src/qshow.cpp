/*
 * QShow - a fast and KISS image viewer
 *
 * 2008 (C) BELiAL <carlo.casta@gmail.com>
 */

#include "qshow.h"

QShow::QShow(const std::string& arg)
    : img(nullptr)
    , imgModified(nullptr)
    , screen(nullptr)
    , quit(false)
    , fullscreen(false)
    , render(false)
    , showCheckerBoard(false)
    , zoomFactor(1.0f)
    , rotDegrees(0.0f)
{
    scrollEnable[SC_LEFT] = false;
	scrollEnable[SC_RIGHT] = false;
	scrollEnable[SC_UP] = false;
	scrollEnable[SC_DOWN] = false;
	isScrolling = false;

	std::list<std::string> supportedFileExts;
    supportedFileExts.push_back(".jpg");
    supportedFileExts.push_back(".png");
    supportedFileExts.push_back(".bmp");
    supportedFileExts.push_back(".gif");
    supportedFileExts.push_back(".tga");
    supportedFileExts.push_back(".tif");

    fs::path selectedFile(fs::current_path() / fs::path(arg));

    auto it = fs::directory_iterator(selectedFile.parent_path());
    for (auto& p : it) {
        const std::string ext = p.path().extension().generic_string();
        if (std::find(supportedFileExts.begin(), supportedFileExts.end(), ext) != supportedFileExts.end()) {
            filelist.push_back(p.path());
        }
    }

    curFile = filelist.begin();
    LoadImage(selectedFile);

    InitSDL();
}

QShow::~QShow()
{
	Shutdown();
}

void QShow::Shutdown()
{
	SDL_FreeSurface(img);
	SDL_FreeSurface(imgModified);
	SDL_FreeSurface(screen);
	IMG_Quit();
	SDL_Quit();
}

void QShow::InitSDL()
{
    int res = SDL_Init(SDL_INIT_VIDEO);
	assert(res == 0);

	const SDL_VideoInfo* vinfo = SDL_GetVideoInfo();

	// best resolution possible (will be used in fullscreen mode)
	bestWidth = vinfo->current_w;
	bestHeight = vinfo->current_h;

	// default initial window dimensions
	width = bestWidth / 2;
	height = bestHeight / 2;

	imgPosition.x = 0;
	imgPosition.y = 0;

	SetVideoMode();
	Reshape();

    SetTitle((*curFile).filename().generic_string());

    res = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    assert(res != 0);
}

void QShow::LoadImage(const fs::path& image_file)
{
	// reset zoom and rotation
	zoomFactor = 1.0;
	rotDegrees = 0.0;

    if (image_file.extension() == ".png") {
        SDL_FreeSurface(img);
        img = IMG_Load(image_file.generic_string().c_str());
        assert(img);
    }
}

bool QShow::ChangeImage(BrowseImg whichImg)
{
	switch (whichImg) {
	case IM_NEXT:
        ++curFile;
        if (curFile == filelist.end()) {
            --curFile;
			return false;
		}
		break;

	case IM_PREV:
		if (curFile == filelist.begin())
			return false;
		--curFile;
		break;

	default:
		break;
	}

    SetTitle((*curFile).filename().generic_string());

    LoadImage(*curFile);

	return true;
}

void QShow::Draw()
{
	assert(screen);
	assert(imgModified);

	SDL_Rect pos = imgPosition;

	if (showCheckerBoard)
		DrawCheckerPattern(); // for images with alpha channel
	else
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

	int res = SDL_BlitSurface(imgModified, NULL, screen, &pos);
	assert (res == 0);

	SDL_Flip(screen);
}

void QShow::DrawCheckerPattern()
{
	// grey       204
	// dark grey  102
	unsigned char color[2] = {0xcc, 0x66};

	unsigned char col;
	int greytype = 0;
	SDL_Rect quad;

	for (int i = 0; i < screen->w; i += 10, ++greytype) {
		for (int j = 0; j < screen->h; j += 10, ++greytype) {

			quad.x = i;
			quad.y = j;
			quad.w = 10;
			quad.h = 10;

			col = color[greytype % 2];

			SDL_FillRect(screen, &quad, SDL_MapRGB(screen->format, col, col, col));
		}
	}
}

void QShow::Reshape()
{
	if (!img)
		return;

	float minSizeW = (fullscreen)? bestWidth : width;
	float minSizeH = (fullscreen)? bestHeight : height;
	float factor = std::min((minSizeW / img->w), (minSizeH / img->h));

	assert(factor > 0.0);
	SDL_FreeSurface(imgModified);
	imgModified = (factor == 1.0f)?
		SDL_ConvertSurface(img, img->format, img->flags) :
		rotozoomSurface(img, rotDegrees, factor, SMOOTHING_ON);

	// if img has been modified, update zoomFactor
	zoomFactor = (factor >= 1.0f)? 1.0 : factor;
}

void QShow::ZoomImage(ZoomParam op, int x, int y)
{
	float wantZoom = zoomFactor += 0.1 * ((op == ZOOMIN)? 1 : -1);
	if (wantZoom < 0.0 || wantZoom > 5.0)
		return;

	zoomFactor = wantZoom;
	SDL_FreeSurface(imgModified);
	if (zoomFactor < 1.0 || zoomFactor > 1.0) {
		imgModified = rotozoomSurface(img, rotDegrees, zoomFactor, SMOOTHING_ON);
	} else {
		imgModified = SDL_ConvertSurface(img, img->format, img->flags);
	}

	Scroll(SC_CHECK);
}

void QShow::SetVideoMode()
{
	screen = SDL_SetVideoMode((fullscreen)? bestWidth : width,
	                          (fullscreen)? bestHeight : height, 0,
	                          SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE |
	                          ((fullscreen) ? SDL_FULLSCREEN : 0));
	assert(screen);
}

bool QShow::Scroll(ScrollDirection dir)
{
	if (dir != SC_CHECK && !scrollEnable[dir])
		return false;

	switch (dir) {
	case SC_DOWN:
		imgPosition.y -= 10;
		break;
	case SC_UP:
		imgPosition.y += 10;
		break;
	case SC_LEFT:
		imgPosition.x += 10;
		break;
	case SC_RIGHT:
		imgPosition.x -= 10;
		break;
	case SC_CHECK:
		scrollEnable[SC_LEFT]  = (imgPosition.x < 0);
		scrollEnable[SC_RIGHT] = (imgPosition.w > screen->w);
		scrollEnable[SC_UP]    = (imgPosition.y < 0);
		scrollEnable[SC_DOWN]  = (imgPosition.h > screen->h);
		return true;
	default:
		return false;
	}

	// unused by SDL_Blit but useful for scrolling
	imgPosition.w = imgPosition.x + imgModified->w;
	imgPosition.h = imgPosition.y + imgModified->h;

	return Scroll(SC_CHECK);
}

void QShow::CenterImage()
{
	// center image
	imgPosition.x = (screen->w / 2) - (imgModified->w / 2);
	imgPosition.y = (screen->h / 2) - (imgModified->h / 2);

	// unused by SDL_Blit but useful for scrolling
	imgPosition.w = imgPosition.x + imgModified->w;
	imgPosition.h = imgPosition.y + imgModified->h;

	Scroll(SC_CHECK);
}

int QShow::Show()
{
	CenterImage();
	Draw();

	while (!quit) {
		render = false;
		SDL_WaitEvent(&event);

		switch (event.type) {
		case SDL_VIDEOEXPOSE:
			render = true;
			break;

		case SDL_VIDEORESIZE:
			if (!fullscreen) {
				width = event.resize.w;
				height = event.resize.h;
				SetVideoMode();
				Reshape();
				CenterImage();
				render = true;
			}
			break;

		case SDL_MOUSEBUTTONDOWN: {
			SDL_MouseButtonEvent m = event.button;
			switch(m.button) {
			case SDL_BUTTON_WHEELUP:   // mouse wheel up
				if (ChangeImage(IM_PREV)) {
					Reshape();
					CenterImage();
					render = true;
				}
				break;
			case SDL_BUTTON_WHEELDOWN: // mouse wheel down
				if (ChangeImage(IM_NEXT)) {
					Reshape();
					CenterImage();
					render = true;
				}
				break;
			default:
				break;
			}
			break;
		}

		case SDL_KEYDOWN: {
			SDLKey key = event.key.keysym.sym;
			switch (key) {
			case SDLK_ESCAPE:
				quit = true;
				break;
			case SDLK_f:
				fullscreen = !fullscreen;
				SetVideoMode();
				Reshape();
				CenterImage();
				render = true;
				break;
			case SDLK_h:
				showCheckerBoard = !showCheckerBoard;
				render = true;
				break;
			case SDLK_PLUS:
			case SDLK_MINUS:
				ZoomImage((key == SDLK_PLUS)? ZOOMIN : ZOOMOUT);
				CenterImage();
				render = true;
				break;
			case SDLK_7:
			case SDLK_9:
				RotateImage((key == SDLK_7)? ROT_CW : ROT_CCW);
				Reshape();
				CenterImage();
				render = true;
				break;
			case SDLK_PAGEUP:
				if (ChangeImage(IM_PREV)) {
					Reshape();
					CenterImage();
					render = true;
				}
				break;
			case SDLK_PAGEDOWN:
				if (ChangeImage(IM_NEXT)) {
					Reshape();
					CenterImage();
					render = true;
				}
				break;
			case SDLK_RIGHT:
				isScrolling = true;
				scrollDir = SC_RIGHT;
				break;
			case SDLK_LEFT:
				isScrolling = true;
				scrollDir = SC_LEFT;
				break;
			case SDLK_UP:
				isScrolling = true;
				scrollDir = SC_UP;
				break;
			case SDLK_DOWN:
				isScrolling = true;
				scrollDir = SC_DOWN;
				break;
			default:
				break;
			}
			break;
		}

		case SDL_KEYUP: {
			SDLKey key = event.key.keysym.sym;
			switch (key) {
			case SDLK_RIGHT:
			case SDLK_LEFT:
			case SDLK_DOWN:
			case SDLK_UP:
				isScrolling = false;
				break;
			default:
				break;
			}
			break;
		}

		case SDL_QUIT:
			quit = true;
			break;

		default:
			break;
		}

		if (isScrolling)
			render = Scroll(scrollDir);

		if (render) {
			Draw();
		}
	}
	return 0;
}

void QShow::SetTitle(const std::string& filename)
{
    size_t title_length = std::snprintf(nullptr, 0, "qShow v1.0 [%s]", filename.c_str());
    std::vector<char> title_string(title_length + 1);
    std::snprintf(&title_string[0], title_length + 1, "qShow v1.0 [%s]", filename.c_str());
    SDL_WM_SetCaption(&title_string[0], "");
}
