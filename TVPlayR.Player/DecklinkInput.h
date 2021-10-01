#pragma once
#include "InputBase.h"
#include "Decklink/DecklinkInput.h"

using namespace System; 
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	ref class InputPreview;
	ref class VideoFormatEventArgs;

	namespace Decklink {
		class DecklinkInput;
	}
	public enum class DecklinkTimecodeSource {
		None,
		RP188Any,
		VITC
	};

	public ref class DecklinkInput : public InputBase
	{
	private:
		delegate void FormatChangedDelegate(Core::VideoFormatType);
		FormatChangedDelegate^ _formatChangedDelegate;
		String^ _modelName;
		GCHandle _formatChangedHandle;
		const std::shared_ptr<Decklink::DecklinkInput>* _decklink;
		void FormatChangedCallback(Core::VideoFormatType newFormat);
	protected:
		virtual String^ GetName() override { return _modelName; }
	internal:
		DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink, String^ modelName);
		virtual std::shared_ptr<Core::InputSource> GetNativeSource() override { return _decklink ? *_decklink : nullptr; }
	public:
		void AddPreview(InputPreview^ preview);
		void RemovePreview(InputPreview^ preview);
		~DecklinkInput();
		!DecklinkInput();
		event EventHandler<VideoFormatEventArgs^>^ FormatChanged;
	};

}
