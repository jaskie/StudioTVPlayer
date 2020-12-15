#pragma once
#include "Core/Channel.h"
#include "VideoFormat.h"
#include "PixelFormat.h"
#include "OutputDevice.h"
#include "InputFile.h"

using namespace System;

namespace TVPlayR {

	public ref class Channel
	{
	private:
		Core::Channel* _channel;
	public:
		Channel(VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount);
		Channel(int formatId, PixelFormat pixelFormat, int audioChannelCount);
		~Channel();
		!Channel();
		bool AddOutput(OutputDevice^ device);
		void Load(InputFile^ file);
		void Clear();
	};
}
