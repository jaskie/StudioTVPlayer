#pragma once

namespace TVPlayR {
	namespace Core {

class AudioVolume final : public Common::NonCopyable
{
public:
	AudioVolume();
	void SetVolume(float volume);

	/// <summary>
	/// Changes volume of the frame and calculates average volume
	/// </summary>
	/// <param name="frame">frame to process</param>
	/// <returns>average volume</returns>
	std::vector<float> ProcessVolume(const std::shared_ptr<AVFrame>& frame, float* coherence);
private:
	float volume_;
	float new_volume_;
};

}}