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
	VideoFilter(AVRational input_time_base, AVPixelFormat output_pix_fmt);
	virtual ~VideoFilter();
	virtual bool Push(std::shared_ptr<AVFrame> frame);
	virtual std::shared_ptr<AVFrame> Pull();
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVRational OutputFrameRate();
	AVPixelFormat GetOutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	virtual void Flush() override;
	virtual bool IsFlushed() const override;
	virtual void Reset() override;
	virtual bool IsEof() const override;
protected:
	void SetFilter(int width, int height, AVPixelFormat pix_fmt, AVRational input_sar, const std::string& filter_string);
	bool IsInitialized() const;
	void CreateFilterChain();

private:
	class implementation;
	std::unique_ptr<implementation> impl_;

};
	
}}