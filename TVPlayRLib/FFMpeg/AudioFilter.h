#pragma once
#include "FilterBase.h"


namespace TVPlayR {
	namespace FFmpeg {
		
class Decoder;

class AudioFilter :
	public FilterBase
{
public:
	AudioFilter(const std::vector<std::unique_ptr<Decoder>>& decoders, const int64_t output_channel_layout, const AVSampleFormat sample_format, const int sample_rate, const int nb_channels);
	~AudioFilter();
	int OutputSampleRate();
	int OutputChannelsCount();
	virtual AVRational OutputTimeBase() const override;
	uint64_t OutputChannelLayout();
	AVSampleFormat OutputSampleFormat();
	void Push(int stream_index, AVFramePtr frame);
	virtual AVFramePtr Pull(int audio_samples_count) ;
	virtual void Flush() override;
	virtual void Reset() override;
	virtual bool IsEof() const override;
private:
	class implementation;
	std::unique_ptr<implementation> impl_;
};

}}