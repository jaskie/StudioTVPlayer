#pragma once
#include "MediaType.h"

namespace TVPlayR {
	namespace Core {
class StreamInfo
{
public:
	int Index;
	MediaType Type;
	bool IsPreffered;
	std::int64_t StartTime;
	std::int64_t Duration;
	int AudioChannelsCount;
	std::string Language;
	const AVCodec* Codec;
	AVStream* Stream;
};

}}