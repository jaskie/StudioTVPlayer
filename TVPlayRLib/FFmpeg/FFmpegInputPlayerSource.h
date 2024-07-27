#pragma once
#include "../Core/PlayerSynchroSource.h"
namespace TVPlayR {
	namespace Core {
		class Player;
	}
	namespace FFmpeg {
class FFmpegInputPlayerSource final : public Core::PlayerSynchroSource, private Common::DebugTarget
{
public:
	FFmpegInputPlayerSource(const Core::Player &player, bool process_video, int audio_channels);
	void Push(const Core::AVSync &sync, AVRational frame_rate) override;
	Core::AVSync PullSync(int audio_samples_count) override;
};

}}
