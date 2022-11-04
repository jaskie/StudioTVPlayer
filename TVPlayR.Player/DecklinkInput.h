#pragma once
#include "InputBase.h"

using namespace System; 
using namespace System::Runtime::InteropServices;

namespace TVPlayR {
	ref class OutputSink;
	ref class VideoFormatEventArgs;
	enum class DecklinkTimecodeSource;

	namespace Decklink {
		class DecklinkInput;
	}
	namespace Core {
		enum class VideoFormatType;
	}

	public ref class DecklinkInput : public InputBase
	{
	private:
		delegate void FormatChangedDelegate(Core::VideoFormatType);
		FormatChangedDelegate^ _formatChangedDelegate;
		String^ _modelName;
		GCHandle _formatChangedHandle;
		const std::shared_ptr<Decklink::DecklinkInput> GetDecklinkInput();
		void FormatChangedCallback(Core::VideoFormatType newFormat);
	protected:
		virtual String^ GetName() override { return _modelName; }
	internal:
		DecklinkInput(std::shared_ptr<Decklink::DecklinkInput>& decklink, String^ modelName);
	public:
		void AddOutputSink(OutputSink^ output_sink);
		void RemoveOutputSink(OutputSink^ output_sink);
		~DecklinkInput();
		!DecklinkInput();
		event EventHandler<VideoFormatEventArgs^>^ FormatChanged;
	};

}
