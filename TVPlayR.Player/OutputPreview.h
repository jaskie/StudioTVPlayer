#pragma once
#include "OutputBase.h"
#include "Preview/OutputPreview.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Media::Imaging;
namespace TVPlayR {
	
	public ref class OutputPreview sealed : public OutputBase
	{
	private:
		const std::shared_ptr<Preview::OutputPreview>* _preview;
		WriteableBitmap^ _target;
		std::shared_ptr<AVFrame>* _buffer_frame = nullptr;
		delegate void FramePlayedDelegate(std::shared_ptr<AVFrame>);
		FramePlayedDelegate^ _framePlayedDelegate;
		GCHandle _framePlayedHandle;
		Action^ _draw_frame_action;
		System::Threading::SemaphoreSlim^ _frame_played_semaphore;
		System::Threading::CancellationTokenSource^ _shutdown_cts;
		System::Windows::Threading::Dispatcher^ _ui_dispatcher;
		void FramePlayedCallback(std::shared_ptr<AVFrame> frame);
		void DrawFrame();
	internal:
		virtual std::shared_ptr<Core::OutputDevice> GetNativeDevice() override { return _preview == nullptr ? nullptr : *_preview; }
	public:
		OutputPreview(System::Windows::Threading::Dispatcher^ ui_dispatcher, int width, int height);
		~OutputPreview();
		!OutputPreview();
		property WriteableBitmap^ PreviewSource
		{
			WriteableBitmap^ get() { return _target; }
		}
	};

}