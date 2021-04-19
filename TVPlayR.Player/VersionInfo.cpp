#pragma once
#include "stdafx.h"
using namespace System;
using namespace System::Reflection;

namespace TVPlayR {

	public ref class VersionInfo sealed
	{
	public:
		static property String^ WrapperVersion 
		{
			String^ get() 
			{
				System::Version^ version = Assembly::GetExecutingAssembly()->GetName()->Version;
				return 	String::Format("{0}.{1}.{2}", version->Major, version->Minor, version->Build);
			}
		}
		static property String^ FFmpegAVFormatVersion 
		{
			String^ get() 
			{
				unsigned int version = avformat_version();
				return 	String::Format("{0}.{1}.{2}", version >> 16, (version >> 8) & 0xFF, version & 0xFF);
			}
		}
	};
}
