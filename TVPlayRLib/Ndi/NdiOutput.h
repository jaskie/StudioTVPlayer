#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Ndi {

class NdiOutput final : public Core::OutputDevice
{
public:
	NdiOutput(const std::string& source_name, const std::string& group_names);
	~NdiOutput();
	//OutputDevice
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