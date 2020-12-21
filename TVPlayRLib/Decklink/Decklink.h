#pragma once

#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Core {
		class OutputFrameClock;
	}
	namespace Decklink {

class Decklink : public Core::OutputDevice
{
public:
	explicit Decklink(IDeckLink* decklink, const int card_index);
	~Decklink();
	virtual std::shared_ptr<Core::OutputFrameClock> OutputFrameClock() override;
	std::wstring GetDisplayName();
	std::wstring GetModelName();
	bool SetBufferSize(size_t size);
	size_t GetBufferSize() const;
	//OutputDevice
	virtual bool AssignToChannel(Core::Channel* channel) override;
	virtual void ReleaseChannel() override;
	virtual bool IsPlaying() const override;
	virtual void Push(FFmpeg::AVFramePtr& video, FFmpeg::AVFramePtr& audio) override;

private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}