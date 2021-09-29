#pragma once
#include "Core/VideoFormat.h"
#include "Rational.h"

using namespace System;

namespace TVPlayR {
	
	public enum class FieldModeEnum {
		Unknown,
		Progressive,
		Lower,
		Upper
	};

	[Diagnostics::DebuggerDisplayAttribute(L"{Name}")]
	public ref class VideoFormat sealed
	{
	private:
		static VideoFormat();
		VideoFormat(int video_format_id);
		Core::VideoFormat* const _native_fomat;
		const String^ _name;
		const Rational _sample_aspect_ratio;
		Rational _frame_rate;
		static array<VideoFormat^>^ _videoFormats;
	internal:
		Core::VideoFormatType GetNativeEnumType() { return _native_fomat->type(); }
		static VideoFormat^ FindFormat(Core::VideoFormatType type);
	public:
		!VideoFormat();
		~VideoFormat();
		static property array<VideoFormat^>^ Formats { array<VideoFormat^>^ get() { return _videoFormats; }}
		property String^ Name {	String^ get() { return (String^)_name; }	}
		property Rational SampleAspectRatio { Rational get() { return _sample_aspect_ratio; }	}
		property int Width { int get() { return _native_fomat->width(); } }
		property int Height { int get() { return _native_fomat->height(); } }
		property Rational FrameRate { Rational get() { return _frame_rate; } }
		property bool IsInterlaced { bool get() { return _native_fomat->interlaced(); } }
		property FieldModeEnum FieldMode { FieldModeEnum get() { return static_cast<FieldModeEnum>(_native_fomat->field_order()); } }
		property int Id { int get() { return static_cast<int>(_native_fomat->type()); }}
		property bool IsDropFrame { bool get() { return _native_fomat->IsDropFrame(); }}
		String^ FrameNumberToString(int frame_number);
		int StringToFrameNumber(String^ tc);
		int TimeToFrameNumber(TimeSpan time);
		TimeSpan FrameNumberToTime(int frame_number);
	};

}