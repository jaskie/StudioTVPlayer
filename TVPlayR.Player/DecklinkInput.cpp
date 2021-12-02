#include "stdafx.h"
#include "DecklinkInput.h"
#include "Decklink/DecklinkInput.h"
#include "InputPreview.h"
#include "VideoFormatEventArgs.h"

namespace TVPlayR {

	const std::shared_ptr<Decklink::DecklinkInput> DecklinkInput::GetDecklinkInput()
	{
		return std::dynamic_pointer_cast<Decklink::DecklinkInput>(InputBase::GetNativeSource());
	}

	void DecklinkInput::FormatChangedCallback(Core::VideoFormatType newFormat)
	{
		VideoFormat^ format = VideoFormat::FindFormat(newFormat);
		FormatChanged(this, gcnew VideoFormatEventArgs(format));
	}

	DecklinkInput::DecklinkInput(std::shared_ptr<Decklink::DecklinkInput> decklink, String^ modelName)
		: InputBase(decklink)
		, _modelName(modelName)
	{
		_formatChangedDelegate = gcnew DecklinkInput::FormatChangedDelegate(this, &DecklinkInput::FormatChangedCallback);
		_formatChangedHandle = GCHandle::Alloc(_formatChangedDelegate);
		IntPtr formatChangedIp = Marshal::GetFunctionPointerForDelegate(_formatChangedDelegate);
		decklink->SetFormatChangedCallback(static_cast<Decklink::FORMAT_CALLBACK>(formatChangedIp.ToPointer()));
	}

	void DecklinkInput::AddPreview(InputPreview^ preview)
	{
		GetDecklinkInput()->AddPreview(preview->GetNative());
	}

	void DecklinkInput::RemovePreview(InputPreview^ preview)
	{
		GetDecklinkInput()->RemovePreview(preview->GetNative());
	}

	DecklinkInput::~DecklinkInput()
	{
		this->!DecklinkInput();
	}

	DecklinkInput::!DecklinkInput()
	{
		if (!GetNativeSource())
			return;
		_formatChangedHandle.Free();
	}
}