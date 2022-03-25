#include "stdafx.h"
#include "InputPreview.h"
#include "Preview/InputPreview.h"

using namespace System::Runtime::InteropServices;
using namespace System::Threading;

namespace TVPlayR
{
    InputPreview::InputPreview(System::Windows::Threading::Dispatcher^ ui_dispatcher, int width, int height)
        : _preview(new Preview::InputPreview(width, height))
        , _ui_dispatcher(ui_dispatcher)
        , _draw_frame_action(gcnew Action(this, &InputPreview::DrawFrame))
        , _frame_played_semaphore(gcnew System::Threading::SemaphoreSlim(1))
        , _shutdown_cts(gcnew System::Threading::CancellationTokenSource())
    {
        _framePlayedDelegate = gcnew FramePlayedDelegate(this, &InputPreview::FramePlayedCallback);
        _framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
        IntPtr framePlayedIp = Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
        _target = gcnew WriteableBitmap(width, height, 96, 96, Windows::Media::PixelFormats::Pbgra32, nullptr);
        _preview->SetFramePlayedCallback(static_cast<Preview::InputPreview::FRAME_PLAYED_CALLBACK>(framePlayedIp.ToPointer()));
    }

    InputPreview::~InputPreview()
    {
        this->!InputPreview();
        delete _preview;
        _preview = nullptr;
        delete _buffer_frame;
        _buffer_frame = nullptr;
    }

    InputPreview::!InputPreview()
    {
        if (!_preview)
            return;
        _shutdown_cts->Cancel();
        _framePlayedHandle.Free();
    }

    void InputPreview::FramePlayedCallback(std::shared_ptr<AVFrame> frame)
    {
        if (_shutdown_cts->IsCancellationRequested)
            return;
        try
        {
            _frame_played_semaphore->Wait(_shutdown_cts->Token);
            if (_shutdown_cts->IsCancellationRequested)
                return;
            System::Diagnostics::Debug::Assert(!_buffer_frame);
            _buffer_frame = new std::shared_ptr<AVFrame>(std::move(frame));
            _ui_dispatcher->BeginInvoke(_draw_frame_action);
        }
        catch (System::OperationCanceledException^)
        {
        }
    }

    void InputPreview::DrawFrame()
    {
        if (!_buffer_frame)
            return;
        std::shared_ptr<AVFrame> frame = *_buffer_frame;
        delete _buffer_frame;
        _buffer_frame = nullptr;
        _frame_played_semaphore->Release();
        WriteableBitmap^ target = _target;
        if (target == nullptr)
            return;
        target->Lock();
        try
        {
            if (target->Width != frame->width || target->Height != frame->height)
                return;
            int linesize = frame->linesize[0];
            auto rect = System::Windows::Int32Rect(0, 0, frame->width, frame->height);
            auto datalength = frame->height * linesize;
            auto data = IntPtr(frame->data[0]);
            target->WritePixels(rect, data, datalength, linesize);
        }
        finally
        {
            target->Unlock();
        }
    }
    Preview::InputPreview& InputPreview::GetNative()
    {
        return *_preview;
    }
}