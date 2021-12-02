#pragma once
#include "../Common/NonCopyable.h"
#include "../FFmpeg/AVSync.h"

namespace TVPlayR {
	namespace Core {

class Channel;
class OverlayBase;

class OutputDevice : private Common::NonCopyable
{
public:
	typedef std::function<void(int audio_samples_required)> FRAME_REQUESTED_CALLBACK;
	virtual bool AssignToChannel(const Channel& channel) = 0;
	virtual void ReleaseChannel() = 0;
	virtual void AddOverlay(std::shared_ptr<OverlayBase> overlay) = 0;
	virtual void RemoveOverlay(std::shared_ptr<OverlayBase> overlay) = 0;
	virtual void Push(FFmpeg::AVSync& sync) = 0;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) = 0;
};

}}