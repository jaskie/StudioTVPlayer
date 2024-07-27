#include "pch.h"
#include <iostream>

#include "Core/VideoFormat.h"
#include "Core/Player.h"
#include "DecklinkKeyerType.h"
#include "DecklinkTimecodeSource.h"
#include "TimecodeOutputSource.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"
#include "Decklink/DecklinkInput.h"
#include "Decklink/DecklinkInfo.h"
#include "DecklinkTimecodeSource.h"
#include "Ndi/NdiOutput.h"
#include "FFmpeg/FFmpegInput.h"
#include "FFmpeg/FFmpegOutput.h"
#include "FFmpeg/FFOutputParams.h"
#include "FFmpeg/FFmpegUtils.h"
#include "FFmpeg/VideoOverlayFilter.h"
#include "PixelFormat.h"
#include "Common/ComInitializer.h"

#undef DEBUG

using namespace TVPlayR;

#define FFMPEG_LOG_LEVEL AV_LOG_INFO

static void avlog_cb(void * ptr, int level, const char * fmt, va_list vargs) {
	if (level <= FFMPEG_LOG_LEVEL)
	{
		char line[ERROR_STRING_LENGTH*2];
		static int prefix(1);
		av_log_format_line(ptr, level, fmt, vargs, line, ERROR_STRING_LENGTH*2, &prefix);
		OutputDebugStringA(line);
	}
}


int main()
{
#ifdef _DEBUG
	try
	{
		av_log_set_flags(AV_LOG_PRINT_LEVEL | AV_LOG_SKIP_REPEATED);
		av_log_set_level(FFMPEG_LOG_LEVEL);
		av_log_set_callback(avlog_cb);
#else
		av_log_set_callback(NULL);
#endif
		Common::ComInitializer com_initializer;
		const Core::VideoFormatType video_format = Core::VideoFormatType::v2160p2500;
		const PixelFormat pixel_format = PixelFormat::yuv422;
		const int audio_channels = 2;
		const int sample_rate = 48000;

		auto player = std::make_shared<Core::Player>("1", video_format, pixel_format, audio_channels, sample_rate);
	/*	player.SetAudioVolumeCallback([](std::vector<float>& volume, float coherence) {
			std::cout << coherence << "\n";
			});
	*/
		Decklink::DecklinkIterator iterator;
		int device_index = 0;
		
		for (size_t i = 0; i < iterator.Size(); i++)
			std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
		auto decklink_output = iterator.CreateOutput(*iterator[device_index], DecklinkKeyerType::Default, TimecodeOutputSource::TimeToEnd);
		decklink_output->Initialize(video_format, pixel_format, audio_channels, sample_rate);
		player->AddOutputSink(decklink_output);
		decklink_output->RegisterClockTarget(player);

		//auto overlay = std::make_shared<FFmpeg::VideoOverlayFilter>(player.Format(), PixelFormatToFFmpegFormat(player.PixelFormat()));
		//player.AddOverlay(overlay);		

		auto ndi = std::make_shared<Ndi::NdiOutput>("Player 1", "");
		ndi->Initialize(video_format, pixel_format, audio_channels, sample_rate);
		player->AddOutputSink(ndi);
		//ndi->RegisterClockTarget(player);
		
		FFmpeg::FFOutputParams stream_params
		{
			"udp://127.0.0.1:1234?pkt_size=1316",									// Url
			"libx264",																// VideoCodec
			"aac", 																	// AudioCodec
			4000,																	// VideoBitrate
			128, 																	// AudioBitrate
			"g=18,bf=0",															// Options
			"",//"bwdif,scale=384x216,interlace",									// VideoFilter
			"yuv420p",																// pixel format
			"service_name=\"Test service\",service_provider=\"TVPlayR test\"",		// OutputMetadata
			"",															// VideoMetadata
			"language=pol",												// AudioMetadata
			121, // VideoStreamId
			122,  // AudioStreamId
			"mpegts"	// output format
		};
		
		//FFmpeg::FFOutputParams stream_params{ "d:\\temp\\aaa.mov", "libx264", "aac", 4000, 128 };
		//auto stream = std::make_shared<FFmpeg::FFmpegOutput>(stream_params);
		//stream->RegisterClockTarget(player);
		//stream->Initialize(video_format, pixel_format, audio_channels, sample_rate);
		//player->AddOutputSink(stream);
		
		//auto input = iterator.CreateInput(*iterator[device_index], Core::VideoFormatType::v1080i5000, 2);

		auto input = std::make_shared<FFmpeg::FFmpegInput>("G:\\media\\test5.mov", Core::HwAccel::none, "");
		//input->SetIsLoop(true);
		//auto input = std::make_shared<FFmpeg::FFmpegInput>("udp://225.100.10.26:5500", Core::HwAccel::none, "", 2);
		//auto seek = input->GetVideoDuration() - AV_TIME_BASE;
		//input->Seek(seek);
		//input->SetStoppedCallback([] {std::wcout << L"Stopped\n"; });
		//input->SetLoadedCallback([] {std::wcout << L"Loaded\n"; });
		//input->SetFramePlayedCallback([](Core::FrameTimeInfo& time) {});
		input->Play();
		//input->SetIsLoop(true);
		player->Load(input);


		// prepare input and recording
		/*
		auto decklink_input = iterator.CreateInput(*iterator[0], Core::VideoFormatType::pal_fha, 2, DecklinkTimecodeSource::RP188Any, true, true);
		FFmpeg::FFOutputParams record_params
		{ 
			"d:\\temp\\cccc.mxf", 
			"mpeg2video", "pcm_s24le", 
			50000, 0,
			"maxrate=50000k,minrate=50000k,bufsize=2M,flags=+ildct+low_delay,g=1,dc=10,ps=1,qmin=1,qmax=3,rc_init_occupancy=2M,intra_vlc=1,non_linear_quant=1,colorspace=5,rc_max_vbv_use=1",
			"pad=720:608:0:32", "yuv422p",
			"", "", "",
			0, 0,
			"mxf_d10"
		};
		auto record_file = std::make_shared<FFmpeg::FFmpegOutput>(record_params);
		record_file->Initialize(video_format, pixel_format, audio_channels, sample_rate);
		decklink_input->AddOutputSink(record_file);
		*/
		while (true)
		{
			char i = std::cin.get();
 			if (i == 'q')
				break;
			/*if (i == 'c')
				player->Clear();
			if (i == 'r')
				player.AddOutput(stream);
			if (i == 's')
				player.RemoveOutput(stream);*/

			if (i == 's')
				input->Seek(AV_TIME_BASE * 10);
			if (i == 'l')
				player->Load(input);
			if (i == ' ')
				if (input->IsPlaying())
					input->Pause();
				else	 
					input->Play();
		}
		//decklink_input->RemoveOutputSink(record_file);
		decklink_output->UnregisterClockTarget(player);
		player->RemoveOutputSink(ndi);
		player->RemoveOutputSink(decklink_output);
		//player->RemoveOutputSink(stream);
#ifdef _DEBUG
	}
	catch (std::exception e)
	{
		OutputDebugStringA("\n");
		OutputDebugStringA(e.what());
	}
	if (_CrtDumpMemoryLeaks())
		OutputDebugStringA("\nMemory leak!\n");
#endif
}