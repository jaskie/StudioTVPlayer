#pragma once
#include "../Common/NonCopyable.h"
#include "../FFMpeg/AVSync.h"

namespace TVPlayR {
	namespace Core {

class Channel;

class OutputDevice : public Common::NonCopyable
{
private:
	Channel* channel_ = nullptr;
public:
	typedef std::function<void(int audio_samples_required)> FRAME_REQUESTED_CALLBACK;
	virtual bool AssignToChannel(Channel& channel) = 0;
	virtual void ReleaseChannel() = 0;
	virtual void Push(FFmpeg::AVSync& sync) = 0;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) = 0;
};

}}