#pragma once

namespace TVPlayR {
	namespace Core {

class AudioVolume final : public Common::NonCopyable
{
public:
	AudioVolume();
	void SetVolume(double volume);

	/// <summary>
	/// Changes volume of the frame and calculates average volume
	/// </summary>
	/// <param name="frame">frame to process</param>
	/// <returns>average volume</returns>
	std::vector<double> ProcessVolume(const std::shared_ptr<AVFrame>& frame, double* coherence);
private:
	std::uint32_t volume_;
	std::int32_t new_volume_;
};

}}