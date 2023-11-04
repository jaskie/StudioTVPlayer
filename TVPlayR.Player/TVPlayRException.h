#pragma once
#include <exception>
#include "Common/Exceptions.h"

namespace TVPlayR {
	public ref class TVPlayRException : public System::Exception
	{
	public:
		TVPlayRException(std::exception e)
			: System::Exception(gcnew System::String(e.what())) 
		{ }
		TVPlayRException(const char* what)
			: System::Exception(gcnew System::String(what))
		{
		}
	};
}

#define REWRAP_EXCEPTION(statement)\
 try { statement }\
  catch (std::exception e)\
  {\
   throw gcnew TVPlayRException(e);\
  }