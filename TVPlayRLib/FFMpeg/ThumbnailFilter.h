#pragma once
#include "VideoFilter.h"

namespace TVPlayR {
	namespace FFmpeg {

class ThumbnailFilter :
	public VideoFilter
{
public:
	ThumbnailFilter(int height, AVFramePtr frame);
	virtual AVFramePtr Pull() override;
private:
	const AVFramePtr thumbnail_frame_;
};

}}

