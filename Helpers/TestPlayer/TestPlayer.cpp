#include "pch.h"
#include <iostream>

#include "Core/Channel.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"
#include "Decklink/DecklinkInput.h"
#include "Decklink/DecklinkInfo.h"
#include "Ndi/NdiOutput.h"
#include "FFmpeg/FFmpegInput.h"
#include "Core/PixelFormat.h"

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
		Core::Channel channel("Channel 1", Core::VideoFormatType::v1080i5000, Core::PixelFormat::bgra, 2);
		Decklink::DecklinkIterator iterator;
		int device_index = 1;
		for (size_t i = 0; i < iterator.Size(); i++)
			std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
		auto decklink_output = iterator.CreateOutput(*iterator[device_index]);
		//channel.SetFrameClock(decklink_output);
		channel.AddOutput(decklink_output);
		
		auto ndi = std::make_shared<Ndi::NdiOutput>("STUDIO_TVPLAYER", "");
		channel.SetFrameClock(ndi);
		channel.AddOutput(ndi);

		auto input = iterator.CreateInput(*iterator[device_index], Core::VideoFormatType::v1080i5000, 2);

		//auto input = std::make_shared<FFmpeg::FFmpegInput>("D:\\TEMP\\test4.mov", Core::HwAccel::none, "", 2);
		//input->SetIsLoop(true);
		//auto input = std::make_shared<FFmpeg::FFmpegInput>("udp://225.100.10.26:5500", Core::HwAccel::none, "", 2);
		//auto seek = /*input->GetVideoDuration() - */AV_TIME_BASE;
		//input->Seek(seek);
		//input->SetStoppedCallback([] {std::wcout << L"Stopped\n"; });
		//input->SetLoadedCallback([] {std::wcout << L"Loaded\n"; });
		//input->Play();
		channel.Load(input);
		while (true)
		{
			char i = std::cin.get();
			if (i == 'q')
				break;
			if (i == 'c')
				channel.Clear();
			//if (i == 's')
			//	input->Seek(AV_TIME_BASE * 0);
			if (i == 'l')
				channel.Load(input);
			if (i == ' ')
				if (input->IsPlaying())
					input->Pause();
				else	 
					input->Play();
		}
		channel.RemoveOutput(decklink_output);
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