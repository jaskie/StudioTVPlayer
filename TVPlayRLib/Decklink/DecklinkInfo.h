#pragma once

namespace TVPlayR {
	enum class DecklinkKeyerType;

	namespace Decklink {

		class DecklinkInfo final : Common::NonCopyable
		{
		public:
			explicit DecklinkInfo(IDeckLink* decklink, int index);
			~DecklinkInfo();
			std::wstring GetDisplayName() const;
			std::wstring GetModelName() const;
			bool SupportsKeyer(DecklinkKeyerType keyer);
			bool SupportsInputModeDetection();
			bool HaveInput() const;
			bool HaveOutput() const;
			int Index() const;
		private:
			IDeckLink* GetDecklink() const;
			struct implementation;
			std::unique_ptr<implementation> impl_;
			friend class DecklinkIterator;
		};

	}
}