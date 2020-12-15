#pragma once

namespace TVPlayR {
	namespace Common {

class ComInitializer
{
private:
	HRESULT m_hr;
public:
	ComInitializer()
	: m_hr(::CoInitialize(nullptr)) { }
	~ComInitializer() { if (SUCCEEDED(m_hr)) ::CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
};

}}