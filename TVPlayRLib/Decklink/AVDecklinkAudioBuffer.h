#pragma once
#include "../FFMpeg/Utils.h"

namespace TVPlayR {
	namespace Decklink {

		class AVDecklinkAudioBuffer 
		{
		private:
			const FFmpeg::AVFramePtr frame_;
		public:
			AVDecklinkAudioBuffer(FFmpeg::AVFramePtr& frame) :
				frame_(frame)
			{};
			AVDecklinkAudioBuffer(const int samples, const int channels) :
				frame_(FFmpeg::AllocFrame())
			{
				frame_->format = AV_SAMPLE_FMT_S32;
				frame_->channels = channels;
				frame_->channel_layout = AV_CH_LAYOUT_STEREO;
				frame_->nb_samples = samples;
				av_frame_get_buffer(frame_.get(), 0);
				av_samples_set_silence(frame_->data, 0, frame_->nb_samples, frame_->channels,  static_cast<AVSampleFormat>(frame_->format));
			}
			
			void* GetBytes() const 
			{
				if (!frame_)
					return nullptr;
				return frame_->data[0];
			}

			unsigned int SamplesCount() const
			{
				if (!frame_)
					return 0;
				return static_cast<unsigned int>(frame_->nb_samples);
			}

			operator bool() const { return !!frame_; }

		};

	}
}