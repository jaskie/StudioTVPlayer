#pragma once
#include "OutputBase.h"

using namespace System;

namespace TVPlayR {
	namespace Decklink {
		class DecklinkOutput;
	}

	public ref class DecklinkOutput : public OutputBase
	{
	private:
		const std::shared_ptr<Decklink::DecklinkOutput>* _decklink;

	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override;
		DecklinkOutput(std::shared_ptr<Decklink::DecklinkOutput>& decklink);

	public:
		~DecklinkOutput();
		!DecklinkOutput();
	};

}
