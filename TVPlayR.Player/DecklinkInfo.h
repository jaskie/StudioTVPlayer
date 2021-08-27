#pragma once
#include "stdafx.h"
#include "Decklink/DecklinkInfo.h"

namespace TVPlayR {
	public ref class DecklinkInfo sealed
	{
	public:
		~DecklinkInfo() {
			this->!DecklinkInfo();
		}
		!DecklinkInfo() 
		{
			if (!native_info_)
				return;
			delete native_info_;
			native_info_ = nullptr;
		}

		property int Index {
			int get() { return (*native_info_)->Index(); }
		}

		property System::String^ DisplayName {
			System::String^ get()
			{
				return gcnew System::String((*native_info_)->GetDisplayName().c_str());
			}
		}

		property System::String^ ModelName {
			System::String^ get()
			{
				return gcnew System::String((*native_info_)->GetModelName().c_str());
			}
		}
		
	internal:
		DecklinkInfo(std::shared_ptr<Decklink::DecklinkInfo>& info)
			: native_info_(new std::shared_ptr<Decklink::DecklinkInfo>(info))
		{ }

	private:
		std::shared_ptr<Decklink::DecklinkInfo>* native_info_;
	};

}