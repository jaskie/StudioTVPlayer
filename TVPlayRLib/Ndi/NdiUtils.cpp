#include "../pch.h"
#include "NdiUtils.h"

namespace TVPlayR {
	namespace Ndi {

		NDIlib_v4* LoadNdi()
		{
			static NDIlib_v4* ndi_library = nullptr;
			if (ndi_library)
				return ndi_library;
			HMODULE hNDILib = LoadLibraryA(NDILIB_LIBRARY_NAME);
			if (!hNDILib)
			{
				size_t required_size = 0;
				if (getenv_s(&required_size, NULL, 0, NDILIB_REDIST_FOLDER) != 0 || required_size == 0)
					return nullptr;
				char* p_ndi_runtime_v4 = (char*)malloc(required_size * sizeof(char));
				if (!p_ndi_runtime_v4
					|| getenv_s(&required_size, p_ndi_runtime_v4, required_size, NDILIB_REDIST_FOLDER) != 0
					|| !p_ndi_runtime_v4)
				{
					free(p_ndi_runtime_v4);
					return nullptr;
				}
				std::string ndi_path(p_ndi_runtime_v4);
				free(p_ndi_runtime_v4);
				ndi_path += "\\" NDILIB_LIBRARY_NAME;
				hNDILib = LoadLibraryA(ndi_path.c_str());
				if (!hNDILib)
					return nullptr;
			}
			NDIlib_v4* (*NDIlib_v4_load)(void) = NULL;
			if (hNDILib)
				*((FARPROC*)&NDIlib_v4_load) = GetProcAddress(hNDILib, "NDIlib_v4_load");

			// Unable to load NDI from the library
			if (!NDIlib_v4_load)
			{
				if (hNDILib)
					FreeLibrary(hNDILib);
				return nullptr;
			}
			ndi_library = NDIlib_v4_load();
			return ndi_library;
		}

}}