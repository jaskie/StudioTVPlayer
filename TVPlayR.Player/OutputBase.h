#pragma once

using namespace System;

namespace TVPlayR {
	ref class OverlayBase;
	namespace Core {
		class OutputSink;
		class OutputDevice;
	}

	public ref class OutputSink abstract {
	internal:
		virtual std::shared_ptr<Core::OutputSink> GetNativeSink() abstract;
	};

	public ref class OutputBase abstract: public OutputSink
	{
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() abstract;
	public:
		virtual void AddOverlay(OverlayBase^ overlay) abstract;
		virtual void RemoveOverlay(OverlayBase^ overlay) abstract;
	};
}