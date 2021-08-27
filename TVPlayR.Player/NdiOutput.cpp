#include "stdafx.h"
#include "NdiOutput.h"
#include "ClrStringHelper.h"

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


}