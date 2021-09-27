#pragma once
#include "OverlayBase.h"
#include "Core/TimecodeOverlay.h"

using namespace System;

namespace TVPlayR {
    ref class VideoFormat;
    enum class PixelFormat;

    public ref class TimecodeOverlay : public OverlayBase
    {
    public:
        TimecodeOverlay(VideoFormat^ videoFormat, PixelFormat pixelFormat);
    public:
        ~TimecodeOverlay();
        !TimecodeOverlay();
    internal:
        virtual std::shared_ptr<Core::OverlayBase> GetNativeObject() override;
    private:
        std::shared_ptr<Core::TimecodeOverlay>* _native_object;
    };
}
