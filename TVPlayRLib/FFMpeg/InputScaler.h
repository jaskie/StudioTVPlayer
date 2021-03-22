#pragma once
#include "VideoFilter.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"



namespace TVPlayR {
	namespace FFmpeg {

class Decoder;

class InputScaler :
	public VideoFilter
{
public:
	InputScaler(const Decoder& decoder, const Core::VideoFormat& output_format, const AVPixelFormat output_pixel_format);
	bool Push(std::shared_ptr<AVFrame> frame);
	const Core::VideoFormat& Format() const { return output_format_; }
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	const Decoder& decoder_;
	std::string Setup(std::shared_ptr<AVFrame>& frame);
};

}}