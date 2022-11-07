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
		REWRAP_EXCEPTION(FormatChanged(this, gcnew VideoFormatEventArgs(format));)
	}

	DecklinkInput::DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink, String^ modelName)
		: InputBase(std::dynamic_pointer_cast<Core::InputSource>(decklink))
		, _modelName(modelName)
	{
		_formatChangedDelegate = gcnew DecklinkInput::FormatChangedDelegate(this, &DecklinkInput::FormatChangedCallback);
		_formatChangedHandle = GCHandle::Alloc(_formatChangedDelegate);
		IntPtr formatChangedIp = Marshal::GetFunctionPointerForDelegate(_formatChangedDelegate);
		decklink->SetFormatChangedCallback(static_cast<Decklink::FORMAT_CALLBACK>(formatChangedIp.ToPointer()));
	}

	void DecklinkInput::AddOutputSink(OutputSink^ preview)
	{
		REWRAP_EXCEPTION(GetDecklinkInput()->AddOutputSink(preview->GetNativeSink());)
	}

	void DecklinkInput::RemoveOutputSink(OutputSink^ output_sink)
	{
		REWRAP_EXCEPTION(GetDecklinkInput()->RemoveOutputSink(output_sink->GetNativeSink());)
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