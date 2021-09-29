#include "../pch.h"
#include "DecklinkInfo.h"

namespace TVPlayR {
	namespace Decklink {

		struct DecklinkInfo::implementation
		{
			CComPtr<IDeckLink> const decklink_;
			const int index_;

			implementation(IDeckLink* decklink, int index)
				: decklink_(decklink)
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
		};

		DecklinkInfo::DecklinkInfo(IDeckLink* decklink, int index)
			:impl_(std::make_unique<implementation>(decklink, index))
		{
		}

		DecklinkInfo::~DecklinkInfo() { }
		std::wstring DecklinkInfo::GetDisplayName() const { return impl_->GetDisplayName(); }
		std::wstring DecklinkInfo::GetModelName() const { return impl_->GetModelName(); }
		int DecklinkInfo::Index() const { return impl_->index_; }
		IDeckLink* DecklinkInfo::GetDecklink() const { return impl_->decklink_; }
	}
}