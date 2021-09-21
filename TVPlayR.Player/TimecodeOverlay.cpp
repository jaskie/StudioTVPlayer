#include "stdafx.h"
#include "TimecodeOverlay.h"
#include "Core/TimecodeOverlay.h"
#include "VideoFormat.h"

namespace TVPlayR {
    
    TimecodeOverlay::TimecodeOverlay(VideoFormat^ video_format, bool no_passthrough_video)
        : _native_object(new std::shared_ptr<Core::TimecodeOverlay>(new Core::TimecodeOverlay(video_format->GetNativeEnumType(), no_passthrough_video)))
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