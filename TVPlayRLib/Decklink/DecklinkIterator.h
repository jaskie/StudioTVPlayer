#pragma once

namespace TVPlayR {

	enum class DecklinkTimecodeSource;
	enum class DecklinkKeyer;

	namespace Core {
		enum class VideoFormatType;
	}
	namespace Decklink {

struct ApiVersion;
class DecklinkOutput;
class DecklinkInfo;
class DecklinkInput;

class DecklinkIterator final : Common::NonCopyable
{
public:
	DecklinkIterator();
	~DecklinkIterator(); 
	std::shared_ptr<Decklink::DecklinkInfo> operator [] (size_t pos);
	std::shared_ptr<Decklink::DecklinkOutput> CreateOutput(const DecklinkInfo& info, DecklinkKeyer keyer);
	std::shared_ptr<Decklink::DecklinkInput> CreateInput(const Decklink::DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video);
	size_t Size() const;
	std::shared_ptr<ApiVersion> GetVersion();
private:
	struct implementation;
	std::unique_ptr<implementation> impl_;
};

}}