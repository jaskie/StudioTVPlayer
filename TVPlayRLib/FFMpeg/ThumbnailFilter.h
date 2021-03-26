#pragma once
#include "VideoFilterBase.h"

namespace TVPlayR {
	namespace FFmpeg {

class ThumbnailFilter :
	public VideoFilterBase
{
public:
	ThumbnailFilter(int height, std::shared_ptr<AVFrame> frame);
	virtual std::shared_ptr<AVFrame> Pull() override;
private:
	const std::shared_ptr<AVFrame> input_frame_;
	std::shared_ptr<AVFrame> result_frame_;
	const int height_;
};

}}

