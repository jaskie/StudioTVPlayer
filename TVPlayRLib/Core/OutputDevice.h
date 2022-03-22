#pragma once

namespace TVPlayR {

	namespace FFmpeg {
		struct AVSync;
	}

	namespace Core {

class Player;
class OverlayBase;

class OutputSink : private Common::NonCopyable
{
public:
	virtual void Push(FFmpeg::AVSync& sync) = 0;
};

class OutputDevice : public OutputSink
{
public:
	typedef std::function<void(int audio_samples_required)> FRAME_REQUESTED_CALLBACK;
	virtual bool AssignToPlayer(const Player& channel) = 0;
	virtual void ReleasePlayer() = 0;
	virtual void AddOverlay(std::shared_ptr<OverlayBase>& overlay) = 0;
	virtual void RemoveOverlay(std::shared_ptr<OverlayBase>& overlay) = 0;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) = 0;
};

}}