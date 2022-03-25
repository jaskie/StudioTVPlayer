#pragma once
#include "VideoFilterBase.h"
#include "../Core/VideoFormat.h"


namespace TVPlayR {
	namespace Core {
		class Player;
	}
	namespace FFmpeg {

class Decoder;

class PlayerScaler final :	public VideoFilterBase
{
public:
	PlayerScaler(const Core::Player& player);
	const Core::VideoFormat& Format() const { return output_format_; }
	bool Push(std::shared_ptr<AVFrame> frame, AVRational input_frame_rate, AVRational input_time_base);
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	std::string GetFilterString(std::shared_ptr<AVFrame>& frame, Common::Rational<int> input_frame_rate);
};

}}