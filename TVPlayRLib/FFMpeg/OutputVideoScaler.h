#pragma once
#include "VideoFilter.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"


namespace TVPlayR {
	namespace Core {
		class InputSource;
	}
	namespace FFmpeg {

class OutputVideoScaler :
	public VideoFilter
{
public:
	OutputVideoScaler(AVRational input_frame_rate, AVRational input_time_base, const Core::VideoFormat& output_format, const AVPixelFormat output_pixel_format);
	virtual bool Push(std::shared_ptr<AVFrame> frame) override;
	const Core::VideoFormat& Format() const { return output_format_; }
private:
	const Core::VideoFormat output_format_;
	const Common::Rational<int> input_frame_rate_;
	const AVRational input_time_base_;
	const AVRational output_time_base_;
	const AVPixelFormat pixel_format_;
	std::string Setup(std::shared_ptr<AVFrame>& frame);
};

}}