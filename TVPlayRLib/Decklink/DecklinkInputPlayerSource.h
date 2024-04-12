#pragma once
#include "../Core/PlayerSynchroSource.h"

namespace TVPlayR {
	namespace Decklink {

class DecklinkInputPlayerSource final : public Core::PlayerSynchroSource
{
public:
	DecklinkInputPlayerSource(const Core::Player &player, bool process_video, int audio_channels);
	~DecklinkInputPlayerSource();
	void Push(const Core::AVSync &sync, AVRational frame_rate) override;
private:
	Common::Executor executor_;
};

}}
