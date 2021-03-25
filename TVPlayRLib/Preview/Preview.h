#pragma once

#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Preview {

class Preview: public Core::OutputDevice
{
public:
	explicit Preview();
	~Preview();
	//OutputDevice
	virtual void ReleaseChannel() override;
	virtual bool IsPlaying() const override;
	virtual void Push(FFmpeg::AVSync& sync) override;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
