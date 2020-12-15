#pragma once
#include "VideoFilter.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"


namespace TVPlayR {
	namespace Core {
		class InputSource;
	}
	namespace FFmpeg {

class OutputVideoFilter :
	public VideoFilter
{
public:
	OutputVideoFilter(const Core::InputSource& source, const Core::VideoFormat& format, const Core::PixelFormat pixel_format);
	virtual bool Push(AVFramePtr frame) override;
private:
	const Core::VideoFormat output_format_;
	const Common::Rational<int> input_frame_rate_;
	const Core::PixelFormat pixel_format_;
	std::string Setup(AVFramePtr& frame);
};

}}