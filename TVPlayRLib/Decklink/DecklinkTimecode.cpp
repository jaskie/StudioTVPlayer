#include "../pch.h"
#include "../Core/VideoFormat.h"
#include "DecklinkTimecode.h"

namespace TVPlayR {
    namespace Decklink {
        DecklinkTimecode::DecklinkTimecode(Core::VideoFormat &format)
            : time_(AV_NOPTS_VALUE)
            , format_(format)
        { }

        bool DecklinkTimecode::IsValid() const
        {
            return time_ != AV_NOPTS_VALUE;
        }

        void DecklinkTimecode::Update(Core::VideoFormat &format, std::int64_t time)
        {
            format_ = format;
            time_ = time;
        }

        STDMETHODIMP DecklinkTimecode::QueryInterface(REFIID, LPVOID*) { return E_NOINTERFACE; }

        STDMETHODIMP_(ULONG) DecklinkTimecode::AddRef() { return 1; }

        STDMETHODIMP_(ULONG) DecklinkTimecode::Release() { return 1; }

        STDMETHODIMP_(BMDTimecodeBCD) DecklinkTimecode::GetBCD()
        {
            int frame_number = format_.TimeToFrameNumber(time_);
            auto smpte = format_.FrameNumberToSmpteTimecode(frame_number);
            BMDTimecodeBCD bcd;
            uint8_t* out;
            auto in = reinterpret_cast<uint8_t*>(&smpte);
            out = reinterpret_cast<uint8_t*>(&bcd);
            out[0] = in[3];
            out[1] = in[2];
            out[2] = in[1];
            out[3] = in[0];
            return bcd;
        }

        STDMETHODIMP DecklinkTimecode::GetComponents(unsigned char *hours, unsigned char *minutes, unsigned char *seconds, unsigned char *frames)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP DecklinkTimecode::GetString(BSTR* timecode)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP_(BMDTimecodeFlags) DecklinkTimecode::GetFlags()
        {
            return BMDTimecodeFlags();
        }

        STDMETHODIMP DecklinkTimecode::GetTimecodeUserBits(BMDTimecodeUserBits *userBits)
        {
            if (userBits == nullptr)
                return E_FAIL;
            *userBits = BMDTimecodeUserBits();
            return S_OK;
        }

    }
}