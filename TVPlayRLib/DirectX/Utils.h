#pragma once
#include <d3d9.h>
#include <d3d11.h>

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

#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
#define SAFE_CLOSE_HANDLE(p) { if (p) { if ((p) != INVALID_HANDLE_VALUE) ASSERT(CloseHandle(p)); (p) = nullptr; } }
#define SAFE_DELETE(p)       { if (p) { delete (p); (p) = nullptr; } }

namespace TVPlayR
{
	namespace DirectX {

		enum Tex2DType {
			Tex2D_Default,
			Tex2D_DefaultRTarget,
			Tex2D_DefaultShader,
			Tex2D_DefaultShaderRTarget,
			Tex2D_DynamicShaderWrite,
			Tex2D_DynamicShaderWriteNoSRV,
			Tex2D_StagingRead,
		};
		const std::string D3DFormatToString(const D3DFORMAT format);
		const std::string DXGIFormatToString(const DXGI_FORMAT format);
		const std::string D3DFilterToString(const D3D11_VIDEO_PROCESSOR_FILTER filter);
		const AVPixelFormat DXGIFormatToAVPixelFormat(DXGI_FORMAT dx_format);
		const DXGI_FORMAT AVPixelFormatToDXGIFormat(AVPixelFormat format);
		int GetBitDepth(const DXGI_FORMAT format);
		D3D11_TEXTURE2D_DESC CreateTex2DDesc(const DXGI_FORMAT format, const UINT width, const UINT height, const Tex2DType type);
		const D3D11_VIDEO_FRAME_FORMAT AVFrameD3D11_VIDEO_FRAME_FORMAT(const std::shared_ptr<AVFrame>& frame);
	}
}
