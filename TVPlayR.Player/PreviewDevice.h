#pragma once
#include "Preview/Preview.h"
using namespace System;

namespace TVPlayR {
	
	public ref class PreviewDevice
	{
	private:
		Preview::Preview* _preview;

	public:
		PreviewDevice();
		~PreviewDevice();
		!PreviewDevice();
	};

}