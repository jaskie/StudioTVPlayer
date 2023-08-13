#pragma once
#include "FilterBase.h"

namespace TVPlayR {
	namespace Core {
		struct AudioParameters;
	}

	namespace FFmpeg {
		
class Decoder;

class AudioMuxer final : public FilterBase, private Common::DebugTarget
{
public:
	AudioMuxer(const std::vector<std::unique_ptr<Decoder>>& decoders, const std::int64_t output_channel_layout, const Core::AudioParameters output_audio_parameters);
	int OutputSampleRate();
	int OutputChannelsCount();
	AVRational OutputTimeBase() const override;
	AVSampleFormat OutputSampleFormat() const;
	void Push(int stream_index, std::shared_ptr<AVFrame> frame);
	std::shared_ptr<AVFrame> Pull() override;
	void Flush() override;
	void Reset();
private:
	std::string GetAudioMuxerString();
	void Initialize();
	const std::vector<std::unique_ptr<Decoder>>& decoders_;
	std::vector<std::pair<int, AVFilterContext*>> source_ctx_;
	const AVRational input_time_base_;
	const std::int64_t output_channel_layout_;
	const Core::AudioParameters output_audio_parameters_;
	AVFilterContext* sink_ctx_ = NULL;
	const std::string filter_str_;
};

}}