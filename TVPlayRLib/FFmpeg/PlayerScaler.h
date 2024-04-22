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
	PlayerScaler(const Core::Player& player, const AVRational input_frame_rate);
	const Core::VideoFormat& Format() const { return output_format_; }
	const AVRational InputFrameRate() const { return  input_frame_rate_; }
protected:
	void Initialize(const std::shared_ptr<AVFrame> &frame) override;
private:
	const Core::VideoFormat output_format_;
	const AVPixelFormat output_pixel_format_;
	const AVRational input_frame_rate_;
	std::string GetFilterString(const std::shared_ptr<AVFrame> &frame);
};

}}