#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	
	enum class DecklinkKeyer;

	namespace Decklink {
		class DecklinkInfo;
	}
	

	public ref class DecklinkInfo sealed
	{
	private:
		std::shared_ptr<Decklink::DecklinkInfo>* native_info_;

	public:
		~DecklinkInfo() {
			this->!DecklinkInfo();
		}
		
		!DecklinkInfo();

		property int Index { int get(); }

		property System::String^ DisplayName { System::String^ get(); }

		property System::String^ ModelName { System::String^ get(); }
		
		property bool HaveOutput { bool get(); }

		property bool HaveInput { bool get(); }

		bool SupportsKeyer(DecklinkKeyer keyer);
		
	internal:
		DecklinkInfo(std::shared_ptr<Decklink::DecklinkInfo>& info);
	
		const std::shared_ptr<Decklink::DecklinkInfo> GetNativeInfo();

	};

}