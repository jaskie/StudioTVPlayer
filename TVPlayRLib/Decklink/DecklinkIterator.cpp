#include "../pch.h"
#include "DecklinkOutput.h"
#include "DecklinkInfo.h"
#include "DecklinkIterator.h"
#include "DecklinkInput.h"
#include "../Common/Exceptions.h"
#include "../Common/ComInitializer.h"
#include "../Core/VideoFormat.h"

namespace TVPlayR {
	namespace Decklink {

		static CComPtr<IDeckLinkIterator> create_iterator()
		{
			CComPtr<IDeckLinkIterator> pDecklinkIterator;
			if (FAILED(pDecklinkIterator.CoCreateInstance(CLSID_CDeckLinkIterator)))
				return nullptr;
			return pDecklinkIterator;
		}

		struct DecklinkIterator::implementation {
			Common::ComInitializer com_;
			const CComPtr<IDeckLinkIterator> decklink_iterator_;
			std::vector<std::shared_ptr<DecklinkInfo>> decklink_list_;
			implementation()
				: decklink_iterator_(create_iterator())
			{ 
				Refresh();
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
				if (!decklink_iterator_)
					return;
				IDeckLink* decklink;
				int index = 0;
				while (decklink_iterator_->Next(&decklink) == S_OK)
					decklink_list_.push_back(std::make_shared<DecklinkInfo>(decklink, index++));
			}

			std::shared_ptr<ApiVersion> GetVersion()
			{
				if (!decklink_iterator_)
					return nullptr;
				int64_t deckLinkVersion;
				CComQIPtr<IDeckLinkAPIInformation> deckLinkAPIInformation(decklink_iterator_);
				if (deckLinkAPIInformation && SUCCEEDED(deckLinkAPIInformation->GetInt(BMDDeckLinkAPIVersion, &deckLinkVersion)))
					return std::make_shared<ApiVersion>(static_cast<int>((deckLinkVersion & 0xFF000000) >> 24), static_cast<int>((deckLinkVersion & 0x00FF0000) >> 16), static_cast<int>((deckLinkVersion & 0x0000FF00) >> 8));
				return nullptr;
			}

			std::shared_ptr<DecklinkOutput> CreateOutput(const DecklinkInfo& info)
			{
				return std::make_shared<DecklinkOutput>(info.GetDecklink(), info.Index());
			}

			std::shared_ptr<DecklinkInput> CreateInput(const DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count)
			{
				return std::make_shared<DecklinkInput>(info.GetDecklink(), format, audio_channels_count);
			}

		};

		DecklinkIterator::DecklinkIterator()
			:impl_(std::make_unique<implementation>())
		{ }
		DecklinkIterator::~DecklinkIterator() {}
		std::shared_ptr<DecklinkInfo> DecklinkIterator::operator[](size_t pos) { return impl_->operator[](pos); }
		std::shared_ptr<DecklinkOutput> DecklinkIterator::CreateOutput(const DecklinkInfo& info) { return impl_->CreateOutput(info); }
		std::shared_ptr<DecklinkInput> DecklinkIterator::CreateInput(const DecklinkInfo& info, Core::VideoFormatType format, int audio_channels_count) { return impl_->CreateInput(info, format, audio_channels_count); }
		size_t DecklinkIterator::Size() const { return impl_->Size(); }
		std::shared_ptr<ApiVersion> DecklinkIterator::GetVersion() { return impl_->GetVersion(); }
	}
}