#pragma once
#include "OutputBase.h"
#include "Decklink/DecklinkOutput.h"

using namespace System;


namespace TVPlayR {
	public ref class DecklinkOutput : public OutputBase
	{
	private:
		const std::shared_ptr<Decklink::DecklinkOutput>* _decklink;

	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return *_decklink; }
		DecklinkOutput(std::shared_ptr<Decklink::DecklinkOutput>& decklink);

	public:
		~DecklinkOutput();
		!DecklinkOutput();
	};

}
