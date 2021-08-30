#pragma once

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace FFmpeg {
		class AVSync;
	}
	namespace Decklink {
		class DecklinkChannelScaler;

class DecklinkSynchroProvider
{
public:
	DecklinkSynchroProvider(const Core::Channel* channel);
	~DecklinkSynchroProvider();
	const Core::Channel* Channel() const;
	void Push(IDeckLinkVideoInputFrame* videoFrame, BMDFieldDominance fieldDominance, IDeckLinkAudioInputPacket* audioPacket);
	FFmpeg::AVSync PullSync(int audioSamplesCount);
private:
	const Core::Channel* channel_;
	//std::unique_ptr<DecklinkChannelScaler> scaler_;
	std::shared_ptr<AVFrame> last_video_;
	int64_t frame_pts_;
};

}}
