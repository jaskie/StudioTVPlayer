#pragma once

#include "../FFMpeg/VideoFilterBase.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace Decklink {

class DecklinkChannelScaler: public FFmpeg::VideoFilterBase
{
public:
	DecklinkChannelScaler(const Core::Channel& channel);
	bool Push(std::shared_ptr<AVFrame> video);
private:
};

}}
