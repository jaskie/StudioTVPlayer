#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	enum class DecklinkKeyer;
	enum class TimecodeOutputSource;
	namespace Decklink {

class DecklinkOutput final : public Core::OutputDevice
{
public:
	explicit DecklinkOutput(IDeckLink* decklink, int index, DecklinkKeyer keyer, TimecodeOutputSource timecode_source);
	virtual ~DecklinkOutput();
	bool SetBufferSize(int size);
	int GetBufferSize() const;
	// Inherited via OutputDevice
	bool Initialize(Core::VideoFormatType video_format, PixelFormat pixel_format, int audio_channel_count, int audio_sample_rate) override;
	void Uninitialize() override;
	void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void Push(Core::AVSync& sync) override;
	void RegisterClockTarget(Core::ClockTarget& target) override;
	void UnregisterClockTarget(Core::ClockTarget& target) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}