#include "stdafx.h"
#include "PreviewDevice.h"

using namespace System::Runtime::InteropServices;
using namespace System::Threading;

namespace TVPlayR
{
    PreviewDevice::PreviewDevice()
        : _preview(new std::shared_ptr<Preview::Preview>(std::make_shared<Preview::Preview>()))
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
        delete _preview;
        _preview = nullptr;
        _framePlayedHandle.Free();
    }

    void PreviewDevice::FramePlayedCallback(std::shared_ptr<AVFrame> frame)
    {
        WriteableBitmap^ target = _target;
        if (target == nullptr)
            return;
        target->Lock();
        try
        {
            if (target->Width != frame->width || target->Height != frame->height)
                return;
            int linesize = frame->linesize[0];
            auto rect = System::Windows::Int32Rect((target->PixelWidth - frame->width) / 2, (target->PixelHeight - frame->height) / 2, frame->width, frame->height);
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
