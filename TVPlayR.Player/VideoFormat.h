#pragma once
#include "Rational.h"

using namespace System;

namespace TVPlayR {

	enum class FieldOrder;

	namespace Core {
		class VideoFormat;
		enum class VideoFormatType;
	}

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
		Core::VideoFormatType GetNativeEnumType();
		static VideoFormat^ FindFormat(Core::VideoFormatType type);
	public:
		!VideoFormat();
		~VideoFormat();
		static property array<VideoFormat^>^ Formats { array<VideoFormat^>^ get() { return _videoFormats; }}
		property String^ Name {	String^ get() { return (String^)_name; }	}
		property Rational SampleAspectRatio { Rational get() { return _sample_aspect_ratio; }	}
		property int Width { int get(); }
		property int Height { int get(); }
		property Rational FrameRate { Rational get() { return _frame_rate; } }
		property bool IsInterlaced { bool get(); }
		property TVPlayR::FieldOrder FieldOrder { TVPlayR::FieldOrder get(); }
		property int Id { int get(); }
		property bool IsDropFrame { bool get(); }
		String^ FrameNumberToString(int frame_number);
		int StringToFrameNumber(String^ tc);
		int TimeToFrameNumber(TimeSpan time);
		TimeSpan FrameNumberToTime(int frame_number);
	};

}