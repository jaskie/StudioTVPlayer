#pragma once


namespace TVPlayR {
	namespace Common {
class TVPlayRException : public std::exception
{
	using std::exception::exception;
public:
	TVPlayRException(const std::string& message) : TVPlayRException(message.c_str()) { }
};

}}

#define THROW_EXCEPTION(error_string) {\
std::string exception_message = std::string(error_string) + std::string(" in ") + std::string(__FUNCTION__) + std::string(" at ") + std::string(__FILE__) + std::string(" in line ") + std::to_string(__LINE__);\
OutputDebugStringA(exception_message.c_str());\
throw TVPlayR::Common::TVPlayRException(exception_message); }