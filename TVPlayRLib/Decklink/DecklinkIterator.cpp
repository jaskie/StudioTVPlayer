#include "../pch.h"
#include "../TimecodeOutputSource.h"
#include "ApiVersion.h"
#include "DecklinkOutput.h"
#include "DecklinkInfo.h"
#include "DecklinkIterator.h"
#include "DecklinkInput.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace Decklink {

		struct DecklinkIterator::implementation {
			std::vector<std::shared_ptr<DecklinkInfo>> decklink_list_;
			implementation()
			{ 
				Refresh();
			}

			static CComPtr<IDeckLinkIterator> create_iterator()
			{
				CComPtr<IDeckLinkIterator> pDecklinkIterator;
				if (FAILED(pDecklinkIterator.CoCreateInstance(CLSID_CDeckLinkIterator)))
					return nullptr;
				return pDecklinkIterator;
			}

			std::shared_ptr<DecklinkInfo>& operator[](size_t pos)
			{
				return decklink_list_.at(pos);
			}

			size_t Size() 
			{
				return decklink_list_.size();
			}

			void Refresh()
			{
				auto iterator = create_iterator();
				if (!iterator)
					return;
				IDeckLink* decklink;
				int index = 0;
				decklink_list_.clear();
				while (iterator->Next(&decklink) == S_OK)
					decklink_list_.push_back(std::make_shared<DecklinkInfo>(decklink, index++));
			}

			std::shared_ptr<ApiVersion> GetVersion()
			{
				auto iterator = create_iterator();
				if (!iterator)
					return nullptr;
				std::int64_t deckLinkVersion;
				CComQIPtr<IDeckLinkAPIInformation> deckLinkAPIInformation(iterator);
				if (deckLinkAPIInformation && SUCCEEDED(deckLinkAPIInformation->GetInt(BMDDeckLinkAPIVersion, &deckLinkVersion)))
					return std::make_shared<ApiVersion>(static_cast<int>((deckLinkVersion & 0xFF000000) >> 24), static_cast<int>((deckLinkVersion & 0x00FF0000) >> 16), static_cast<int>((deckLinkVersion & 0x0000FF00) >> 8));
				return nullptr;
			}

			std::shared_ptr<DecklinkOutput> CreateOutput(const DecklinkInfo& info, DecklinkKeyer keyer, TimecodeOutputSource timecode_source)
			{
				return std::make_shared<DecklinkOutput>(info.GetDecklink(), info.Index(), keyer, timecode_source);
			}

			std::shared_ptr<DecklinkInput> CreateInput(const DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video)
			{
				return std::make_shared<DecklinkInput>(info.GetDecklink(), format, audio_channels_count, timecode_source, capture_video);
			}

		};

		DecklinkIterator::DecklinkIterator()
			:impl_(std::make_unique<implementation>())
		{ }
		DecklinkIterator::~DecklinkIterator() {}
		std::shared_ptr<DecklinkInfo> DecklinkIterator::operator[](size_t pos) { return impl_->operator[](pos); }
		std::shared_ptr<DecklinkOutput> DecklinkIterator::CreateOutput(const DecklinkInfo& info, DecklinkKeyer keyer, TimecodeOutputSource timecode_source) { return impl_->CreateOutput(info, keyer, timecode_source); }
		std::shared_ptr<DecklinkInput> DecklinkIterator::CreateInput(const Decklink::DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count, TVPlayR::DecklinkTimecodeSource timecode_source, bool capture_video) { return impl_->CreateInput(info, format, audio_channels_count, timecode_source, capture_video); }
		size_t DecklinkIterator::Size() const { return impl_->Size(); }
		std::shared_ptr<ApiVersion> DecklinkIterator::GetVersion() { return impl_->GetVersion(); }
	}
}