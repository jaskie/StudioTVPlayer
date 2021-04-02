#pragma once
#include "../FFMpeg/VideoFilterBase.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	
	namespace Preview {

class PreviewScaler : public FFmpeg::VideoFilterBase
{
private:
	Core::Channel& channel_;
	const std::string filter_str_;
	std::string GetFilterString(const Core::VideoFormat& channel_format, int width, int height);
public:
	PreviewScaler(Core::Channel& channel, int width, int height);
	void Push(std::shared_ptr<AVFrame> frame);
protected:
	virtual void PushMoreFrames() override {};
};

}}
