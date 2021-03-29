#pragma once
#include "VideoFilterBase.h"
#include "../Common/Rational.h"

namespace TVPlayR {
	namespace FFmpeg {

class PreviewScaler : public VideoFilterBase
{
private:
	const Common::Rational<int> channel_frame_rate_;
	const std::string filter_str_;
	std::string GetFilterString(const Core::VideoFormat& channel_format, int width, int height);
public:
	PreviewScaler(const Core::VideoFormat& channel_format, int width, int height);
	void Push(std::shared_ptr<AVFrame> frame);
};

}}
