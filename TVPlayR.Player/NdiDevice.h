#pragma once
#include "Ndi/Ndi.h"
using namespace System;

namespace TVPlayR {

	public ref class NdiDevice 
	{
	private:
		Ndi::Ndi* const _ndi;
		String^ _sourceName;
		String^ _groupName;		
	public:
		NdiDevice(String^ sourceName, String^ groupName);
		!NdiDevice();
		~NdiDevice();
		property String^ SourceName
		{
			String^ get() { return _sourceName; }
		}
		property String^ GroupName
		{
			String^ get() { return _groupName; }
		}
	internal:
		virtual Ndi::Ndi& GetNativeDevice() { return *_ndi; }
	};

}