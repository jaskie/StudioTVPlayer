#pragma once

#include "OutputBase.h"
#include "Ndi/Ndi.h"

using namespace System;

namespace TVPlayR {

	public ref class NdiOutput : public OutputBase
	{
	private:
		const std::shared_ptr<Ndi::Ndi>* _ndi;
		String^ _sourceName;
		String^ _groupName;		
	public:
		NdiOutput(String^ sourceName, String^ groupName);
		~NdiOutput();
		!NdiOutput();
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