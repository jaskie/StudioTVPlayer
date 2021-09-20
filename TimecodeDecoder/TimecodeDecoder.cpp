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

#include "Core/Channel.h"
#include "Decklink/DecklinkIterator.h"
#include "Decklink/DecklinkOutput.h"
#include "Decklink/DecklinkInput.h"
#include "Decklink/DecklinkInfo.h"

using namespace TVPlayR;

int main()
{
	Core::Channel channel("Channel 1", Core::VideoFormatType::v1080i5000, Core::PixelFormat::yuv422, 2);
	Decklink::DecklinkIterator iterator;
	int device_index = 1;
	for (size_t i = 0; i < iterator.Size(); i++)
		std::wcout << L"Device " << i << L": " << iterator[i]->GetDisplayName() << L" Model: " << iterator[i]->GetModelName() << std::endl;
	auto decklink_output = iterator.CreateOutput(*iterator[device_index]);
	channel.SetFrameClock(decklink_output);
	channel.AddOutput(decklink_output);
	auto input = iterator.CreateInput(*iterator[device_index], Core::VideoFormatType::v1080i5000, 2, Decklink::DecklinkTimecodeSource::RP188Any);
	input->Play();
	channel.Load(input);
	std::getchar();
}

