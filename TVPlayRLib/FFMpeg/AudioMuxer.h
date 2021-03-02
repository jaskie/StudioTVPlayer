#pragma once
#include "FilterBase.h"


namespace TVPlayR {
	namespace FFmpeg {
		
class Decoder;

class AudioMuxer :
	public FilterBase
{
public:
	AudioMuxer(const std::vector<std::unique_ptr<Decoder>>& decoders, const int64_t output_channel_layout, const AVSampleFormat sample_format, const int sample_rate, const int nb_channels);
	~AudioMuxer();
	int OutputSampleRate();
	int OutputChannelsCount();
	virtual AVRational OutputTimeBase() const override;
	uint64_t OutputChannelLayout();
	AVSampleFormat OutputSampleFormat();
	void Push(int stream_index, std::shared_ptr<AVFrame> frame);
	virtual std::shared_ptr<AVFrame> Pull() ;
	virtual void Flush() override;
	virtual void Reset() override;
	virtual bool IsEof() const override;
	virtual bool IsFlushed() const override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}