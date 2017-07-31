#include "player_simple2.h"

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
int thread_exit = 0;

int sfp_refresh_thread(void *opaque)
{
	while (thread_exit == 0) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	av_register_all();
	avformat_network_init();

	AVFormatContext* pFormatCtx = avformat_alloc_context();
	char filepath[] = "../testfile/mp4_test.mp4";
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.（无法打开输入流）\n");
		return -1;
	}
	if (av_find_stream_info(pFormatCtx)<0)
	{
		printf("Couldn't find stream information.（无法获取流信息）\n");
		return -1;
	}
	int videoindex = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	if (videoindex == -1)
	{
		printf("Didn't find a video stream.（没有找到视频流）\n");
		return -1;
	}

	AVCodecContext* pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		printf("Codec not found.（没有找到解码器）\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	{
		printf("Could not open codec.（无法打开解码器）\n");
		return -1;
	}

	AVFrame* pFrame = avcodec_alloc_frame();
	AVFrame* pFrameYUV = avcodec_alloc_frame();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	//------------SDL----------------
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	int screen_w = 0, screen_h = 0;
	SDL_Window *screen;
	//SDL 2.0 Support for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL);

	if (!screen)
	{
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}

	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	SDL_Rect sdlRect;
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	int ret, got_picture;
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("File Information（文件信息）---------------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");

	struct SwsContext *img_convert_ctx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	//--------------
	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
	//
	//Event Loop
	SDL_Event event;
	while (1)
	{
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			//------------------------------
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{
				if (packet->stream_index == videoindex)
				{
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if (ret < 0)
					{
						printf("Decode Error.（解码错误）\n");
						return -1;
					}
					if (got_picture)
					{
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
						//SDL---------------------------
						SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]);
						SDL_RenderClear(sdlRenderer);
						SDL_RenderCopy(sdlRenderer, sdlTexture, &sdlRect, &sdlRect);
						SDL_RenderPresent(sdlRenderer);
						//SDL End-----------------------
					}
				}
				av_free_packet(packet);
			}
			else
			{
				//Exit Thread
				thread_exit = 1;
				break;
			}
		}
	}

	sws_freeContext(img_convert_ctx);

	SDL_Quit();
	//--------------
	av_free(out_buffer);
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

