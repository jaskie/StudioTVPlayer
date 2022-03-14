#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Media::Imaging;
namespace TVPlayR {

	namespace Preview {
		class InputPreview;
	}

	public ref class InputPreview sealed
	{
	private:
		Preview::InputPreview* _preview;
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
		virtual Preview::InputPreview& GetNative();
	public:
		InputPreview(System::Windows::Threading::Dispatcher^ ui_dispatcher, int width, int height);
		~InputPreview();
		!InputPreview();
		property WriteableBitmap^ PreviewSource
		{
			WriteableBitmap^ get() { return _target; }
		}
	};
}
