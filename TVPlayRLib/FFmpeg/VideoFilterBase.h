#pragma once
#include "FilterBase.h"


namespace TVPlayR {
	namespace Common {
		template <typename> class Rational;
	}

	namespace FFmpeg {

class VideoFilterBase :	public FilterBase, Common::DebugTarget
{
public:
	VideoFilterBase(AVPixelFormat output_pix_fmt);
	virtual std::shared_ptr<AVFrame> Pull() override;
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat OutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	AVRational OutputFrameRate() const;
	virtual void Flush() override;
	void Reset();
	bool IsInitialized() const;
protected:
	bool Push(std::shared_ptr<AVFrame> frame);
	void SetFilter(const std::string& filter_str, const AVRational input_time_base );
private:
	std::string filter_;
	AVFilterContext* source_ctx_ = NULL;
	AVFilterContext* sink_ctx_ = NULL;
	const AVPixelFormat output_pix_fmt_;
	int input_width_ = 0;
	int input_height_ = 0;
	AVPixelFormat input_pixel_format_ = AV_PIX_FMT_NONE;
	AVRational input_time_base_ = av_make_q(1, 1);
	AVRational input_sar_ = av_make_q(1, 1);
	void CreateFilter(int input_width, int input_height, AVPixelFormat input_pixel_format, const AVRational input_sar);
};
	
}}