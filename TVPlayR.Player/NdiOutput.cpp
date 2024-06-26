#include "stdafx.h"
#include "NdiOutput.h"
#include "ClrStringHelper.h"
#include "OverlayBase.h"
#include "Ndi/NdiOutput.h"
#include "VideoFormat.h"
#include "Core/Player.h"
#include "Core/VideoFormat.h"

namespace TVPlayR {

	Ndi::NdiOutput* CreateNativeOutput(String^ sourceName, String^ groupNames)
	{
		REWRAP_EXCEPTION(return new Ndi::NdiOutput(ClrStringToStdString(sourceName), ClrStringToStdString(groupNames));)
	}


	NdiOutput::NdiOutput(String^ sourceName, String^ groupNames)
		: _ndi(new std::shared_ptr<Ndi::NdiOutput>(CreateNativeOutput(sourceName, groupNames)))
		, _sourceName(sourceName)
		, _groupNames(groupNames)
	{
	}

	NdiOutput::~NdiOutput()
	{
		this->!NdiOutput();
	}

	NdiOutput::!NdiOutput()
	{
		if (!_ndi)
			return;
		REWRAP_EXCEPTION(delete _ndi;)
		_ndi = nullptr;
	}

	void NdiOutput::AddOverlay(OverlayBase^ overlay)
	{
		if (!_ndi)
			return;
		REWRAP_EXCEPTION((*_ndi)->AddOverlay(overlay->GetNativeObject());)
	}

	void NdiOutput::RemoveOverlay(OverlayBase^ overlay)
	{
		if (!_ndi)
			return;
		REWRAP_EXCEPTION((*_ndi)->RemoveOverlay(overlay->GetNativeObject());)
	}

	void NdiOutput::Initialize(VideoFormat^ format, PixelFormat pixelFormat, int audioChannelsCount, int audioSampleRate)
	{
		REWRAP_EXCEPTION((*_ndi)->Initialize(format->GetNativeEnumType(), pixelFormat, audioChannelsCount, audioSampleRate);)
	}

	std::shared_ptr<Core::OutputDevice> NdiOutput::GetNativeDevice()
	{
		return _ndi == nullptr ? nullptr : *_ndi;
	}
	
	std::shared_ptr<Core::OutputSink> NdiOutput::GetNativeSink()
	{
		return _ndi == nullptr ? nullptr : *_ndi;
	}


}