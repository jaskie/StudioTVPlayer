#pragma once
#include "../FFmpeg/VideoFilterBase.h"
#include "../Core/PixelFormat.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
		class VideoFormat;
	}
	
	namespace Preview {

class PreviewScaler : public FFmpeg::VideoFilterBase
{
private:
	Core::Channel& channel_;
	const std::string filter_str_;
	std::string GetFilterString(const Core::VideoFormat& channel_format, const Core::PixelFormat pix_fmt, int width, int height);
public:
	PreviewScaler(Core::Channel& channel,  int width, int height);
	void Push(std::shared_ptr<AVFrame> frame);
};

}}
