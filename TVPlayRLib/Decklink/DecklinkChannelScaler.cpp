#include "../pch.h"
#include "DecklinkChannelScaler.h"
#include "../Core/Channel.h"
#include "../Core/PixelFormat.h"


namespace TVPlayR {
	namespace Decklink {

	DecklinkChannelScaler::DecklinkChannelScaler(const Core::Channel& channel)
		: VideoFilterBase(Core::PixelFormatToFFmpegFormat(channel.PixelFormat()))
	{

	}

	bool DecklinkChannelScaler::Push(std::shared_ptr<AVFrame> video)
	{
		return false;
	}
	

}
}
