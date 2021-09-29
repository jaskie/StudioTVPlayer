#pragma once
#include "VideoFilterBase.h"
#include "../Core/VideoFormat.h"


namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {

class Decoder;

class ChannelScaler final :	public VideoFilterBase
{
public:
	ChannelScaler(const Core::Channel& channel);
	const Core::VideoFormat& Format() const { return output_format_; }
	bool Push(std::shared_ptr<AVFrame> frame, AVRational time_base, AVRational frame_rate);
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	std::string Setup(std::shared_ptr<AVFrame>& frame, Common::Rational<int> time_base);
};

}}