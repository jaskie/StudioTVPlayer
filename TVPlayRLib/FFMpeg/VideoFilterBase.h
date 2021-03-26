#pragma once
#include "FilterBase.h"

namespace TVPlayR {

	namespace FFmpeg {

class VideoFilterBase :
	public FilterBase
{
private:
public:
	typedef std::unique_ptr<VideoFilterBase> Ptr;
	VideoFilterBase(AVPixelFormat output_pix_fmt);
	virtual ~VideoFilterBase();
	virtual std::shared_ptr<AVFrame> Pull();
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat GetOutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	virtual void Flush() override;
	virtual bool IsFlushed() const override;
	virtual bool IsEof() const override;
	virtual void Reset() override;

protected:
	bool Push(std::shared_ptr<AVFrame> frame);
	bool IsInitialized() const;
	void CreateFilterChain(std::shared_ptr<AVFrame> frame, const AVRational& input_time_base, const std::string& filter_str);

private:
	class implementation;
	std::unique_ptr<implementation> impl_;

};
	
}}