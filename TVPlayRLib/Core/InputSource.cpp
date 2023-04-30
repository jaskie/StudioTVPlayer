#include "../pch.h"
#include "InputSource.h"

namespace TVPlayR {
	namespace Core {
		void InputSource::RaiseLoaded()
		{
			LOADED_CALLBACK callback = _loadedCallback;
			if (callback)
				callback();
		}

		void InputSource::SetLoadedCallback(LOADED_CALLBACK callback)
		{
			_loadedCallback = callback;
		}
	}
}