// TimecodeDecoder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <vector>
#include <functional>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/rational.h"
#include "libavutil/mathematics.h"
#include "libavutil/dict.h"
#include "libavutil/opt.h"
#include "libavutil/avutil.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
#include "libavutil/audio_fifo.h"
#include "libswscale/swscale.h"
#include "libavutil/timecode.h"
#include "../dependencies/Ndi/Include/Processing.NDI.Lib.h"
}
#include "Decklink/DeckLinkAPI_h.h"
#include "Common/Exceptions.h"
#include "Common/Rational.h"
#include "Common/Debug.h"
#include "Common/NonCopyable.h"
#include "Common/ComInitializer.h"
#include "Core/Player.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"
#include "Decklink/DecklinkInput.h"
#include "Decklink/DecklinkInfo.h"
#include "Core/TimecodeOverlay.h"
#include "Core/VideoFormat.h"
#include "PixelFormat.h"
#include "DecklinkTimecodeSource.h"
#include "TimecodeOutputSource.h"
#include "DecklinkKeyer.h"
#include "Ndi/NdiOutput.h"
#include "FFmpeg/FFmpegInput.h"

using namespace TVPlayR;

int main()
{
#ifdef _DEBUG
	try
	{
#endif
	Common::ComInitializer com_initializer;
	const Core::VideoFormatType video_format = Core::VideoFormatType::pal_fha;
	const PixelFormat pixel_format = PixelFormat::bgra;
	const int audio_channels = 2;
	const int audio_sample_rate = 48000;
	Core::Player player("Player 1", video_format, pixel_format, audio_channels, audio_sample_rate);
	Decklink::DecklinkIterator iterator;
	int device_index = 1;
	//for (size_t i = 0; i < iterator.Size(); i++)
	//	std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
	auto output = iterator.CreateOutput(*iterator[device_index], TVPlayR::DecklinkKeyer::Default, TVPlayR::TimecodeOutputSource::Timecode);
	auto ndi = std::make_shared<Ndi::NdiOutput>("NDI Output", "");
	auto overlay = std::make_shared<Core::TimecodeOverlay>(TVPlayR::TimecodeOutputSource::Timecode, player.Format().type(), player.PixelFormat());
	output->Initialize(video_format, pixel_format, audio_channels, audio_sample_rate);
	ndi->Initialize(video_format, pixel_format, audio_channels, audio_sample_rate);
	player.AddOverlay(overlay);
	player.SetFrameClockSource(*output);
	player.AddOutputSink(output);
	player.AddOutputSink(ndi);

	//auto input = iterator.CreateInput(*iterator[device_index], Core::VideoFormatType::pal_fha, 2, DecklinkTimecodeSource::VITC, false);
	auto input = std::make_shared<FFmpeg::FFmpegInput>("d:\\TEMP\\test5.mov", Core::HwAccel::none, "");
	input->SetIsLoop(true);

	input->Play();
	player.Load(input);
	std::getchar();
	player.RemoveOutputSink(ndi);
	player.RemoveOutputSink(output);
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

