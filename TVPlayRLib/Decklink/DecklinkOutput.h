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
	bool AssignToPlayer(const Core::Player& player) override;
	void ReleasePlayer() override;
	void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void Push(FFmpeg::AVSync& sync) override;
	void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}