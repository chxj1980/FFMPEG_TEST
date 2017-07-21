#pragma once

#include <stdio.h>
#include <tchar.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
// SDL2.0
#include "sdl2/SDL.h"
#include "sdl2/SDL_thread.h"

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"SDL2.lib")
};