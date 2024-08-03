#include "../pch.h"
#include "../Core/VideoFormat.h"
#include "DecklinkTimecode.h"

namespace TVPlayR {
    namespace Decklink {
        DecklinkTimecode::DecklinkTimecode(const Core::VideoFormat &format)
            : time_(AV_NOPTS_VALUE)
            , format_(format)
        { }

        bool DecklinkTimecode::IsValid() const
        {
            return time_ != AV_NOPTS_VALUE;
        }

        void DecklinkTimecode::Update(std::int64_t time)
        {
            time_ = time;
        }

        STDMETHODIMP DecklinkTimecode::QueryInterface(REFIID, LPVOID*) { return E_NOINTERFACE; }

        STDMETHODIMP_(ULONG) DecklinkTimecode::AddRef() { return 1; }

        STDMETHODIMP_(ULONG) DecklinkTimecode::Release() { return 1; }

        STDMETHODIMP_(BMDTimecodeBCD) DecklinkTimecode::GetBCD()
        {
            if (time_ == AV_NOPTS_VALUE)
                return 0;
            int frame_number = format_.TimeToFrameNumber(time_);
            uint32_t smpte = format_.FrameNumberToSmpteTimecode(frame_number);
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

        STDMETHODIMP DecklinkTimecode::GetComponents(unsigned char* hours, unsigned char* minutes, unsigned char* seconds, unsigned char* frames)
        {
            if (time_ == AV_NOPTS_VALUE)
                return E_FAIL;
            int frame_number = format_.TimeToFrameNumber(time_);
            uint32_t smpte = format_.FrameNumberToSmpteTimecode(frame_number);
            auto in = reinterpret_cast<uint8_t*>(&smpte);
            if (hours)
                *hours   = in[0];
            if (minutes)
                *minutes = in[1];
            if (seconds)
                *seconds = in[2];
            if (frames)
                *frames  = in[3];
            return S_OK;
        }

        STDMETHODIMP DecklinkTimecode::GetString(BSTR* timecode)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP_(BMDTimecodeFlags) DecklinkTimecode::GetFlags()
        {
            return 0;
        }

        STDMETHODIMP DecklinkTimecode::GetTimecodeUserBits(BMDTimecodeUserBits* userBits)
        {
            return E_POINTER;
        }

    }
}