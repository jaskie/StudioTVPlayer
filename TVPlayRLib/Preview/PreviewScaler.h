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
	const std::string filter_str_;
	const AVRational time_base_;
	std::string GetFilterString(int output_width, int output_height);
public:
	PreviewScaler(AVRational input_frame_rate, int output_width, int output_height);
	void Push(std::shared_ptr<AVFrame> frame);
};

}}
