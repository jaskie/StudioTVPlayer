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
			default:					return "UNKNOWN";
			};
		}

		const std::string DXGIFormatToString(const DXGI_FORMAT format)
		{
			switch (format)
			{
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
			default:										return "UNKNOWN";
			};
		}

		int GetBitDepth(const DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_AYUV:
			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
			case DXGI_FORMAT_YUY2:
			default:
				return 8;
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y410:
				return 10;
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_P016:
			case DXGI_FORMAT_Y216:
			case DXGI_FORMAT_Y416:
				return 16;
			}
		}

		const std::string D3DFilterToString(const D3D11_VIDEO_PROCESSOR_FILTER filter)
		{
			switch (filter)
			{
			case D3D11_VIDEO_PROCESSOR_FILTER_BRIGHTNESS:			return "BRIGHTNESS";
			case D3D11_VIDEO_PROCESSOR_FILTER_CONTRAST:				return "CONTRAST";
			case D3D11_VIDEO_PROCESSOR_FILTER_HUE:					return "HUE";
			case D3D11_VIDEO_PROCESSOR_FILTER_SATURATION:			return "SATURATION";
			case D3D11_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION:		return "NOISE_REDUCTION";
			case D3D11_VIDEO_PROCESSOR_FILTER_EDGE_ENHANCEMENT:		return "EDGE_ENHANCEMENT";
			case D3D11_VIDEO_PROCESSOR_FILTER_ANAMORPHIC_SCALING:	return "ANAMORPHIC_SCALING";
			case D3D11_VIDEO_PROCESSOR_FILTER_STEREO_ADJUSTMENT:	return "STEREO_ADJUSTMENT";
			default:												return "UNKNOWN";
			}
		}

		const AVPixelFormat DXGIFormatToAVPixelFormat(DXGI_FORMAT dx_format)
		{
			switch (dx_format)
			{
			case DXGI_FORMAT_NV12:				return AV_PIX_FMT_NV12;
			case DXGI_FORMAT_P010:				return AV_PIX_FMT_P010;
			case DXGI_FORMAT_B8G8R8A8_UNORM:	return AV_PIX_FMT_BGRA;
			case DXGI_FORMAT_R10G10B10A2_UNORM: return AV_PIX_FMT_X2BGR10;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:return AV_PIX_FMT_RGBAF16;
			case DXGI_FORMAT_AYUV:				return AV_PIX_FMT_VUYX;
			case DXGI_FORMAT_YUY2:				return AV_PIX_FMT_YUYV422;
			case DXGI_FORMAT_Y210:				return AV_PIX_FMT_Y210;
			case DXGI_FORMAT_Y410:				return AV_PIX_FMT_XV30;
			case DXGI_FORMAT_P016:				return AV_PIX_FMT_P012;
			case DXGI_FORMAT_Y216:				return AV_PIX_FMT_Y212;
			case DXGI_FORMAT_Y416:				return AV_PIX_FMT_XV36;
			default:							return AV_PIX_FMT_NONE;
			}
		}

		const DXGI_FORMAT AVPixelFormatToDXGIFormat(AVPixelFormat format)
		{
			switch (format)
			{
			case AV_PIX_FMT_NV12:	return DXGI_FORMAT_NV12;
			case AV_PIX_FMT_P010:	return DXGI_FORMAT_P010;
			case AV_PIX_FMT_BGRA:	return DXGI_FORMAT_B8G8R8A8_UNORM;
			case AV_PIX_FMT_X2BGR10:return DXGI_FORMAT_R10G10B10A2_UNORM;
			case AV_PIX_FMT_RGBAF16:return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case AV_PIX_FMT_VUYX:	return DXGI_FORMAT_AYUV;
			case AV_PIX_FMT_YUYV422:return DXGI_FORMAT_YUY2;
			case AV_PIX_FMT_Y210:	return DXGI_FORMAT_Y210;
			case AV_PIX_FMT_XV30:	return DXGI_FORMAT_Y410;
			case AV_PIX_FMT_P012:	return DXGI_FORMAT_P016;
			case AV_PIX_FMT_Y212:	return DXGI_FORMAT_Y216;
			case AV_PIX_FMT_XV36:	return DXGI_FORMAT_Y416;
			default:				return DXGI_FORMAT_UNKNOWN;
			}
		}

		D3D11_TEXTURE2D_DESC CreateTex2DDesc(const DXGI_FORMAT format, const UINT width, const UINT height, const Tex2DType type)
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = format;
			desc.SampleDesc = { 1, 0 };

			switch (type)
			{
			default:
			case Tex2D_Default:
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = 0;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				break;
			case Tex2D_DefaultRTarget:
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_RENDER_TARGET;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				break;
			case Tex2D_DefaultShader:
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				break;
			case Tex2D_DefaultShaderRTarget:
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				break;
			case Tex2D_DynamicShaderWrite:
			case Tex2D_DynamicShaderWriteNoSRV:
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;
				break;
			case Tex2D_StagingRead:
				desc.Usage = D3D11_USAGE_STAGING;
				desc.BindFlags = 0;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				desc.MiscFlags = 0;
				break;
			}
			return desc;
		}
	}
}