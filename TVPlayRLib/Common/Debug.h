#pragma once

#define DebugPrintIf(c, s) \
if (c) \
	DebugPrint(s)

namespace TVPlayR {
	namespace Common {

template <const bool debug_output> class DebugTarget
{
protected:

	inline void DebugPrint(const std::string& s)
	{
		DebugPrint(s.c_str());
	}

	inline void DebugPrint(const char * s)
	{
#ifdef DEBUG
		if (debug_output)
			OutputDebugStringA(s);
#endif // DEBUG
	}
};

}	
}
