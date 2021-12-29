#pragma once

#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Core {
		class Player;
	}
	namespace Preview {

class OutputPreview final : public Core::OutputDevice
{
public:
	explicit OutputPreview(int width, int height);
	~OutputPreview();
	typedef void(*FRAME_PLAYED_CALLBACK)(std::shared_ptr<AVFrame>);
	bool AssignToPlayer(const Core::Player& player) override;
	void ReleasePlayer() override;
	void AddOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void RemoveOverlay(std::shared_ptr<Core::OverlayBase>& overlay) override;
	void Push(FFmpeg::AVSync& sync) override;
	void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
	void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
