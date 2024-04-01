#pragma once
#include "FFmpegInputBase.h"
#include "../Core/AudioParameters.h"

namespace TVPlayR {
	namespace Core {
		struct AudioParameters;
		struct AVSync;
	}
    namespace FFmpeg {
		class AudioMuxer;
		class SynchronizingBuffer;

		struct FFmpegBufferedInput : public FFmpegInputBase, protected Common::DebugTarget
		{
		public:
			FFmpegBufferedInput(const std::string& file_name, Core::HwAccel acceleration, const std::string& hw_device, const Core::AudioParameters & output_audio_parameters);
			virtual ~FFmpegBufferedInput();
		protected:
			void Initialize();
			void InitializeAudioDecoders();
			void ProcessNextInputPacket();
			void ProcessVideo();
			void ProcessAudio(const std::unique_ptr<Decoder>& decoder);
			void FlushAudioMuxerIfNeeded();
			void FlushBufferOrLoopIfNeeded();
			virtual Core::AVSync PullSync(int audio_samples_count);
			bool Seek(const std::int64_t time);
		private:
			std::atomic_bool is_eof_ = false;
			std::atomic_bool is_playing_ = false;
			std::atomic_bool is_loop_ = false;
			std::vector<std::unique_ptr<Decoder>> audio_decoders_;
			std::unique_ptr<AudioMuxer> audio_muxer_;
			std::unique_ptr<SynchronizingBuffer> buffer_;
			std::thread producerThread_;
			const Core::AudioParameters output_audio_parameters_;
			void ProducerTheradStart();
		};
	}
}