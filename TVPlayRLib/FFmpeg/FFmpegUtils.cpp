#include "../pch.h"
#include "FFmpegUtils.h"
#include "../Core/VideoFormat.h"
#include "../PixelFormat.h"
#include "../FieldOrder.h"

namespace TVPlayR {
	namespace FFmpeg {


		std::shared_ptr<AVPacket> AllocPacket()
		{
			return std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket* p) { av_packet_free(&p); });
		}

		static void FreeFrame(AVFrame* f) { av_frame_free(&f); }

		std::shared_ptr<AVFrame> AllocFrame()
		{
			return std::shared_ptr<AVFrame>(av_frame_alloc(), FreeFrame);
		}

		std::shared_ptr<AVFrame> CloneFrame(const std::shared_ptr<AVFrame>& source)
		{
			if (!source)
				return nullptr;
			AVFrame* frame = av_frame_alloc();
			THROW_ON_FFMPEG_ERROR(av_frame_ref(frame, source.get()));
			return std::shared_ptr<AVFrame>(frame, FreeFrame);
		}

		std::shared_ptr<AVFrame> CopyFrame(const std::shared_ptr<AVFrame>& source)
		{
			if (!source)
				return nullptr;
			AVFrame* frame = av_frame_alloc();
			frame->width = source->width;
			frame->height = source->height;
			frame->format = source->format;
			frame->pict_type = source->pict_type;
			frame->sample_aspect_ratio = source->sample_aspect_ratio;
			frame->interlaced_frame = source->interlaced_frame;
			frame->top_field_first = source->top_field_first;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame, 0));
			av_image_copy(frame->data, frame->linesize, const_cast<const uint8_t**>(source->data), const_cast<const int*>(source->linesize), static_cast<AVPixelFormat>(source->format), source->width, source->height);
			return std::shared_ptr<AVFrame>(frame, FreeFrame);
		}

		std::shared_ptr<AVFrame> CreateEmptyVideoFrame(const Core::VideoFormat& format, TVPlayR::PixelFormat pix_fmt)
		{
			AVFrame* frame = av_frame_alloc();
			if (!frame)
				THROW_EXCEPTION("FFmpegUtils: video frame not allocated");
			frame->width = format.width();
			frame->height = format.height();
			frame->format = TVPlayR::PixelFormatToFFmpegFormat(pix_fmt);
			frame->pict_type = AV_PICTURE_TYPE_NONE;
			frame->sample_aspect_ratio = format.SampleAspectRatio().av();
			frame->interlaced_frame = format.interlaced();
			frame->top_field_first = format.field_order() == TVPlayR::FieldOrder::TopFieldFirst;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame, 0));
			uint32_t* data_begin = reinterpret_cast<uint32_t*>(frame->data[0]);
			ptrdiff_t linesize[4] = { frame->linesize[0], 0, 0, 0 };
			switch (pix_fmt)
			{
			case TVPlayR::PixelFormat::rgb10:
			case TVPlayR::PixelFormat::yuv422:
				THROW_ON_FFMPEG_ERROR(av_image_fill_black(frame->data, linesize, static_cast<AVPixelFormat>(frame->format), AVColorRange::AVCOL_RANGE_MPEG, frame->width, frame->height));
				break;
			case TVPlayR::PixelFormat::bgra:
				// we want fully transparent, black frame here
				std::fill(data_begin, data_begin + (frame->linesize[0] * frame->height / sizeof(uint32_t)), 0x00000000);
				break;
			default:
				THROW_EXCEPTION("Invalid frame pixel format")
				break;
			}
			return std::shared_ptr<AVFrame>(frame, FreeFrame);
		}

		std::shared_ptr<AVFrame> CreateSilentAudioFrame(int samples_count, int num_channels, AVSampleFormat format)
		{
			if (samples_count <= 0)
				return nullptr;
			assert(num_channels <= 63);
			AVFrame* frame = av_frame_alloc();
			if (!frame)
				THROW_EXCEPTION("FFmpegUtils: audio frame not allocated");
			frame->format = format;
			av_channel_layout_default(&frame->ch_layout, num_channels);
			frame->nb_samples = samples_count;
			frame->sample_rate = 48000;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame, 0));
			THROW_ON_FFMPEG_ERROR(av_samples_set_silence(frame->data, 0, frame->nb_samples, frame->ch_layout.nb_channels, static_cast<AVSampleFormat>(frame->format)));
			return std::shared_ptr<AVFrame>(frame, FreeFrame);
		}

		void DumpFilter(const std::string& filter_str, AVFilterGraph* graph)
		{
			OutputDebugStringA("\nFilter: ");
			OutputDebugStringA(filter_str.c_str());
			OutputDebugStringA("\n");
			char* filter_dump = avfilter_graph_dump(graph, NULL);
			OutputDebugStringA(filter_dump);
			av_free(filter_dump);
		}

		bool HaveAlphaChannel(AVPixelFormat format)
		{
			switch (format)
			{
			case AV_PIX_FMT_ARGB:
			case AV_PIX_FMT_RGBA:
			case AV_PIX_FMT_ABGR:
			case AV_PIX_FMT_BGRA:
			case AV_PIX_FMT_YA8:
			case AV_PIX_FMT_YA16:
			case AV_PIX_FMT_GBRAPF32:
			case AV_PIX_FMT_YUVA422P:
			case AV_PIX_FMT_YUVA444P:
			case AV_PIX_FMT_YUVA420P9:
			case AV_PIX_FMT_YUVA422P9:
			case AV_PIX_FMT_YUVA444P9:
			case AV_PIX_FMT_YUVA420P10:
			case AV_PIX_FMT_YUVA422P10:
			case AV_PIX_FMT_YUVA444P10:
			case AV_PIX_FMT_YUVA420P16:
			case AV_PIX_FMT_YUVA422P16:
			case AV_PIX_FMT_YUVA444P16:
			case AV_PIX_FMT_YUVA422P12:
			case AV_PIX_FMT_YUVA444P12:
				return true;
			default:
				return false;
			}
		}


		AVDictionary* ReadOptions(const std::string& params)
		{
			AVDictionary* result = nullptr;
			if (av_dict_parse_string(&result, params.c_str(), "=", ",", 0))
			{
#ifdef DEBUG
				OutputDebugStringA("ReadOptions failed for: ");
				OutputDebugStringA(params.c_str());
				OutputDebugStringA("\n");
#endif // DEBUG
			}
			return result;
		}

		AVPixelFormat FindBestOutputPixelFormat(const AVCodec *codec, const std::string& preffered_format, AVPixelFormat channelPixelFormat)
		{
			AVPixelFormat preffered = av_get_pix_fmt(preffered_format.c_str());
			int i = 0;
			if (preffered != AV_PIX_FMT_NONE)
			{
				while (codec->pix_fmts[i] != AV_PIX_FMT_NONE)
				{
					if (codec->pix_fmts[i] == preffered)
						return preffered;
					i++;
				}
			}
			/*
			* The following code is commented out until a method to establish actual codec support for a pixel format.
			* Currently, only way to determine the codec support on specific hardware is to try to open the codec with the desired pixel format, which ends in access violation exception.
			*
			i = 0;
			while (codec->pix_fmts[i] != AV_PIX_FMT_NONE)
			{
				if (codec->pix_fmts[i] == channelPixelFormat)
					return channelPixelFormat;
				i++;
			}
			*/
			return codec->pix_fmts[0];
		}


}}