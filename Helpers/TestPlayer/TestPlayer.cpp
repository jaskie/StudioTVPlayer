#include "pch.h"
#include <iostream>

#include "Core/Channel.h"
#include "Decklink/Iterator.h"
#include "Decklink/Decklink.h"
#include "Ndi/Ndi.h"
#include "FFMpeg/FFMpegInputSource.h"
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
		Core::Channel channel(Core::VideoFormatType::v1080i5000, Core::PixelFormat::yuv422, 2);
		Decklink::Iterator iterator;
		size_t device_index = 0;
		for (size_t i = 0; i < iterator.Size(); i++)
			std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
		auto device = iterator[device_index];
		channel.SetFrameClock(device);
		channel.AddOutput(device);
		//auto input = std::make_shared<FFmpeg::FFmpegInputSource>("D:\\Wilno\\MajaPoniatowska.wmv", Core::HwAccel::none, "", 2);
		auto input = std::make_shared<FFmpeg::FFmpegInputSource>("D:\\Wilno\\2021-04-06_KORONAWIRUS.mxf", Core::HwAccel::none, "", 2);
		//auto input = std::make_shared<FFmpeg::FFmpegInputSource>("D:\\Temp\\25i.mov", Core::HwAccel::none, "", 2);
		input->SetIsLoop(true);
		//auto input = std::make_shared<FFmpeg::FFmpegInputSource>("udp://225.100.10.26:5500", Core::HwAccel::none, "", 2);
		input->Seek(AV_TIME_BASE * 8);
		input->SetStoppedCallback([] {std::wcout << L"Stopped\n"; });
		input->SetLoadedCallback([] {std::wcout << L"Loaded\n"; });
		input->Play();
		channel.Load(input);
		//Ndi::Ndi ndi("NDI_SOURCE", "");
		//channel.SetFrameClock(ndi.OutputFrameClock());
		//channel.AddOutput(ndi);
		while (true)
		{
			char i = std::cin.get();
			if (i == 'q')
				break;
			if (i == 'c')
				channel.Clear();
			if (i == 's')
				input->Seek(AV_TIME_BASE * 2);
			if (i == 'l')
				channel.Load(input);
			if (i == ' ')
				if (input->IsPlaying())
					input->Pause();
				else	 
					input->Play();
		}
		channel.RemoveOutput(device);
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