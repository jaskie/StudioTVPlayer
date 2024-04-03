#pragma once

namespace TVPlayR {
		enum class FieldOrder;
		enum class ColorSpace;
		
		namespace Core {
		enum class VideoFormatType {
			invalid,
			pal,
			pal_fha,
			ntsc,
			ntsc_fha,
			v720p5000,
			v720p5994,
			v720p6000,
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
			v2160p5994,
			v2160p6000,
			count
		};

class VideoFormat final
{
public:
	VideoFormat();
	VideoFormat(enum VideoFormatType type);
	VideoFormat& operator=(const VideoFormat&) = default;
	const enum VideoFormatType type() const;
	const enum ColorSpace ColorSpace() const;
	int width() const;
	int height() const;
	Common::Rational<int> SampleAspectRatio() const;
	Common::Rational<int> FrameRate() const;
	std::string Name() const;
	FieldOrder field_order() const;
	bool interlaced() const;
	bool IsDropFrame() const;
	std::string FrameNumberToString(int frame_number) const;
	int StringToFrameNumber(const std::string &tc) const;
	uint32_t FrameNumberToSmpteTimecode(int frame_number) const;
	int TimeToFrameNumber(std::int64_t time) const;

private:
	enum VideoFormatType type_;
	enum ColorSpace color_space_;
	int width_;
	int height_;
	std::string name_;
	Common::Rational<int> sample_aspect_ratio_;
	Common::Rational<int> frame_rate_;
	TVPlayR::FieldOrder field_order_;
	bool timecode_is_supported_;
	bool is_drop_frame_;
	AVTimecode timecode_;
};


}}