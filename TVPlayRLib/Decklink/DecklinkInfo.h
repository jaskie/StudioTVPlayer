#pragma once

namespace TVPlayR {
	namespace Decklink {

		class DecklinkInfo final
		{
		public:
			explicit DecklinkInfo(IDeckLink* decklink, int index);
			~DecklinkInfo();
			std::wstring GetDisplayName() const;
			std::wstring GetModelName() const;
			int Index() const;
		private:
			IDeckLink* GetDecklink() const;
			struct implementation;
			std::unique_ptr<implementation> impl_;
			friend class DecklinkIterator;
		};

	}
}