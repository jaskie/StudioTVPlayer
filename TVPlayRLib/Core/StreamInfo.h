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
	int64_t StartTime;
	int64_t Duration;
	int AudioChannelsCount;
	std::string Language;
	const AVCodec* Codec;
	AVStream* Stream;
};

}}