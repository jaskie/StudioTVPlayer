#include "stdafx.h"
#include "DecklinkDevice.h"
#include "Decklink/Iterator.h"
#include "Decklink/Decklink.h"



namespace TVPlayR {

	DecklinkDevice::DecklinkDevice(const int index, std::shared_ptr<Decklink::Decklink>& decklink)
		: _index(index)
		, _decklink(new std::shared_ptr<Decklink::Decklink>(decklink))
	{
	}

	DecklinkDevice::~DecklinkDevice()
	{
		this->!DecklinkDevice();
	}

	DecklinkDevice::!DecklinkDevice()
	{
		delete _decklink;
	}

	array<DecklinkDevice^>^ DecklinkDevice::EnumerateDevices()
	{
		if (_devices == nullptr)
		{
			Decklink::Iterator iterator;
			int count = static_cast<int> (iterator.Size());
			_devices = gcnew array<DecklinkDevice^>(count);
			for (int i = 0; i < count; i++)
				_devices[i] = gcnew DecklinkDevice(i, iterator[i]);
		}
		return _devices;
	}

}