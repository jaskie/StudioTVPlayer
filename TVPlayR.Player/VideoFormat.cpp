#include "stdafx.h"
#include "VideoFormat.h"
#include "ClrStringHelper.h"

namespace TVPlayR {

	array<VideoFormat^>^ VideoFormat::EnumVideoFormats()
	{
		if (_videoFormats == nullptr)
		{
			_videoFormats = gcnew array<VideoFormat^>(static_cast<int>(Core::VideoFormat::Type::count) - 1);
			for (int type = Core::VideoFormat::Type::pal; type != Core::VideoFormat::Type::count; type++)
				_videoFormats[type - 1] = gcnew VideoFormat(static_cast<Core::VideoFormat::Type>(type));
		}
		return _videoFormats;
	}

	VideoFormat::VideoFormat(int video_format_id)
		: _native_fomat(new Core::VideoFormat(static_cast<Core::VideoFormat::Type>(video_format_id)))
		, _name(gcnew String(_native_fomat->Name().c_str()))
		, _sample_aspect_ratio(gcnew Rational(_native_fomat->SampleAspectRatio()))
		, _frame_rate(gcnew Rational(_native_fomat->FrameRate()))
	{ }

	VideoFormat::~VideoFormat()
	{
		this->!VideoFormat();
	}

	VideoFormat::!VideoFormat()
	{
		delete _native_fomat;
	}
		
	String^ VideoFormat::FrameNumberToString(int frame_number, bool drop_frame)
	{
		return gcnew String(_native_fomat->FrameNumberToString(frame_number, drop_frame).c_str());
	}

	int VideoFormat::StringToFrameNumber(String^ tc)
	{
		return _native_fomat->StringToFrameNumber(ClrStringToStdString(tc));
	}

	int VideoFormat::TimeToFrameNumber(TimeSpan time)
	{
		return static_cast<int>(time.Ticks * _frame_rate->Numerator / (_frame_rate->Denominator * TimeSpan::TicksPerSecond));
	}

	TimeSpan VideoFormat::FrameNumberToTime(int frame_number)
	{
		return TimeSpan(frame_number * TimeSpan::TicksPerSecond * _frame_rate->Denominator / _frame_rate->Numerator);
	}

}