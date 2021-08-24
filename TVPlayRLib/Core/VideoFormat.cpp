#include "../pch.h"
#include "VideoFormat.h"
#include "../FFMpeg/FFMpegUtils.h"

namespace TVPlayR {
	namespace Core {

VideoFormat::VideoFormat()
	: VideoFormat(VideoFormatType::invalid)
{
}

VideoFormat::VideoFormat(enum VideoFormatType type)
	: type_(type)
	, is_drop_frame_(false)
{
	switch (type)
	{
	case VideoFormatType::ntsc:
		width_ = 720;
		height_ = 480;
		field_mode_ = FieldMode::lower;
		sample_aspect_ratio_ = Common::Rational<int>(10, 11);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "NTSC 4:3";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::ntsc_fha:
		width_ = 720;
		height_ = 480;
		field_mode_ = FieldMode::lower;
		sample_aspect_ratio_ = Common::Rational<int>(40, 33);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "NTSC 16:9";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::pal:
		width_ = 720;
		height_ = 576;
		field_mode_ = FieldMode::upper;
		sample_aspect_ratio_ = Common::Rational<int>(59, 54);
		frame_rate_ = Common::Rational<int>(25, 1);
		name_ = "PAL 4:3";
		break;
	case VideoFormatType::pal_fha:
		width_ = 720;
		height_ = 576;
		field_mode_ = FieldMode::upper;
		sample_aspect_ratio_ = Common::Rational<int>(64, 45);
		frame_rate_ = Common::Rational<int>(25, 1);
		name_ = "PAL 16:9";
		break;
	case VideoFormatType::v1080i5000:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::upper;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(25, 1);
		name_ = "1080i50";
		break;
	case VideoFormatType::v1080i5994:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::upper;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "1080i59.94";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::v1080i6000:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::upper;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30, 1);
		name_ = "1080i60";
		break;
	case VideoFormatType::v1080p2398:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(24000, 1001);
		name_ = "1080p23.98";
		break;
	case VideoFormatType::v1080p2400:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(24, 1);
		name_ = "1080p24";
		break;
	case VideoFormatType::v1080p2500:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(25, 1);
		name_ = "1080p25";
		break;
	case VideoFormatType::v1080p2997:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "1080p29.97";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::v1080p3000:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30, 1);
		name_ = "1080p30";
		break;
	case VideoFormatType::v1080p5000:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(50, 1);
		name_ = "1080p50";
		break;
	case VideoFormatType::v1080p5994:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "1080p59.94";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::v1080p6000:
		width_ = 1920;
		height_ = 1080;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(60, 1);
		name_ = "1080p60";
		break;
	case VideoFormatType::v2160p2398:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(24000, 1001);
		name_ = "2160p23.98";
		break;
	case VideoFormatType::v2160p2400:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(24, 1);
		name_ = "2160p24";
		break;
	case VideoFormatType::v2160p2500:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(25, 1);
		name_ = "2160p25";
		break;
	case VideoFormatType::v2160p2997:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30000, 1001);
		name_ = "2160p29.97";
		is_drop_frame_ = true;
		break;
	case VideoFormatType::v2160p3000:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(30, 1);
		name_ = "2160p30";
		break;
	case VideoFormatType::v2160p5000:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(50, 1);
		name_ = "2160p50";
		break;
	case VideoFormatType::v2160p6000:
		width_ = 3840;
		height_ = 2160;
		field_mode_ = FieldMode::progressive;
		sample_aspect_ratio_ = Common::Rational<int>(1, 1);
		frame_rate_ = Common::Rational<int>(60, 1);
		name_ = "2160p60";
		break;
				
	default:
		width_ = 0;
		height_ = 0;
		field_mode_ = FieldMode::unknown;
		frame_rate_ = Common::Rational<int>();
		name_ = "invalid";
		break;
	}

	bool timecode_is_supported_ = av_timecode_check_frame_rate(frame_rate_.av()) == 0;
	if (timecode_is_supported_)
		THROW_ON_FFMPEG_ERROR(av_timecode_init(
			&timecode_,
			frame_rate_.av(),
			is_drop_frame_ ? AV_TIMECODE_FLAG_DROPFRAME : 0,
			0,
			NULL
		));
}

const enum VideoFormatType VideoFormat::type() const
{
	return type_;
}

int VideoFormat::width() const
{
	return width_;
}

int VideoFormat::height() const
{
	return height_;
}

Common::Rational<int> VideoFormat::SampleAspectRatio() const
{
	return sample_aspect_ratio_;
}

Common::Rational<int> VideoFormat::FrameRate() const
{
	return frame_rate_;
}

std::string VideoFormat::Name() const
{
	return name_;
}

VideoFormat::FieldMode VideoFormat::field_mode() const
{
	return field_mode_;
}

bool VideoFormat::interlaced() const
{
	return field_mode_ == FieldMode::lower || field_mode_ == FieldMode::upper;
}

bool VideoFormat::IsDropFrame() const
{
	return is_drop_frame_;
}

std::string VideoFormat::FrameNumberToString(int frame_number)
{
	if (timecode_is_supported_)
	{
		char buffer[AV_TIMECODE_STR_SIZE];
		return av_timecode_make_string(&timecode_, buffer, frame_number);
	}
	return "";
}

int VideoFormat::StringToFrameNumber(const std::string& tc)
{
	if (!timecode_is_supported_)
		return 0;
	AVTimecode timecode;
	av_timecode_init_from_string(&timecode, frame_rate_.av(), tc.c_str(), NULL);
	return timecode.start;
}

uint32_t VideoFormat::FrameNumberToSmpteTimecode(int frame_number)
{
	if (!timecode_is_supported_)
		return 0;
	return av_timecode_get_smpte_from_framenum(&timecode_, frame_number);
}

int VideoFormat::TimeToFrameNumber(int64_t time)
{
	return static_cast<int>(av_rescale(time,  frame_rate_.Numerator(), frame_rate_.Denominator() * AV_TIME_BASE));

}


		
}}