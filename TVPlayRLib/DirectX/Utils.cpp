#include "../pch.h"
#include "Utils.h"
#include <d3d11.h>

namespace TVPlayR
{
	namespace DirectX {
		const std::string D3DFormatToString(const D3DFORMAT format)
		{
			switch (format)
			{
			case D3DFMT_A8R8G8B8:		return "A8R8G8B8";
			case D3DFMT_X8R8G8B8:		return "X8R8G8B8";
			case D3DFMT_A2B10G10R10:	return "A2B10G10R10";
			case D3DFMT_A8B8G8R8:		return "A8B8G8R8";		// often not supported
			case D3DFMT_G16R16:			return "G16R16";
			case D3DFMT_A2R10G10B10:	return "A2R10G10B10";
			case D3DFMT_A8P8:			return "A8P8";			// DXVA-HD
			case D3DFMT_P8:				return "P8";				// DXVA-HD
			case D3DFMT_L8:				return "L8";
			case D3DFMT_A8L8:			return "A8L8";
			case D3DFMT_L16:			return "L16";
			case D3DFMT_A16B16G16R16F:	return "A16B16G16R16F";
			case D3DFMT_A32B32G32R32F:	return "A32B32G32R32F";
			case D3DFMT_YUY2:			return "YUY2";
			case D3DFMT_UYVY:			return "UYVY";
			case D3DFMT_NV12:			return "NV12";
			case D3DFMT_YV12:			return "YV12";
			case D3DFMT_P010:			return "P010";
			case D3DFMT_P016:			return "P016";
			case D3DFMT_P210:			return "P210";
			case D3DFMT_P216:			return "P216";
			case D3DFMT_AYUV:			return "AYUV";
			case D3DFMT_Y410:			return "Y410";
			case D3DFMT_Y416:			return "Y416";
			case FCC('Y210'):			return "Y210";			// Intel
			case FCC('Y216'):			return "Y216";			// Intel
			case FCC('AIP8'):			return "AIP8";			// DXVA-HD
			case FCC('AI44'):			return "AI44";			// DXVA-HD
			default: return "UNKNOWN";
			};
		}

		const std::string DXGIFormatToString(const DXGI_FORMAT format)
		{
			switch (format) {
			case DXGI_FORMAT_R16G16B16A16_FLOAT:			return "R16G16B16A16_FLOAT";
			case DXGI_FORMAT_R16G16B16A16_UNORM:			return "R16G16B16A16_UNORM";
			case DXGI_FORMAT_R10G10B10A2_UNORM:				return "R10G10B10A2_UNORM";
			case DXGI_FORMAT_R8G8B8A8_UNORM:				return "R8G8B8A8_UNORM";
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:			return "R8G8B8A8_UNORM_SRGB";
			case DXGI_FORMAT_R16G16_UNORM:					return "R16G16_UNORM";
			case DXGI_FORMAT_R8G8_UNORM:					return "R8G8_UNORM";
			case DXGI_FORMAT_R16_TYPELESS:					return "R16_TYPELESS";
			case DXGI_FORMAT_R16_UNORM:						return "R16_UNORM";
			case DXGI_FORMAT_R8_TYPELESS:					return "R8_TYPELES";
			case DXGI_FORMAT_R8_UNORM:						return "R8_UNORM";
			case DXGI_FORMAT_B8G8R8A8_UNORM:				return "B8G8R8A8_UNORM";
			case DXGI_FORMAT_B8G8R8X8_UNORM:				return "B8G8R8X8_UNORM";
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:	return "R10G10B10_XR_BIAS_A2_UNORM";
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:			return "B8G8R8A8_UNORM_SRGB";
			case DXGI_FORMAT_AYUV:							return "AYUV";
			case DXGI_FORMAT_Y410:							return "Y410";
			case DXGI_FORMAT_Y416:							return "Y416";
			case DXGI_FORMAT_NV12:							return "NV12";
			case DXGI_FORMAT_P010:							return "P010";
			case DXGI_FORMAT_P016:							return "P016";
			case DXGI_FORMAT_420_OPAQUE:					return "420_OPAQUE";
			case DXGI_FORMAT_YUY2:							return "YUY2";
			case DXGI_FORMAT_Y210:							return "Y210";
			case DXGI_FORMAT_Y216:							return "Y216";
			case DXGI_FORMAT_AI44:							return "AI44";
			case DXGI_FORMAT_IA44:							return "IA44";
			case DXGI_FORMAT_P8:							return "P8";
			case DXGI_FORMAT_A8P8:							return "A8P8";
			default: return "UNKNOWN";
			};
		}
	}
}