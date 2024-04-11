#include "../pch.h"
#include "DecklinkInputPlayerSource.h"
#include "../DecklinkTimecodeSource.h"
#include "DecklinkUtils.h"
#include "../Core/Player.h"
#include "../Core/AVSync.h"

namespace TVPlayR {
	namespace Decklink {
		DecklinkInputPlayerSource::DecklinkInputPlayerSource(const Core::Player &player, bool process_video, int audio_channels)
			: PlayerSynchroSource(player, process_video, audio_channels)
			, executor_("DecklinkInputPlayerSource for " + player.Name())
		{
		}

		DecklinkInputPlayerSource::~DecklinkInputPlayerSource()
		{
		}

		void DecklinkInputPlayerSource::Push(const Core::AVSync &sync, AVRational frame_rate)
		{
			Core::AVSync copy(sync);
			executor_.begin_invoke([=]
				{
					PlayerSynchroSource::Push(copy, frame_rate);
				});
		}

}}
