#pragma once
using namespace System;

namespace TVPlayR {

static std::string ClrStringToStdString(String ^str) 
{
	if (str == nullptr)
		return "";
	IntPtr ansiStr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
	std::string outString((const char*)ansiStr.ToPointer());
	Runtime::InteropServices::Marshal::FreeHGlobal(ansiStr);
	return outString;
}

}