#pragma once
#include "../Common/Rational.h"

namespace TVPlayR {
	namespace Core {

		enum class VideoFormatType {
			invalid,
			pal,
			pal_fha,
			ntsc,
			ntsc_fha,
			v1080p2398,
			v1080p2400,
			v1080i5000,
			v1080i5994,
			v1080i6000,
			v1080p2500,
			v1080p2997,
			v1080p3000,
			v1080p5000,
			v1080p5994,
			v1080p6000,
			v2160p2398,
			v2160p2400,
			v2160p2500,
			v2160p2997,
			v2160p3000,
			v2160p5000,
			v2160p6000,
			count
		};

class VideoFormat
{
public:
	enum class FieldMode {
		unknown,
		progressive,
		lower,
		upper
	};
	VideoFormat();
	VideoFormat(enum VideoFormatType type);
	VideoFormat& operator=(const VideoFormat&) = default;
	const enum VideoFormatType type() const;
	int width() const;
	int height() const;
	Common::Rational<int> SampleAspectRatio() const;
	Common::Rational<int> FrameRate() const;
	std::string Name() const;
	FieldMode field_mode() const;
	bool interlaced() const;
	std::string FrameNumberToString(int frame_number, bool drop_frame);
	int StringToFrameNumber(const std::string& tc);
private:
	enum VideoFormatType type_;
	int width_;
	int height_;
	std::string name_;
	Common::Rational<int> sample_aspect_ratio_;
	Common::Rational<int> frame_rate_;
	FieldMode field_mode_;
};


}}