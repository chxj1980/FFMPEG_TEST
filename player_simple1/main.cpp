#include "player_simple.h"

int _tmain(int argc, _TCHAR* argv[])
{
	av_register_all();
	avformat_network_init();

	AVFormatContext* pFormatCtx = pFormatCtx = avformat_alloc_context();
	if (!pFormatCtx)
	{
		printf("avformat_alloc_context is failed. \n");
		return -1;
	}
	char filepath[] = "../testfile/mp4_test.mp4";
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("avformat_open_input is failed. \n");
		return -1;
	}
	if (av_find_stream_info(pFormatCtx) < 0)
	{
		printf("av_find_stream_info is failed.\n");
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
		printf("Didn't find a video stream.\n");
		return -1;
	}

	AVCodecContext* pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		printf("avcodec_find_decoder is failed. \n");
		return -1;
	}
	if (avcodec_open(pCodecCtx, pCodec) < 0)
	{
		printf("avcodec_open is failed. \n");
		return -1;
	}

	AVFrame* pFrame = avcodec_alloc_frame();
	AVFrame* pFrameYUV = avcodec_alloc_frame();
	uint8_t* out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	//------------SDL----------------
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("SDL_Init is failed : %s\n", SDL_GetError());
		exit(1);
	}
	SDL_Surface *screen;
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
	if (!screen)
	{
		printf("SDL: could not set video mode - exiting\n");
		exit(1);
	}
	SDL_Overlay *bmp;
	bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height, SDL_YV12_OVERLAY, screen);
	SDL_Rect rect;
	//---------------
	int ret, got_picture;
	static struct SwsContext *img_convert_ctx;
	int y_size = pCodecCtx->width * pCodecCtx->height;

	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
	memset(packet, 0, sizeof(AVPacket));
	av_new_packet(packet, y_size);
	//输出一下信息-----------------------------
	printf("File Info-----------------------------------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	//------------------------------
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0)
			{
				printf("avcodec_decode_video2 is failed. \n");
				return -1;
			}
			if (got_picture)
			{
				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

				SDL_LockYUVOverlay(bmp);
				bmp->pixels[0] = pFrameYUV->data[0];
				bmp->pixels[2] = pFrameYUV->data[1];
				bmp->pixels[1] = pFrameYUV->data[2];
				bmp->pitches[0] = pFrameYUV->linesize[0];
				bmp->pitches[2] = pFrameYUV->linesize[1];
				bmp->pitches[1] = pFrameYUV->linesize[2];
				SDL_UnlockYUVOverlay(bmp);
				rect.x = 0;
				rect.y = 0;
				rect.w = pCodecCtx->width;
				rect.h = pCodecCtx->height;
				SDL_DisplayYUVOverlay(bmp, &rect);
				//延时40ms
				SDL_Delay(40);
			}
		}
		av_free_packet(packet);
	}
	delete[] out_buffer;
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}