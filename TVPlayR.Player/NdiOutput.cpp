#include "stdafx.h"
#include "NdiOutput.h"
#include "ClrStringHelper.h"
#include "OverlayBase.h"
#include "Ndi/NdiOutput.h"

namespace TVPlayR {

	NdiOutput::NdiOutput(String^ sourceName, String^ groupNames)
		: _ndi(new std::shared_ptr<Ndi::NdiOutput>(new Ndi::NdiOutput(ClrStringToStdString(sourceName), ClrStringToStdString(groupNames))))
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
		delete _ndi;
		_ndi = nullptr;
	}

	void NdiOutput::AddOverlay(OverlayBase^ overlay)
	{
		if (!_ndi)
			return;
		(*_ndi)->AddOverlay(overlay->GetNativeObject());
	}

	void NdiOutput::RemoveOverlay(OverlayBase^ overlay)
	{
		if (!_ndi)
			return;
		(*_ndi)->RemoveOverlay(overlay->GetNativeObject());
	}

	std::shared_ptr<Core::OutputDevice> NdiOutput::GetNativeDevice()
	{
		return _ndi == nullptr ? nullptr : *_ndi;
	}


}