#pragma once

#include "OutputBase.h"

using namespace System;

namespace TVPlayR {
	namespace Ndi {
		class NdiOutput;
	}

	public ref class NdiOutput sealed : public OutputBase
	{
	private:
		const std::shared_ptr<Ndi::NdiOutput>* _ndi;
		String^ _sourceName;
		String^ _groupNames;		
	public:
		NdiOutput(String^ sourceName, String^ groupNames);
		~NdiOutput();
		!NdiOutput();
		property String^ SourceName
		{
			String^ get() { return _sourceName; }
		}
		property String^ GroupNames
		{
			String^ get() { return _groupNames; }
		}
		void AddOverlay(OverlayBase^ overlay) override;
		void RemoveOverlay(OverlayBase^ overlay) override;
		void Initialize(VideoFormat^ format, PixelFormat pixelFormat, int audioChannelsCount, int audioSampleRate) override;

	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override;
		virtual std::shared_ptr<Core::OutputSink> GetNativeSink() override;
	};

}