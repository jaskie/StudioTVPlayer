#include "../pch.h"
#include "OutputDevice.h"
#include "Channel.h"
#include "../Common/Exceptions.h"

namespace TVPlayR {
	namespace Core {

bool OutputDevice::AssignToChannel(Channel* channel)
{
	if (channel_)
		return false;
	channel_ = channel;
	return true;
}

void OutputDevice::ReleaseChannel()
{
	channel_ = nullptr;
}

}}