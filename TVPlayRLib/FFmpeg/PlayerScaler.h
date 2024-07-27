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
	const AVRational GetInputFrameRate() const { return  input_frame_rate_; }
	void SetInputFrameRate(const AVRational input_frame_rate);
protected:
	void Initialize(const std::shared_ptr<AVFrame> &frame) override;
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	AVRational input_frame_rate_ = { 0, 1 };
	std::string GetFilterString(const std::shared_ptr<AVFrame> &frame, Common::Rational<int> input_frame_rate);
};

}}