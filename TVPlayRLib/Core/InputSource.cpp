#include "../pch.h"
#include "InputSource.h"

namespace TVPlayR {
	namespace Core {
		void InputSource::RaiseIsActiveOnPlayer()
		{
			ACTIVE_ON_PLAYER_CALLBACK callback = _activeOnPlayerCallback;
			if (callback)
				callback();
		}

		void InputSource::SetIsActiveOnPlayerCallback(ACTIVE_ON_PLAYER_CALLBACK callback)
		{
			_activeOnPlayerCallback = callback;
		}
	}
}