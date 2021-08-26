#pragma once

#define DebugPrintIf(c, s) \
if (c) \
	DebugPrintLine(s)

namespace TVPlayR {
	namespace Common {

template <const bool debug_output = false> class DebugTarget
{
protected:

	inline void DebugPrintLine(const std::string& s)
	{
		DebugPrintLine(s.c_str());
	}

	inline void DebugPrintLine(const char * s)
	{
#ifdef DEBUG
		if (debug_output)
		{
			OutputDebugStringA(s);
			OutputDebugStringA("\n");
		}
#endif // DEBUG
	}

	inline bool IsDebugOutput() const { return debug_output; }
};

}	
}
