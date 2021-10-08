#pragma once
#include "../FFmpeg/VideoFilterBase.h"
#include "../PixelFormat.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
		class VideoFormat;
	}
	
	namespace Preview {

class PreviewScaler final : public FFmpeg::VideoFilterBase
{
private:
	const std::string filter_str_;
	std::string GetFilterString(int output_width, int output_height);
public:
	PreviewScaler(int output_width, int output_height);
	void Push(std::shared_ptr<AVFrame> frame);
};

}}
