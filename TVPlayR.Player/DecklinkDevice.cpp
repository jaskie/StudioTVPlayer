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
		static Decklink::Iterator iterator;
		int size = static_cast<int> (iterator.Size());
		array<DecklinkDevice^>^ result = gcnew array<DecklinkDevice^>(size);
		for (int i = 0; i < size; i++)
		{
			result[i] = gcnew DecklinkDevice(i, iterator[i]);
		}
		return result;
	}

}