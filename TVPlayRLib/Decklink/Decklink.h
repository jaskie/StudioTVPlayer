#pragma once

#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Decklink {

class Decklink : public Core::OutputDevice
{
public:
	explicit Decklink(IDeckLink* decklink, int index);
	~Decklink();
	std::wstring GetDisplayName();
	std::wstring GetModelName();
	bool SetBufferSize(int size);
	int GetBufferSize() const;
	//OutputDevice
	virtual bool AssignToChannel(Core::Channel& channel) override;
	virtual void ReleaseChannel() override;
	virtual bool IsPlaying() const override;
	virtual void Push(FFmpeg::AVSync& sync) override;
	virtual void SetFrameRequestedCallback(FRAME_REQUESTED_CALLBACK frame_requested_callback) override;
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}