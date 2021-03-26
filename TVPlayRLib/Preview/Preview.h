#pragma once

#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Core {
		class Channel;
	}
	namespace Preview {

class Preview: public Core::OutputDevice
{
public:
	explicit Preview();
	~Preview();
	typedef void(*FRAME_PLAYED_CALLBACK)(std::shared_ptr<AVFrame>);
	virtual bool AssignToChannel(Core::Channel& channel) override;
	virtual void ReleaseChannel() override;
	virtual void Push(FFmpeg::AVSync& sync) override;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
	void SetFramePlayedCallback(FRAME_PLAYED_CALLBACK frame_played_callback);
	void CreateFilter(int width, int height);
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}
