#pragma once
#include "FilterBase.h"

namespace TVPlayR {

	namespace FFmpeg {

class VideoFilter :
	public FilterBase
{
private:
public:
	typedef std::unique_ptr<VideoFilter> Ptr;
	VideoFilter(AVPixelFormat output_pix_fmt);
	virtual ~VideoFilter();
	virtual bool Push(std::shared_ptr<AVFrame> frame, const AVRational& time_base);
	virtual std::shared_ptr<AVFrame> Pull();
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVPixelFormat GetOutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	virtual void Flush() override;
	virtual bool IsFlushed() const override;
	virtual bool IsEof() const override;
protected:
	void SetFilter(int width, int height, AVPixelFormat pix_fmt, const AVRational& input_sar, const AVRational& input_time_base, const std::string& filter_string);
	bool IsInitialized() const;
	void CreateFilterChain(const AVRational& input_time_base);

private:
	class implementation;
	std::unique_ptr<implementation> impl_;

};
	
}}