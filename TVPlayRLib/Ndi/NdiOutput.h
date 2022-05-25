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
	bool InitializeFor(const Core::Player& player) override;
	void Uninitialize() override;
	void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	//OutputSink
	void Push(Core::AVSync& sync) override;
	//FrameClockSource
	virtual void RegisterClockTarget(Core::ClockTarget& target) override;
	virtual void UnregisterClockTarget(Core::ClockTarget& target) override;

private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}