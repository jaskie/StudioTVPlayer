#pragma once
#include "FilterBase.h"


namespace TVPlayR {
	namespace Common {
		template <typename> class Rational;
	}

	namespace FFmpeg {

class VideoFilterBase :	public FilterBase, protected Common::DebugTarget
{
public:
	std::shared_ptr<AVFrame> Pull() override;
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat OutputPixelFormat();
	AVRational OutputTimeBase() const override;
	AVRational OutputFrameRate() const;
	void Flush() override;
	bool IsInitialized() const;
	void Push(const std::shared_ptr<AVFrame>& frame);
protected:
	VideoFilterBase(AVPixelFormat output_pix_fmt, const std::string &name);
	void SetFilter(const std::string &filter_str);
	virtual void Initialize(const std::shared_ptr<AVFrame> &frame);
	void ClearFilter();
private:
	std::string filter_;
	unique_ptr<AVFilterContext> source_ctx_;
	unique_ptr<AVFilterContext> sink_ctx_;
	const AVPixelFormat output_pix_fmt_;
	int input_width_ = 0;
	int input_height_ = 0;
	AVPixelFormat input_pixel_format_ = AV_PIX_FMT_NONE;
	AVRational input_time_base_ = { 0, 1 };
	AVRational input_sar_ = { 1, 1 };
	std::mutex frame_queue_mutex_;
	std::deque<std::shared_ptr<AVFrame>> frame_buffer_;
	bool PushFrameFromBuffer();
};

}}