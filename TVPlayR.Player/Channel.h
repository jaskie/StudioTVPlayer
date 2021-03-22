#pragma once
#include "Core/Channel.h"
#include "VideoFormat.h"
#include "PixelFormat.h"
#include "InputFile.h"
#include "DecklinkDevice.h"

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
		bool AddOutput(DecklinkDevice^ device);
		void Load(InputFile^ file);
		void Clear();
		property float Volume
		{
			float get() { return _channel->GetVolume(); }
			void set(float volume) { _channel->SetVolume(volume); }
		}
	};
}
