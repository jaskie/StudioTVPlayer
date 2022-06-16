#include "../pch.h"
#include "DecklinkInfo.h"
#include "../DecklinkKeyer.h"


namespace TVPlayR {
	namespace Decklink {

		struct DecklinkInfo::implementation
		{
			const int index_;
			CComPtr<IDeckLink> const decklink_;
			const CComQIPtr<IDeckLinkAttributes> attributes_;
			CComQIPtr<IDeckLinkInput> const input_;
			CComQIPtr<IDeckLinkOutput> const output_;

			implementation(IDeckLink* decklink, int index)
				: decklink_(decklink)
				, attributes_(decklink)
				, input_(decklink)
				, output_(decklink)
				, index_(index)
			{ }

			std::wstring GetDisplayName() const
			{
				CComBSTR pModelName;
				if (FAILED(decklink_->GetDisplayName(&pModelName)))
					return L"";
				return std::wstring(pModelName);
			}

			std::wstring GetModelName() const
			{
				CComBSTR pModelName;
				if (FAILED(decklink_->GetModelName(&pModelName)))
					return L"";
				return std::wstring(pModelName);
			}

			bool SupportsKeyer(DecklinkKeyer keyer)
			{
				BOOL support = FALSE;
				switch (keyer)
				{
				case DecklinkKeyer::Default:
					return true;
				case DecklinkKeyer::Internal:
					if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsInternalKeying, &support)) && support)
						return true;
					break;
				case DecklinkKeyer::External:
					if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsExternalKeying, &support)) && support)
						return true;
					break;
				}
				return false;
			}
		};

		DecklinkInfo::DecklinkInfo(IDeckLink* decklink, int index)
			:impl_(std::make_unique<implementation>(decklink, index))
		{
		}

		DecklinkInfo::~DecklinkInfo() { }
		std::wstring DecklinkInfo::GetDisplayName() const { return impl_->GetDisplayName(); }
		std::wstring DecklinkInfo::GetModelName() const { return impl_->GetModelName(); }
		bool DecklinkInfo::SupportsKeyer(DecklinkKeyer keyer) { return impl_->SupportsKeyer(keyer); }
		bool DecklinkInfo::HaveInput() const { return impl_->input_; }
		bool DecklinkInfo::HaveOutput() const { return impl_->output_; }
		int DecklinkInfo::Index() const { return impl_->index_; }
		IDeckLink* DecklinkInfo::GetDecklink() const { return impl_->decklink_; }
	}
}