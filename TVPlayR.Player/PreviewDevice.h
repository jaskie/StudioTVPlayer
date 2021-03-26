#pragma once
#include "Preview/Preview.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Media::Imaging;
namespace TVPlayR {
	
	public ref class PreviewDevice
	{
	private:
		std::shared_ptr<Preview::Preview>* _preview;
		WriteableBitmap^ _target;
		delegate void FramePlayedDelegate(std::shared_ptr<AVFrame>);
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		void FramePlayedCallback(std::shared_ptr<AVFrame> frame);
	internal:
		std::shared_ptr<Preview::Preview> GetNativeDevice() { return *_preview; }
	public:
		property WriteableBitmap^ PreviewSource 
		{
			WriteableBitmap^ get() { return _target; }
		}
		void CreatePreview(int width, int height);
		PreviewDevice();
		~PreviewDevice();
		!PreviewDevice();
	};

}