#include "stdafx.h"
#include "PreviewDevice.h"

using namespace System::Runtime::InteropServices;
using namespace System::Threading;

namespace TVPlayR
{
    PreviewDevice::PreviewDevice(System::Windows::Threading::Dispatcher^ ui_dispatcher)
        : _preview(new std::shared_ptr<Preview::Preview>(std::make_shared<Preview::Preview>()))
        , _ui_dispatcher(ui_dispatcher)
        , _draw_frame_action(gcnew Action(this, &PreviewDevice::DrawFrame))
        , _frame_played_semaphore(1)
    {
        _framePlayedDelegate = gcnew FramePlayedDelegate(this, &PreviewDevice::FramePlayedCallback);
        _framePlayedHandle = GCHandle::Alloc(_framePlayedDelegate);
        IntPtr framePlayedIp = Marshal::GetFunctionPointerForDelegate(_framePlayedDelegate);
        (*_preview)->SetFramePlayedCallback(static_cast<Preview::Preview::FRAME_PLAYED_CALLBACK>(framePlayedIp.ToPointer()));
    }

    PreviewDevice::~PreviewDevice()
    {
        this->!PreviewDevice();
    }

    PreviewDevice::!PreviewDevice()
    {
        if (!_preview)
            return;
        _shutdown_cts.Cancel();
        (*_preview)->SetFramePlayedCallback(nullptr);
        _framePlayedHandle.Free();
        delete _preview;
        _preview = nullptr;
    }

    void PreviewDevice::FramePlayedCallback(std::shared_ptr<AVFrame> frame)
    {
        try
        {
            _frame_played_semaphore.Wait(_shutdown_cts.Token);
            assert(!_buffer_frame);
            _buffer_frame = new std::shared_ptr<AVFrame>(std::move(frame));
            _ui_dispatcher->BeginInvoke(_draw_frame_action);
        }
        catch (System::OperationCanceledException^)
        { }
    }

    void PreviewDevice::DrawFrame()
    {
        std::shared_ptr<AVFrame> frame = *_buffer_frame;
        delete _buffer_frame;
        _buffer_frame = nullptr;
        _frame_played_semaphore.Release();
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

    void PreviewDevice::CreatePreview(int width, int height)
    {
        _target = gcnew WriteableBitmap(width, height, 96, 96, Windows::Media::PixelFormats::Pbgra32, nullptr);
        (*_preview)->CreateFilter(width, height);
    }


}
