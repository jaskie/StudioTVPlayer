#pragma once
#include "VideoFilter.h"

namespace TVPlayR {
	namespace FFmpeg {

class ThumbnailFilter :
	public VideoFilter
{
public:
	ThumbnailFilter(int height, std::shared_ptr<AVFrame> frame);
	virtual std::shared_ptr<AVFrame> Pull() override;
private:
	const std::shared_ptr<AVFrame> thumbnail_frame_;
};

}}

