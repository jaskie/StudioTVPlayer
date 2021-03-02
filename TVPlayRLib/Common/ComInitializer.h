#pragma once

namespace TVPlayR {
	namespace Common {

class ComInitializer
{
private:
	const HRESULT m_hr;
public:
	ComInitializer()
	: m_hr(::CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED)) { }
	~ComInitializer() { if (SUCCEEDED(m_hr)) ::CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
};

}}