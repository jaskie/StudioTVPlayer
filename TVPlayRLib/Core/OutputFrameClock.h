#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Core {

class Channel;

class OutputFrameClock: Common::NonCopyable
{
private:
	Channel* channel_ = nullptr;
	void AssignToChannel(Channel* channel);
	void ReleaseChannel(Channel* channel);
	friend class Channel;

public:
	OutputFrameClock();
	void RequestFrame(int audio_samples_count);

};

}}