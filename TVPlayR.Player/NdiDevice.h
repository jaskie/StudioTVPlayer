#pragma once
#include "OutputDevice.h"
#include "Ndi/Ndi.h"
using namespace System;

namespace TVPlayR {

	public ref class NdiDevice :
		public OutputDevice
	{
	private:
		std::shared_ptr<Ndi::Ndi>* _ndi;
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
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_ndi; }
	};

}