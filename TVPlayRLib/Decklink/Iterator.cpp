#include "../pch.h"
#include "Decklink.h"
#include "Iterator.h"
#include "../Common/Exceptions.h"
#include "../Common/ComInitializer.h"

namespace TVPlayR {
	namespace Decklink {

		static CComPtr<IDeckLinkIterator> create_iterator()
		{
			CComPtr<IDeckLinkIterator> pDecklinkIterator;
			if (FAILED(pDecklinkIterator.CoCreateInstance(CLSID_CDeckLinkIterator)))
				return nullptr;
			return pDecklinkIterator;
		}

		static ApiVersion get_version(const CComPtr<IDeckLinkIterator>& iterator)
		{
			
		}

		struct Iterator::implementation {
			Common::ComInitializer com_;
			CComPtr<IDeckLinkIterator> decklink_iterator_;
			std::vector< std::shared_ptr<Decklink>> decklink_list_;
			implementation()
			{
				Refresh();
			}

			std::shared_ptr<Decklink>& operator[](size_t pos)
			{
				return decklink_list_.at(pos);
			}

			size_t Size() 
			{
				return decklink_list_.size();
			}

			void Refresh()
			{
				decklink_iterator_ = create_iterator();
				if (!decklink_iterator_)
					return;
				IDeckLink* decklink;
				int index = 0;
				while (decklink_iterator_->Next(&decklink) == S_OK)
					decklink_list_.push_back(std::make_shared<Decklink>(decklink, index++));
			}

			ApiVersion GetVersion()
			{
				if (decklink_iterator_)
				{
				int64_t deckLinkVersion;
				CComQIPtr<IDeckLinkAPIInformation> deckLinkAPIInformation(decklink_iterator_);
				if (deckLinkAPIInformation && SUCCEEDED(deckLinkAPIInformation->GetInt(BMDDeckLinkAPIVersion, &deckLinkVersion)))
					return ApiVersion((deckLinkVersion & 0xFF000000) >> 24, (deckLinkVersion & 0x00FF0000) >> 16, (deckLinkVersion & 0x0000FF00) >> 8);
				}
				return ApiVersion(0, 0, 0);
			}

		};

		Iterator::Iterator()
			:impl_(new implementation())
		{ }

		Iterator::~Iterator() { }

		std::shared_ptr<Decklink> Iterator::operator[](size_t pos) { return impl_->operator[](pos); }
		size_t Iterator::Size() const { return impl_->Size(); }
		ApiVersion Iterator::GetVersion() { return impl_->GetVersion(); }
		void Iterator::Refresh() { return impl_->Refresh();	}
	}
}