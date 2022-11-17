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
		virtual std::shared_ptr<Core::OutputSink> GetNativeSink() override;
		DecklinkOutput(std::shared_ptr<Decklink::DecklinkOutput>& decklink);

	public:
		virtual void AddOverlay(OverlayBase^ overlay) override;
		virtual void RemoveOverlay(OverlayBase^ overlay) override;
		virtual void Initialize(VideoFormat^ format, PixelFormat pixelFormat, int audioChannelsCount, int audioSampleRate) override;
		~DecklinkOutput();
		!DecklinkOutput();
	};

}
