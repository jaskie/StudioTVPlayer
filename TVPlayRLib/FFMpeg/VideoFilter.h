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
	VideoFilter(AVRational input_frame_rate, AVRational input_time_base, AVPixelFormat output_pix_fmt);
	virtual ~VideoFilter();
	virtual bool Push(AVFramePtr frame);
	virtual AVFramePtr Pull();
	int OutputWidth();
	int OutputHeight();
	AVRational OutputSampleAspectRatio();
	AVRational OutputFrameRate();
	AVPixelFormat GetOutputPixelFormat();
	virtual AVRational OutputTimeBase() const override;
	virtual void Flush() override;
	bool IsFlushed() const;
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