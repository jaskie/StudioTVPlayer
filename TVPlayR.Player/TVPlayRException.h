#pragma once
#include <exception>
#include "Common/Exceptions.h"

namespace TVPlayR {
	public ref class TVPlayRException : public System::Exception
	{
	private:
		const System::String^ _stack_trace;
	public:
		TVPlayRException(Common::TVPlayRException e)
			: System::Exception(gcnew System::String(e.what()))
			, _stack_trace(gcnew System::String(e.StackTrace()))
		{
		}
		TVPlayRException(std::exception e)
			: System::Exception(gcnew System::String(e.what())) 
		{ }

	};
}

#define REWRAP_EXCEPTION(statement) try { statement } catch (TVPlayR::Common::TVPlayRException te) { throw gcnew TVPlayRException(te); } catch (std::exception e) { throw gcnew TVPlayRException(e); }