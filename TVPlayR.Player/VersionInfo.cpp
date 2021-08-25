#pragma once
#include "stdafx.h"
#include "Ndi/NdiUtils.h"
using namespace System;
using namespace System::Reflection;

namespace TVPlayR {

	public ref class VersionInfo sealed
	{
	public:
		static property String^ Wrapper 
		{
			String^ get() 
			{
				System::Version^ version = Assembly::GetExecutingAssembly()->GetName()->Version;
				return 	String::Format("{0}.{1}.{2}", version->Major, version->Minor, version->Build);
			}
		}

		static property String^ FFmpegAVFormat
		{
			String^ get() 
			{
				unsigned int version = avformat_version();
				return 	String::Format("{0}.{1}.{2}", version >> 16, (version >> 8) & 0xFF, version & 0xFF);
			}
		}

		static property String^ FFmpegAVCodec
		{
			String^ get() 
			{
				unsigned int version = avcodec_version();
				return 	String::Format("{0}.{1}.{2}", version >> 16, (version >> 8) & 0xFF, version & 0xFF);
			}
		}
		
		static property String^ FFmpegAVFilter
		{
			String^ get() 
			{
				unsigned int version = avfilter_version();
				return 	String::Format("{0}.{1}.{2}", version >> 16, (version >> 8) & 0xFF, version & 0xFF);
			}
		}

		static property String^ Ndi
		{
			String^ get()
			{
				NDIlib_v4* ndi = TVPlayR::Ndi::LoadNdi();
				if (!ndi)
					return "not found";				
				return gcnew String(ndi->version());
			}
		}

	};
}
