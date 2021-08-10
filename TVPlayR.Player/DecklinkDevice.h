#pragma once
#include "OutputDevice.h"
#include "Decklink/Decklink.h"

using namespace System;

namespace TVPlayR {

	public ref class DecklinkDevice : public OutputDevice
	{
	private:
		const int _index;
		const std::shared_ptr<Decklink::Decklink>* _decklink;
		DecklinkDevice(const int index, std::shared_ptr<Decklink::Decklink>& decklink);
		static array<DecklinkDevice^>^ _devices;

	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_decklink; }

	public:
		~DecklinkDevice();
		!DecklinkDevice();

		static array<DecklinkDevice^>^ EnumerateDevices();

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
