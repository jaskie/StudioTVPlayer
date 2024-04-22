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
	AudioMuxer(const std::vector<std::unique_ptr<Decoder>> &decoders, const std::int64_t output_channel_layout, const Core::AudioParameters output_audio_parameters);
	int OutputSampleRate() const;
	int OutputChannelsCount() const;
	AVRational OutputTimeBase() const override;
	AVSampleFormat OutputSampleFormat() const;
	void Push(int stream_index, std::shared_ptr<AVFrame> frame);
	std::shared_ptr<AVFrame> Pull() override;
	void Flush() override;
	void Initialize(const std::vector<std::unique_ptr<Decoder>> &decoders);
private:
	std::string GetAudioMuxerString(const std::vector<std::unique_ptr<Decoder>> &decoders);
	std::vector<std::pair<int, unique_ptr<AVFilterContext>>> source_ctx_;
	const AVRational input_time_base_;
	AVRational output_time_base_;
	const std::int64_t output_channel_layout_;
	const Core::AudioParameters output_audio_parameters_;
	unique_ptr<AVFilterContext> sink_ctx_;
	unique_ptr<AVFilterInOut> inputs_;
	unique_ptr<AVFilterInOut> outputs_;
	std::string filter_str_;
};

}}