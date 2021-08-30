#pragma once
#include "VideoFilterBase.h"
#include "../Core/VideoFormat.h"


namespace TVPlayR {
	namespace Core {
		class VideoFormat;
	}
	namespace FFmpeg {

class Decoder;

class ChannelScaler :
	public VideoFilterBase
{
public:
	ChannelScaler(Decoder& decoder, const Core::VideoFormat& output_format, const AVPixelFormat output_pixel_format);
	const Core::VideoFormat& Format() const { return output_format_; }
	bool Push(std::shared_ptr<AVFrame> frame);
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	Decoder& decoder_;
	std::string Setup(std::shared_ptr<AVFrame>& frame);
};

}}