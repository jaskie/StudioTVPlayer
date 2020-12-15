#pragma once

namespace TVPlayR {
	namespace Common {

class NonCopyable
{
private:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
protected:
	NonCopyable() = default;
};

}}