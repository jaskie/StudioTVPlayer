#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Decklink {

class DecklinkOutput final : public Core::OutputDevice
{
public:
	explicit DecklinkOutput(IDeckLink* decklink, bool internal_keyer, int index);
	~DecklinkOutput();
	bool SetBufferSize(int size);
	int GetBufferSize() const;
	// Inherited via OutputDevice
	virtual bool AssignToChannel(const Core::Channel& channel) override;
	virtual void ReleaseChannel() override;
	virtual void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	virtual void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	virtual void Push(FFmpeg::AVSync& sync) override;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}