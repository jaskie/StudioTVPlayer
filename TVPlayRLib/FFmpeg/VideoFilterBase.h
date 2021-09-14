#pragma once
#include "FilterBase.h"
#include "../Common/Debug.h"

namespace TVPlayR {
	namespace Common {
		template <typename> class Rational;
	}

	namespace FFmpeg {

class VideoFilterBase :
	public FilterBase, Common::DebugTarget
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
	bool IsInitialized() const;
protected:
	bool Push(std::shared_ptr<AVFrame> frame);
	void CreateFilterChain(std::shared_ptr<AVFrame> frame, const Common::Rational<int> input_time_base, const std::string& filter_str);
private:
	AVFilterContext* source_ctx_ = NULL;
	AVFilterContext* sink_ctx_ = NULL;
	const AVPixelFormat output_pix_fmt_;
};
	
}}