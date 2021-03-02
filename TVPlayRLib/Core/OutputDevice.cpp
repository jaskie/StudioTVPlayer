#include "../pch.h"
#include "OutputDevice.h"
#include "Channel.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace Core {

bool OutputDevice::AssignToChannel(Channel& channel)
{
	if (IsPlaying())
		return false;
	return true;
}

}}