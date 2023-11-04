#include "../pch.h"
#include "DecklinkInputSynchroProvider.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkUtils.h"
#include "../Core/Player.h"
#include "../Core/AVSync.h"

namespace TVPlayR {
	namespace Decklink {
		DecklinkInputSynchroProvider::DecklinkInputSynchroProvider(const Core::Player& player, bool process_video, int audio_channels)
			: PlayerSynchroSource(player, process_video, audio_channels)
			, executor_("DecklinkInputSynchroProvider for " + player.Name())
		{
		}

		DecklinkInputSynchroProvider::~DecklinkInputSynchroProvider()
		{
		}

		void DecklinkInputSynchroProvider::Push(const Core::AVSync& sync, AVRational frame_rate)
		{
			Core::AVSync copy(sync);
			executor_.begin_invoke([=]
				{
					PlayerSynchroSource::Push(copy, frame_rate);
				});
		}

}}
