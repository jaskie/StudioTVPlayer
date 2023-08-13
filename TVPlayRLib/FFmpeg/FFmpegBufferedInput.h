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
			void InitializeBuffer();
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
			std::atomic_bool is_producer_running_ = true;
			std::vector<std::unique_ptr<Decoder>> audio_decoders_;
			std::unique_ptr<AudioMuxer> audio_muxer_;
			std::unique_ptr<SynchronizingBuffer> buffer_;
			std::mutex initialization_mutex_;
			std::condition_variable initialization_cv_;
			std::mutex producer_mutex_;
			std::condition_variable producer_cv_;
			std::thread producer_;
			const Core::AudioParameters output_audio_parameters_;
			void ProducerTheradStart();
		};
	}
}