#include "../pch.h"
#include "VideoProcessor.h"
#include "../Common/Debug.h"
#include <DirectXMath.h>
#include "Utils.h"
#include <d3d11.h>
#include <d3d11_1.h>
#include <DXGI1_2.h>
#include <dxva2api.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3d11.lib")

namespace TVPlayR
{
	namespace DirectX {
		struct VideoProcessor::implementation : private Common::DebugTarget
		{
			UINT vendor_id = 0;
			CComPtr<ID3D11Device1>			m_pDevice;
			CComPtr<ID3D11DeviceContext1>	m_pDeviceContext;
			CComPtr<ID3D11VideoDevice>		m_pVideoDevice;
			CComPtr<ID3D11VideoProcessor>	m_pVideoProcessor;


			CComPtr<ID3D11VideoContext> m_pVideoContext; 

			implementation()
				: Common::DebugTarget(Common::DebugSeverity::trace, "DirectX Video Processor")
			{
				CreateDevice();
#ifdef DEBUG
				DumpVideoDevice();
#endif
			}

			~implementation()
			{
				ReleaseVideoDevice();
			}

			void CreateDevice()
			{
				ID3D11Device* pDevice = nullptr;
				D3D_FEATURE_LEVEL featurelevel;

				D3D_FEATURE_LEVEL featureLevels[] = {
					D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
				};

				HRESULT hr = D3D11CreateDevice(
					nullptr, // we choose first adapter available
					D3D_DRIVER_TYPE_HARDWARE,
					nullptr,
#ifdef DEBUG
					D3D11_CREATE_DEVICE_DEBUG,
#else
					0,
#endif
					featureLevels,
					sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
					D3D11_SDK_VERSION,
					&pDevice,
					&featurelevel,
					nullptr);
				if (FAILED(hr))
				{
					DebugWindowsError("Unable to create ID3D11Device", hr);
					return;
				}
				else
				{
					DebugPrintLineFmt(Common::DebugSeverity::trace, "Successfully created ID3D11Device with feature level: {}.{}", (featurelevel >> 12), (featurelevel >> 8) & 0xF);
				}
				hr = SetDevice(pDevice);
				if (FAILED(hr))
				{
					DebugWindowsError("SetDevice() failed", hr);
					return;
				}

				pDevice->Release();
			}

			HRESULT SetDevice(ID3D11Device* pDevice)
			{
				if (!pDevice)
					return E_POINTER;
				HRESULT hr = pDevice->QueryInterface(IID_PPV_ARGS(&m_pDevice));
				if (FAILED(hr)) {
					return hr;
				}
				m_pDevice->GetImmediateContext1(&m_pDeviceContext);

				return InitVideoDevice(pDevice);
			}

			HRESULT InitVideoDevice(ID3D11Device* pDevice)
			{
				HRESULT hr = pDevice->QueryInterface(IID_PPV_ARGS(&m_pVideoDevice));
				if (FAILED(hr))
				{
					DebugWindowsError("InitVideoDevice() : QueryInterface(ID3D11VideoDevice) failed", hr);
					ReleaseVideoDevice();
					return hr;
				}

				hr = m_pDeviceContext->QueryInterface(IID_PPV_ARGS(&m_pVideoContext));
				if (FAILED(hr))
				{
					DebugWindowsError("InitVideoDevice() : QueryInterface(ID3D11VideoContext) failed", hr);
					ReleaseVideoDevice();
					return hr;
				}

				return S_OK;
			}

			void ReleaseVideoDevice()
			{
				m_pVideoContext.Release();
				m_pVideoDevice.Release();
				m_pDeviceContext.Release();
				m_pDevice.Release();
			}

			void DumpVideoDevice()
			{
				D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc = { D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE, {}, 1920, 1080, {}, 1920, 1080, D3D11_VIDEO_USAGE_PLAYBACK_NORMAL };
				CComPtr<ID3D11VideoProcessorEnumerator> pVideoProcEnum;
				if (S_OK == m_pVideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &pVideoProcEnum)) {
					std::string input = "Supported input DXGI formats (for 1080p):";
					std::string output = "Supported output DXGI formats (for 1080p):";
					for (int fmt = DXGI_FORMAT_R32G32B32A32_TYPELESS; fmt <= DXGI_FORMAT_B4G4R4A4_UNORM; fmt++) {
						UINT uiFlags;
						if (S_OK == pVideoProcEnum->CheckVideoProcessorFormat((DXGI_FORMAT)fmt, &uiFlags)) {
							if (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT) {
								input.append("\n  ");
								input.append(DXGIFormatToString((DXGI_FORMAT)fmt));
							}
							if (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) {
								output.append("\n  ");
								output.append(DXGIFormatToString((DXGI_FORMAT)fmt));
							}
						}
					}
					DebugPrintLine(Common::DebugSeverity::debug, input);
					DebugPrintLine(Common::DebugSeverity::debug, output);
				}
			}

		};

		VideoProcessor::VideoProcessor()
			: impl_(std::make_unique<implementation>())
		{

		}
		VideoProcessor::~VideoProcessor()
		{
		}
	}
}