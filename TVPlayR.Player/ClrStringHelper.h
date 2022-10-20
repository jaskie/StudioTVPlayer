#pragma once
using namespace System;

namespace TVPlayR {

static std::string ClrStringToStdString(String ^str) 
{
	if (str == nullptr)
		return "";
	array<Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(str + "\0");
	pin_ptr<Byte> pinnedBytes = &bytes[0];
	return reinterpret_cast<char*>(pinnedBytes);
}

}