#include "stdafx.h"
#include "TimecodeOverlay.h"
#include "Core/TimecodeOverlay.h"
#include "VideoFormat.h"
#include "PixelFormat.h"
#include "TimecodeOutputSource.h"

namespace TVPlayR {
    
    TimecodeOverlay::TimecodeOverlay(const TimecodeOutputSource source, VideoFormat^ videoFormat, const PixelFormat pixelFormat)
        : _native_object(new std::shared_ptr<Core::TimecodeOverlay>(new Core::TimecodeOverlay(source, videoFormat->GetNativeEnumType(), pixelFormat)))
    { }
    
    TimecodeOverlay::~TimecodeOverlay()
    {
        this->!TimecodeOverlay();
    }

    TimecodeOverlay::!TimecodeOverlay()
    {
        if (!_native_object)
            return;
        delete _native_object;
        _native_object = nullptr;
    }

    std::shared_ptr<Core::OverlayBase> TVPlayR::TimecodeOverlay::GetNativeObject()
    {
        return _native_object ? *_native_object : nullptr;
    }
}