#include "stdafx.h"
#include "VideoFormat.h"
#include "FieldOrder.h"
#include "ClrStringHelper.h"

namespace TVPlayR {


	static VideoFormat::VideoFormat()
	{
		av_log_set_callback(NULL);
		_videoFormats = gcnew array<VideoFormat^>(static_cast<int>(Core::VideoFormatType::count) - 1);
		for (int type = static_cast<int>(Core::VideoFormatType::pal); type != static_cast<int>(Core::VideoFormatType::count); type++)
			_videoFormats[type - 1] = gcnew VideoFormat(type);
	}

	VideoFormat::VideoFormat(int video_format_id)
		: _native_fomat(new Core::VideoFormat(static_cast<Core::VideoFormatType>(video_format_id)))
		, _name(gcnew String(_native_fomat->Name().c_str()))
		, _sample_aspect_ratio(_native_fomat->SampleAspectRatio())
		, _frame_rate(_native_fomat->FrameRate())
	{ }

	VideoFormat::~VideoFormat()
	{
		this->!VideoFormat();
	}

	VideoFormat^ VideoFormat::FindFormat(Core::VideoFormatType type)
	{
		for each (VideoFormat ^ format in _videoFormats)
			if (format->GetNativeEnumType() == type)
				return format;
		return nullptr;
	}

	VideoFormat::!VideoFormat()
	{
		delete _native_fomat;
	}
		
	String^ VideoFormat::FrameNumberToString(int frame_number)
	{
		return gcnew String(_native_fomat->FrameNumberToString(frame_number).c_str());
	}

	int VideoFormat::StringToFrameNumber(String^ tc)
	{
		return _native_fomat->StringToFrameNumber(ClrStringToStdString(tc));
	}

	int VideoFormat::TimeToFrameNumber(TimeSpan time)
	{
		return static_cast<int>(time.Ticks * _frame_rate.Numerator / (_frame_rate.Denominator * TimeSpan::TicksPerSecond));
	}

	TimeSpan VideoFormat::FrameNumberToTime(int frame_number)
	{
		return TimeSpan(frame_number * TimeSpan::TicksPerSecond * _frame_rate.Denominator / _frame_rate.Numerator);
	}

}