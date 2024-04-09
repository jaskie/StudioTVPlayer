#pragma once
#include "MediaType.h"

namespace TVPlayR {
	namespace Core {
struct StreamInfo
{
	int Index;
	MediaType Type;
	/// <summary>
	/// if choosen by av_find_best_stream()
	/// </summary>
	bool IsPreffered;
	/// <summary>
	/// stream start time, in AV_TIME_BASE units
	/// </summary>
	std::int64_t StartTime;
	/// <summary>
	/// stream durationm, in AV_TIME_BASE units, may be zero if unknown
	/// </summary>
	std::int64_t Duration;
	int AudioChannelsCount;
	std::string Language;
	const AVCodec *Codec;
	AVStream *Stream;
};

}}