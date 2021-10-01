#include "stdafx.h"
#include "DecklinkInput.h"
#include "Decklink/DecklinkInput.h"
#include "InputPreview.h"
#include "VideoFormatEventArgs.h"

namespace TVPlayR {

	void DecklinkInput::FormatChangedCallback(Core::VideoFormatType newFormat)
	{
		VideoFormat^ format = VideoFormat::FindFormat(newFormat);
		FormatChanged(this, gcnew VideoFormatEventArgs(format));
	}

	DecklinkInput::DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink, String^ modelName)
		: _decklink(new std::shared_ptr<Decklink::DecklinkInput>(decklink))
		, _modelName(modelName)
	{
		_formatChangedDelegate = gcnew DecklinkInput::FormatChangedDelegate(this, &DecklinkInput::FormatChangedCallback);
		_formatChangedHandle = GCHandle::Alloc(_formatChangedDelegate);
		IntPtr formatChangedIp = Marshal::GetFunctionPointerForDelegate(_formatChangedDelegate);
		decklink->SetFormatChangedCallback(static_cast<Decklink::FORMAT_CALLBACK>(formatChangedIp.ToPointer()));
	}

	void DecklinkInput::AddPreview(InputPreview^ preview)
	{
		(*_decklink)->AddPreview(preview->GetNative());
	}

	void DecklinkInput::RemovePreview(InputPreview^ preview)
	{
		(*_decklink)->RemovePreview(preview->GetNative());
	}

	DecklinkInput::~DecklinkInput()
	{
		this->!DecklinkInput();
	}

	DecklinkInput::!DecklinkInput()
	{
		if (!_decklink)
			return;
		delete _decklink;
		_decklink = nullptr;
		_formatChangedHandle.Free();
	}
}