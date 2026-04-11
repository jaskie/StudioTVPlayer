#pragma once

namespace TVPlayR {

    namespace Core {
        class VideoFormat;
    }

	namespace Decklink {
        class DecklinkTimecode final : public IDeckLinkTimecode
        {
        private:
            std::int64_t time_;
            const Core::VideoFormat &format_;
        public:
            DecklinkTimecode(const Core::VideoFormat &format);
            bool IsValid() const;
            void Update(std::int64_t time);
            //IUnknown
            STDMETHOD(QueryInterface(REFIID, LPVOID*));
            STDMETHOD_(ULONG, AddRef());
            STDMETHOD_(ULONG, Release());
            //IDecklinkTimeCode
            STDMETHOD_(BMDTimecodeBCD, GetBCD());
            STDMETHOD(GetComponents(unsigned char* hours, unsigned char* minutes, unsigned char* seconds, unsigned char* frames));
            STDMETHOD(GetString(/* [out] */ BSTR* timecode));
            STDMETHOD_(BMDTimecodeFlags, GetFlags());
            STDMETHOD(GetTimecodeUserBits(/* [out] */ BMDTimecodeUserBits* userBits));
        };

}}

