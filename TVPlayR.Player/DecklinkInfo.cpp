#include "stdafx.h"
#include "DecklinkInfo.h"
#include "Decklink/DecklinkInfo.h"
#include "DecklinkKeyer.h"

namespace TVPlayR
{
	DecklinkInfo::DecklinkInfo(std::shared_ptr<Decklink::DecklinkInfo>& info)
		: native_info_(new std::shared_ptr<Decklink::DecklinkInfo>(info))
	{ }

	DecklinkInfo::!DecklinkInfo()
	{
		if (!native_info_)
			return;
		delete native_info_;
		native_info_ = nullptr;
	}


	int DecklinkInfo::Index::get()
	{
		return (*native_info_)->Index();
	}

	System::String^  DecklinkInfo::DisplayName::get()
	{
		return gcnew System::String((*native_info_)->GetDisplayName().c_str());
	}

	System::String^ DecklinkInfo::ModelName::get()
	{
		return gcnew System::String((*native_info_)->GetModelName().c_str());
	}

	bool DecklinkInfo::SupportsKeyer(DecklinkKeyer keyer)
	{
		return (*native_info_)->SupportsKeyer(keyer);
	}

	const std::shared_ptr<Decklink::DecklinkInfo> DecklinkInfo::GetNativeInfo() { return *native_info_; }


}