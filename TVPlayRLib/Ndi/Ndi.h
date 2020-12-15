#pragma once
#include "../Core/OutputDevice.h"

namespace TVPlayR {
	namespace Core {
		class OutputFrameClock;

	}
	namespace Ndi {

class Ndi : public Core::OutputDevice
{
public:
	Ndi(const std::string& source_name, const std::string& group_name);
	~Ndi();
	virtual std::shared_ptr<Core::OutputFrameClock> OutputFrameClock() override;
	//OutputDevice
	virtual bool AssignToChannel(Core::Channel * channel) override;
	virtual void ReleaseChannel() override;
	virtual bool IsPlaying() const override;
	virtual void Push(FFmpeg::AVFramePtr& video, FFmpeg::AVFramePtr& audio) override;

private:
	struct implementation;
	std::unique_ptr<implementation> impl_;

};

}}