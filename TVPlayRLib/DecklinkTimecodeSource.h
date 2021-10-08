#pragma once

namespace TVPlayR {
#if (_MANAGED == 1)
	public 
#endif
		enum class DecklinkTimecodeSource
	{
		None,
		RP188VITC1,
		RP188VITC2,
		RP188LTC,
		RP188Any,
		VITC,
		VITCField2,
		Serial
	};
}