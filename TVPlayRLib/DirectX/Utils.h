#pragma once
#include <d3d9.h>

#define FCC(ch4)	((((DWORD)(ch4) & 0xFF) << 24) | \
					(((DWORD)(ch4) & 0xFF00) << 8) | \
					(((DWORD)(ch4) & 0xFF0000) >> 8) | \
					(((DWORD)(ch4) & 0xFF000000) >> 24))

#define D3DFMT_YV12 (D3DFORMAT)FCC('YV12')
#define D3DFMT_NV12 (D3DFORMAT)FCC('NV12')
#define D3DFMT_P010 (D3DFORMAT)FCC('P010')
#define D3DFMT_P016 (D3DFORMAT)FCC('P016')
#define D3DFMT_P210 (D3DFORMAT)FCC('P210')
#define D3DFMT_P216 (D3DFORMAT)FCC('P216')
#define D3DFMT_AYUV (D3DFORMAT)FCC('AYUV')
#define D3DFMT_Y410 (D3DFORMAT)FCC('Y410')
#define D3DFMT_Y416 (D3DFORMAT)FCC('Y416')

namespace TVPlayR
{
	namespace DirectX {
		const std::string D3DFormatToString(const D3DFORMAT format);
		const std::string DXGIFormatToString(const DXGI_FORMAT format);
	}
}
