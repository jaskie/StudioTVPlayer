#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	namespace Core {
		class Player;
	}

	ref class DecklinkOutput;
	ref class OutputPreview;
	ref class InputBase;
	ref class OutputBase;
	ref class OverlayBase;
	ref class VideoFormat;
	ref class AudioVolumeEventArgs;
	enum class PixelFormat;

	public ref class Player sealed
	{
	private:
		Core::Player* const _player;
		float _volume = 1.0f;
		VideoFormat^ _videoFormat;
		const PixelFormat _pixelFormat;
		delegate void AudioVolumeDelegate(std::vector<float>&, float);
		AudioVolumeDelegate^ _audioVolumeDelegate;
		GCHandle _audioVolumeHandle;
		System::Collections::Generic::List<OutputBase^>^ _outputs = gcnew System::Collections::Generic::List<OutputBase^>();
		void AudioVolumeCallback(std::vector<float>& audio_volume, float coherence);
	public:
		Player(String^ name, TVPlayR::VideoFormat^ videoFormat, TVPlayR::PixelFormat pixelFormat, int audioChannelCount);
		~Player();
		!Player();
		bool AddOutput(OutputBase^ output, bool setAsClockBase);
		void RemoveOutput(OutputBase^ output);
		void AddOverlay(OverlayBase^ overlay);
		void Load(InputBase^ file);
		void Preload(InputBase^ file);
		void Clear();
		property float Volume { float get(); void set(float volume); }
		property TVPlayR::VideoFormat^ VideoFormat { TVPlayR::VideoFormat^ get() { return _videoFormat; }}
		property TVPlayR::PixelFormat PixelFormat { TVPlayR::PixelFormat get() { return _pixelFormat; } }
		event EventHandler<AudioVolumeEventArgs^>^ AudioVolume;
	};
}
