#pragma once

#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <direct.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
	//�°����ͼ��ת���ṹ��Ҫ�����ͷ�ļ�
#include "libswscale/swscale.h"
	//SDL
#include "sdl/SDL.h"
#include "sdl/SDL_thread.h"

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"SDL.lib")
};
