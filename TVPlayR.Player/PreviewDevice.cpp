#include "stdafx.h"
#include "PreviewDevice.h"

namespace TVPlayR
{

    PreviewDevice::PreviewDevice()
        : _preview(new Preview::Preview())
    {

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
    }

}
