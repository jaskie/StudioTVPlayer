#pragma once
#include "VideoFilterBase.h"
#include "../Core/VideoFormat.h"


namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {

class Decoder;

class ChannelScaler :
	public VideoFilterBase
{
public:
	ChannelScaler(const Core::Channel& channel);
	const Core::VideoFormat& Format() const { return output_format_; }
	bool Push(std::shared_ptr<AVFrame> frame, AVRational time_base, AVRational frame_rate);
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	std::string Setup(std::shared_ptr<AVFrame>& frame, Common::Rational<int> time_base);
	int current_width_ = 0; 
	int current_height_ = 0;
	Common::Rational<int> current_frame_rate_, current_time_base_;
	AVPixelFormat current_pixel_fomat_ = AVPixelFormat::AV_PIX_FMT_NONE;
};

}}