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
	VideoFilterBase(AVPixelFormat output_pix_fmt);
	std::shared_ptr<AVFrame> Pull() override;
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat OutputPixelFormat();
	AVRational OutputTimeBase() const override;
	AVRational OutputFrameRate() const;
	void Flush() override;
	void Clear();
	bool IsInitialized() const;
	void Push(const std::shared_ptr<AVFrame>& frame);
protected:
	void SetFilter(const std::string &filter_str, const AVRational input_time_base );
private:
	std::string filter_;
	AVFilterContext* source_ctx_ = NULL;
	AVFilterContext* sink_ctx_ = NULL;
	const AVPixelFormat output_pix_fmt_;
	AVRational output_time_base_ = { 0, 1 };
	int input_width_ = 0;
	int input_height_ = 0;
	AVPixelFormat input_pixel_format_ = AV_PIX_FMT_NONE;
	AVRational input_time_base_ = { 0, 1 };
	AVRational input_sar_ = { 1, 1 };
	std::mutex frame_queue_mutex_;
	std::deque<std::shared_ptr<AVFrame>> frame_buffer_;
	void CreateFilter(int input_width, int input_height, AVPixelFormat input_pixel_format, const AVRational input_sar);
	bool PushFrameFromBuffer();
};

}}