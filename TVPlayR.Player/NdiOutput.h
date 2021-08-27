#pragma once

#include "OutputBase.h"
#include "Ndi/NdiOutput.h"

using namespace System;

namespace TVPlayR {

	public ref class NdiOutput : public OutputBase
	{
	private:
		const std::shared_ptr<Ndi::NdiOutput>* _ndi;
		String^ _sourceName;
		String^ _groupNames;		
	public:
		NdiOutput(String^ sourceName, String^ groupNames);
		~NdiOutput();
		!NdiOutput();
		property String^ SourceName
		{
			String^ get() { return _sourceName; }
		}
		property String^ GroupNames
		{
			String^ get() { return _groupNames; }
		}
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_ndi; }
	};

}