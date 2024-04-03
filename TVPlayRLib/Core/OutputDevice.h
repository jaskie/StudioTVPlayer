#pragma once

namespace TVPlayR {
	enum class PixelFormat;

	namespace Core {
		enum class VideoFormatType;
		class ClockTarget;
		class OverlayBase;
		struct AVSync;

class OutputSink
{
public:
	virtual void Push(const Core::AVSync &sync) = 0;
};

class FrameClockSource {
public:
	virtual void RegisterClockTarget(const std::shared_ptr<ClockTarget> &target) = 0;
	virtual void UnregisterClockTarget(const std::shared_ptr<ClockTarget> &target) = 0;
};

class OutputDevice : public FrameClockSource, public OutputSink, private Common::NonCopyable
{
public:
	virtual void Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) = 0;
	virtual void AddOverlay(const std::shared_ptr<OverlayBase> &overlay) = 0;
	virtual void RemoveOverlay(const std::shared_ptr<OverlayBase> &overlay) = 0;
};

}}