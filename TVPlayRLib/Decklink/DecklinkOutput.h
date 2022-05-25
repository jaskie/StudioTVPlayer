#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	enum class DecklinkKeyer;
	namespace Decklink {

class DecklinkOutput final : public Core::OutputDevice
{
public:
	explicit DecklinkOutput(IDeckLink* decklink, DecklinkKeyer keyer, int index);
	~DecklinkOutput();
	bool SetBufferSize(int size);
	int GetBufferSize() const;
	// Inherited via OutputDevice
	bool InitializeFor(const Core::Player& player) override;
	void Uninitialize() override;
	void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void Push(Core::AVSync& sync) override;
	virtual void RegisterClockTarget(Core::ClockTarget& target) override;
	virtual void UnregisterClockTarget(Core::ClockTarget& target) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}