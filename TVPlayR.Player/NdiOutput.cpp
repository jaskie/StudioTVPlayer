#include "stdafx.h"
#include "NdiOutput.h"
#include "ClrStringHelper.h"

namespace TVPlayR {

	NdiOutput::NdiOutput(String^ sourceName, String^ groupName)
		: _ndi(new std::shared_ptr<Ndi::NdiOutput>(new Ndi::NdiOutput(ClrStringToStdString(sourceName), ClrStringToStdString(groupName))))
		, _sourceName(sourceName)
		, _groupName(groupName)
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


}