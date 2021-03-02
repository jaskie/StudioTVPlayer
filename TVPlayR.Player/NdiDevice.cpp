#include "stdafx.h"
#include "NdiDevice.h"
#include "ClrStringHelper.h"

namespace TVPlayR {

	NdiDevice::NdiDevice(String^ sourceName, String^ groupName)
		: _ndi(new Ndi::Ndi(ClrStringToStdString(sourceName), ClrStringToStdString(groupName)))
		, _sourceName(sourceName)
		, _groupName(groupName)
	{
	}

	NdiDevice::!NdiDevice()
	{
		delete _ndi;
	}

	NdiDevice::~NdiDevice()
	{
		this->!NdiDevice();
	}

}