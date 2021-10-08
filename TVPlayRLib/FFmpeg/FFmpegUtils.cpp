#include "../pch.h"
#include "FFmpegUtils.h"
#include "../Core/VideoFormat.h"
#include "../Core/PixelFormat.h"

namespace TVPlayR {
	namespace FFmpeg {
		std::shared_ptr<AVFrame> CreateEmptyVideoFrame(const Core::VideoFormat& format, Core::PixelFormat pix_fmt)
		{
			auto frame = AllocFrame();
			frame->width = format.width();
			frame->height = format.height();
			frame->display_picture_number = -1;
			frame->format = Core::PixelFormatToFFmpegFormat(pix_fmt);
			frame->pict_type = AV_PICTURE_TYPE_I;
			THROW_ON_FFMPEG_ERROR(av_frame_get_buffer(frame.get(), 0));
			if (pix_fmt == Core::PixelFormat::bgra)
			{
				// to make transparent alpha
				memset(frame->data[0], 0x10101000, frame->linesize[0] * frame->height);
			}
			else
			{
				ptrdiff_t linesize[4] = { frame->linesize[0], 0, 0, 0 };
				THROW_ON_FFMPEG_ERROR(av_image_fill_black(frame->data, linesize, static_cast<AVPixelFormat>(frame->format), AVColorRange::AVCOL_RANGE_MPEG, frame->width, frame->height));
			}
			return frame;
		}

		std::shared_ptr<AVFrame> CreateSilentAudioFrame(int samples_count, int num_channels, AVSampleFormat format)
		{
			if (samples_count < 0)
				samples_count = 0;
			assert(num_channels <= 63);
			auto frame = AllocFrame();
			frame->format = format;
			frame->channels = num_channels;
			frame->channel_layout = 0x7FFFFFFFFFFFFFFFULL >> (63 - num_channels);
			frame->nb_samples = samples_count;
			frame->sample_rate = 48000;
			av_frame_get_buffer(frame.get(), 0);
			av_samples_set_silence(frame->data, 0, frame->nb_samples, frame->channels, static_cast<AVSampleFormat>(frame->format));
			return frame;
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
			case AV_PIX_FMT_YA16BE:
			case AV_PIX_FMT_YA16LE:
			case AV_PIX_FMT_GBRAPF32BE:
			case AV_PIX_FMT_GBRAPF32LE:
			case AV_PIX_FMT_YUVA422P:
			case AV_PIX_FMT_YUVA444P:
			case AV_PIX_FMT_YUVA420P9LE:
			case AV_PIX_FMT_YUVA420P9BE:
			case AV_PIX_FMT_YUVA422P9BE:
			case AV_PIX_FMT_YUVA422P9LE:
			case AV_PIX_FMT_YUVA444P9BE:
			case AV_PIX_FMT_YUVA444P9LE:
			case AV_PIX_FMT_YUVA420P10BE:
			case AV_PIX_FMT_YUVA420P10LE:
			case AV_PIX_FMT_YUVA422P10BE:
			case AV_PIX_FMT_YUVA422P10LE:
			case AV_PIX_FMT_YUVA444P10BE:
			case AV_PIX_FMT_YUVA444P10LE:
			case AV_PIX_FMT_YUVA420P16BE:
			case AV_PIX_FMT_YUVA420P16LE:
			case AV_PIX_FMT_YUVA422P16BE:
			case AV_PIX_FMT_YUVA422P16LE:
			case AV_PIX_FMT_YUVA444P16BE:
			case AV_PIX_FMT_YUVA444P16LE:
			case AV_PIX_FMT_YUVA422P12BE:
			case AV_PIX_FMT_YUVA422P12LE:
			case AV_PIX_FMT_YUVA444P12BE:
			case AV_PIX_FMT_YUVA444P12LE:
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


}}