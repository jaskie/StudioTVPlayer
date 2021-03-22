#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Core {

class AudioVolume : public Common::NonCopyable
{
public:
	AudioVolume();
	void SetVolume(float volume);
	void ProcessVolume(const std::shared_ptr<AVFrame>& frame);
	float GetVolume() const;
private:
	uint32_t old_volume_;
	uint32_t new_volume_;
};

}}