#pragma once

namespace TVPlayR {

	namespace FFmpeg {
		struct AVSync;
	}

	namespace Core {

		class ClockTarget;
		class Player;
		class OverlayBase;

class OutputSink
{
public:
	virtual void Push(FFmpeg::AVSync& sync) = 0;
};

class FrameClockSource {
public:
	virtual void RegisterClockTarget(ClockTarget& target) = 0;
	virtual void UnregisterClockTarget(ClockTarget& target) = 0;
};

class OutputDevice : public FrameClockSource, public OutputSink, private Common::NonCopyable
{
public:
	virtual bool InitializeFor(const Player& player) = 0;
	virtual void Uninitialize() = 0;
	virtual void AddOverlay(std::shared_ptr<OverlayBase>& overlay) = 0;
	virtual void RemoveOverlay(std::shared_ptr<OverlayBase>& overlay) = 0;
};

}}