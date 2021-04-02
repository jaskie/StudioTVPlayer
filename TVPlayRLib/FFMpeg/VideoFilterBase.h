#pragma once
#include "FilterBase.h"

namespace TVPlayR {

	namespace FFmpeg {

class VideoFilterBase :
	public FilterBase
{
private:
public:
	VideoFilterBase(AVPixelFormat output_pix_fmt);
	virtual std::shared_ptr<AVFrame> Pull() override;
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat OutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	virtual void Flush() override;
	void Reset();
protected:
	bool Push(std::shared_ptr<AVFrame> frame);
	bool IsInitialized() const;
	virtual void Initialize() override;
	virtual void PushMoreFrames() = 0;
	void CreateFilterChain(std::shared_ptr<AVFrame> frame, const AVRational& input_time_base, const std::string& filter_str);
private:
	AVFilterContext* source_ctx_ = NULL;
	AVFilterContext* sink_ctx_ = NULL;
	const AVPixelFormat output_pix_fmt_;
};
	
}}