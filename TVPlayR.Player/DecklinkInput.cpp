#include "stdafx.h"
#include "DecklinkInput.h"
#include "Decklink/DecklinkInput.h"
#include "PreviewSink.h"
#include "VideoFormatEventArgs.h"
#include "FieldOrder.h"

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

	void DecklinkInput::AddPreview(PreviewSink^ preview)
	{
		GetDecklinkInput()->AddPreview(preview->GetNativeSink());
	}

	void DecklinkInput::RemovePreview(PreviewSink^ preview)
	{
		GetDecklinkInput()->RemovePreview(preview->GetNativeSink());
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