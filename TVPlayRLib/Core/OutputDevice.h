#pragma once
#include "../Common/NonCopyable.h"
#include "../FFMpeg/Utils.h"

namespace TVPlayR {
	namespace Core {

class Channel;
class OutputFrameClock;

class OutputDevice : public Common::NonCopyable
{
private:
	Channel* channel_ = nullptr;
public:
	virtual bool AssignToChannel(Channel* channel);
	virtual void ReleaseChannel();
	virtual std::shared_ptr<OutputFrameClock> OutputFrameClock() = 0;
	virtual bool IsPlaying() const = 0;
	virtual void Push(FFmpeg::AVFramePtr& video, FFmpeg::AVFramePtr& audio) = 0;
};

}}