#include "../pch.h"
#include "OutputFrameClock.h"
#include "Channel.h"

namespace TVPlayR {
	namespace Core {

OutputFrameClock::OutputFrameClock()
{
}

void OutputFrameClock::AssignToChannel(Channel* channel)
{
	channel_ = channel;
}

void OutputFrameClock::ReleaseChannel(Channel* channel)
{
	assert(channel == channel_);
	channel_ = nullptr;
}


void OutputFrameClock::RequestFrame(int audio_samples_count)
{
	if (!channel_)
		return;
	channel_->RequestFrame(audio_samples_count);
}

}}
