#pragma once

#include "OutputBase.h"
#include "Decklink/Decklink.h"

using namespace System;

namespace TVPlayR {

	public ref class DecklinkOutput : public OutputBase
	{
	private:
		const int _index;
		const std::shared_ptr<Decklink::Decklink>* _decklink;
		DecklinkOutput(const int index, std::shared_ptr<Decklink::Decklink>& decklink);
		static array<DecklinkOutput^>^ _devices;

	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_decklink; }

	public:
		~DecklinkOutput();
		!DecklinkOutput();

		static array<DecklinkOutput^>^ EnumerateDevices();

		property int Index
		{
			int get() { return _index; }
		}

		property System::String^ DisplayName {
			System::String^ get() 
			{
				return gcnew System::String((*_decklink)->GetDisplayName().c_str());
			}
		}

		property System::String^ ModelName {
			System::String^ get() 
			{
				return gcnew System::String((*_decklink)->GetModelName().c_str());
			}
		}
	};

}
