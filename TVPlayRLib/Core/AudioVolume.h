#pragma once
#include "../Common/NonCopyable.h"

namespace TVPlayR {
	namespace Core {

class AudioVolume : public Common::NonCopyable
{
public:
	AudioVolume();
	void SetVolume(double volume);
	void ProcessVolume(const std::shared_ptr<AVFrame>& frame);
private:
	uint32_t old_volume_;
	uint32_t new_volume_;
};

}}