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
		double _volume = 1.0f;
	public:
		Channel(VideoFormat^ videoFormat, PixelFormat pixelFormat, int audioChannelCount);
		Channel(int formatId, PixelFormat pixelFormat, int audioChannelCount);
		~Channel();
		!Channel();
		bool AddOutput(DecklinkDevice^ device);
		void Load(InputFile^ file);
		void Clear();
		property double Volume
		{
			double get() { return _volume; }
			void set(double volume) 
			{
				_volume = volume;
				_channel->SetVolume(volume);
			}
		}
	};
}
