#pragma once
#include "../Core/PlayerSynchroSource.h"

namespace TVPlayR {
	namespace Decklink {

class DecklinkInputSynchroProvider final : public Core::PlayerSynchroSource
{
public:
	DecklinkInputSynchroProvider(const Core::Player& player, bool process_video, int audio_channels);
	~DecklinkInputSynchroProvider();
	void Push(Core::AVSync& sync) override;
private:
	Common::Executor	executor_;
};

}}
