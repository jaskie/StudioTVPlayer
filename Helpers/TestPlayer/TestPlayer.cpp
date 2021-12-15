#include "pch.h"
#include <iostream>

#include "Core/VideoFormat.h"
#include "Core/Player.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"
#include "Decklink/DecklinkInput.h"
#include "Decklink/DecklinkInfo.h"
#include "Ndi/NdiOutput.h"
#include "FFmpeg/FFmpegInput.h"
#include "FFmpeg/FFmpegOutput.h"
#include "FFmpeg/FFOutputParams.h"
#include "FFmpeg/FFmpegUtils.h"
#include "PixelFormat.h"
#include "Common/ComInitializer.h"

#undef DEBUG

using namespace TVPlayR;

#define FFMPEG_LOG_LEVEL AV_LOG_INFO

static void avlog_cb(void * ptr, int level, const char * fmt, va_list vargs) {
	if (level <= FFMPEG_LOG_LEVEL)
	{
		char line[ERROR_STRING_LENGTH];
		static int prefix(1);
		av_log_format_line(ptr, level, fmt, vargs, line, ERROR_STRING_LENGTH, &prefix);
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
		Core::Player player("Channel 1", Core::VideoFormatType::v1080i5000, PixelFormat::yuv422, 2, 48000);
		player.SetAudioVolumeCallback([](std::vector<float>& volume, float coherence) {
			std::cout << coherence << "\n";
			});
		//Decklink::DecklinkIterator iterator;
		//int device_index = 0;
		//for (size_t i = 0; i < iterator.Size(); i++)
		//	std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
		//auto decklink_output = iterator.CreateOutput(*iterator[device_index], false);
		//player.SetFrameClock(decklink_output);
		//player.AddOutput(decklink_output);
		using namespace std::chrono_literals;

		auto ndi = std::make_shared<Ndi::NdiOutput>("STUDIO_TVPLAYER", "");
		player.SetFrameClock(ndi);
		std::this_thread::sleep_for(200ms);
		player.AddOutput(ndi);
		FFmpeg::FFOutputParams stream_params{ "udp://127.0.0.1:1234?pkt_size=1316", // Url
			"libx264",															// VideoCodec
			"aac", 																	// AudioCodec
			4000,																	// VideoBitrate
			128, 																	// AudioBitrate
			"g=18,bf=0",															// Options
			"",//"bwdif,scale=384x216,interlace",										// VideoFilter
			"service_name=\"Test service\",service_provider=\"TVPlayR test\"",		// OutputMetadata
			"",															// VideoMetadata
			"language=pol",												// AudioMetadata
			121, // VideoStreamId
			122  // AudioStreamId
		};
		//FFmpeg::FFOutputParams stream_params{ "d:\\temp\\aaa.mov", "libx264", "aac", 4000, 128 };
		auto stream = std::make_shared<FFmpeg::FFmpegOutput>(stream_params);
		//player.SetFrameClock(stream);
		player.AddOutput(stream);

		//auto input = iterator.CreateInput(*iterator[device_index], Core::VideoFormatType::v1080i5000, 2);

		auto input = std::make_shared<FFmpeg::FFmpegInput>("D:\\Temp\\test4.mov", Core::HwAccel::none, "");
		//input->SetIsLoop(true);
		//auto input = std::make_shared<FFmpeg::FFmpegInput>("udp://225.100.10.26:5500", Core::HwAccel::none, "", 2);
		//auto seek = input->GetVideoDuration() - AV_TIME_BASE;
		//input->Seek(seek);
		input->SetStoppedCallback([] {std::wcout << L"Stopped\n"; });
		input->SetLoadedCallback([] {std::wcout << L"Loaded\n"; });
		input->SetFramePlayedCallback([&](int64_t time) {});
		input->Play();
		input->SetIsLoop(true);
		player.Load(input);
		while (true)
		{
			char i = std::cin.get();
			if (i == 'q')
				break;
			if (i == 'c')
				player.Clear();
			if (i == 'r')
				player.AddOutput(stream);
			if (i == 's')
				player.RemoveOutput(stream);

			if (i == 's')
				input->Seek(AV_TIME_BASE * 10);
			if (i == 'l')
				player.Load(input);
			if (i == ' ')
				if (input->IsPlaying())
					input->Pause();
				else	 
					input->Play();
		}
		player.SetFrameClock(nullptr);
		player.RemoveOutput(ndi);
		//player.RemoveOutput(decklink_output);
		player.RemoveOutput(stream);
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