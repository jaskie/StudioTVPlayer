#pragma once
#include "../Common/NonCopyable.h"
#include "ApiVersion.h"

namespace TVPlayR {
	namespace Core {
		enum class VideoFormatType;
	}
	namespace Decklink {

class DecklinkOutput;
class DecklinkInfo;
class DecklinkInput;
enum class DecklinkTimecodeSource;

class DecklinkIterator: Common::NonCopyable
{
public:
	DecklinkIterator();
	~DecklinkIterator(); 
	std::shared_ptr<DecklinkInfo> operator [] (size_t pos);
	std::shared_ptr<DecklinkOutput> CreateOutput(const DecklinkInfo& info);
	std::shared_ptr<DecklinkInput> CreateInput(const DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count, DecklinkTimecodeSource timecode_source);
	size_t Size() const;
	std::shared_ptr<ApiVersion> GetVersion();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}