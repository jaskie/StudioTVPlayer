#pragma once

#define DebugPrintLineIf(condition, severity, message) \
if (condition) \
	DebugPrintLine(severity, message)

namespace TVPlayR {
	namespace Common {

enum class DebugSeverity
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

class DebugTarget
{
private:
	const std::string name_;
	const enum DebugSeverity severity_;
	inline void DebugPrint(const char *s)
	{
		OutputDebugStringA(s);
	}

protected:
	DebugTarget(enum DebugSeverity severity, const std::string name)
		: name_(name)
		, severity_(severity)
	{}

	inline void DebugPrintLine(enum DebugSeverity severity, const std::string &s)
	{
#ifdef DEBUG
		if (severity >= severity_)
		{
			DebugPrint((name_ + ": " + s + "\n").c_str());
		}
#endif // DEBUG
	}

	inline enum DebugSeverity DebugSeverity() const { return severity_; }
};

}
}
