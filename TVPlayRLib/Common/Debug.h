#pragma once

#define DebugPrintLineIf(c, s) \
if (c) \
	DebugPrintLine(s)

namespace TVPlayR {
	namespace Common {

class DebugTarget
{
private:
	const std::string name_;
	bool debug_output_;
	inline void DebugPrint(const char* s)
	{
			OutputDebugStringA(s);
	}

protected:
	DebugTarget(bool debug_output, const std::string name)
		: name_(name)
		, debug_output_(debug_output)
	{}

	inline void DebugPrintLine(const std::string& s)
	{
#ifdef DEBUG
		if (debug_output_)
		{
			DebugPrint((name_ + ": " + s + "\n").c_str());
		}
#endif // DEBUG
	}


	inline bool IsDebugOutput() const { return debug_output_; }
};

}	
}
