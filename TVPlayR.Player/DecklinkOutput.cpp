#include "stdafx.h"
#include "DecklinkOutput.h"
#include "Decklink/Iterator.h"
#include "Decklink/Decklink.h"



namespace TVPlayR {

	DecklinkOutput::DecklinkOutput(const int index, std::shared_ptr<Decklink::Decklink>& decklink)
		: _index(index)
		, _decklink(new std::shared_ptr<Decklink::Decklink>(decklink))
	{
	}

	DecklinkOutput::~DecklinkOutput()
	{
		this->!DecklinkOutput();
	}

	DecklinkOutput::!DecklinkOutput()
	{
		if (!_decklink)
			return;
		delete _decklink;
		_decklink = nullptr;
	}

	array<DecklinkOutput^>^ DecklinkOutput::EnumerateDevices()
	{
		if (_devices == nullptr)
		{
			Decklink::Iterator iterator;
			iterator.Refresh();
			int count = static_cast<int> (iterator.Size());
			_devices = gcnew array<DecklinkOutput^>(count);
			for (int i = 0; i < count; i++)
				_devices[i] = gcnew DecklinkOutput(i, iterator[i]);
		}
		return _devices;
	}

}