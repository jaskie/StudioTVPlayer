#pragma once
#include "OverlayBase.h"
#include "Core/TimecodeOverlay.h"

using namespace System;

namespace TVPlayR {
    ref class VideoFormat;

    ref class TimecodeOverlay : public OverlayBase
    {
    public:
        TimecodeOverlay(VideoFormat^ video_format, bool no_passthrough_video);
    public:
        ~TimecodeOverlay();
        !TimecodeOverlay();
    internal:
        virtual std::shared_ptr<Core::OverlayBase> GetNativeObject() override;
    private:
        std::shared_ptr<Core::TimecodeOverlay>* _native_object;
    };
}
