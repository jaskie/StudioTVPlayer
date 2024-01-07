#pragma once

#include <d3d11.h>
#include <d3d11_1.h>
#include <DXGI1_2.h>
#include <dxva2api.h>
#include <atltypes.h>
namespace TVPlayR
{
	namespace DirectX {
		class D3D11VP : private Common::DebugTarget
		{
			D3D11VP();
			~D3D11VP();
			void CreateDevice();
			HRESULT SetDevice(ID3D11Device* pDevice);
			HRESULT InitVideoDevice(ID3D11Device* pDevice);
		private:
			UINT vendor_id = 0;
			CComPtr<ID3D11Device1>					m_pDevice;
			CComPtr<ID3D11DeviceContext1>			m_pDeviceContext;
			CComPtr<ID3D11VideoDevice>				m_pVideoDevice;
			CComPtr<ID3D11VideoProcessor>			m_pVideoProcessor;
			CComPtr<ID3D11VideoProcessorEnumerator>	m_pVideoProcessorEnum;

			CComPtr<ID3D11VideoContext>				m_pVideoContext;
			CComPtr<ID3D11VideoProcessorEnumerator1> m_pVideoProcessorEnum1;

			D3D11_VIDEO_PROCESSOR_CAPS				m_VPCaps = {};

			VideoTextureBuffer m_VideoTextures;

			UINT m_nInputFrameOrField = 0;
			bool m_bPresentFrame = false;
			UINT m_nPastFrames = 0;
			UINT m_nFutureFrames = 0;
			UINT m_RateConvIndex = 0;
			D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS m_RateConvCaps = {};

			DXGI_FORMAT m_srcFormat = DXGI_FORMAT_UNKNOWN;
			UINT m_srcWidth = 0;
			UINT m_srcHeight = 0;
			//bool m_bInterlaced = false;
			DXGI_FORMAT m_dstFormat = DXGI_FORMAT_UNKNOWN;

			// Filters
			struct {
				int support;
				int value;
				D3D11_VIDEO_PROCESSOR_FILTER_RANGE range;
			} m_VPFilters[8] = {};
			bool m_bUpdateFilters = false;
			D3D11_VIDEO_PROCESSOR_ROTATION m_Rotation = D3D11_VIDEO_PROCESSOR_ROTATION_IDENTITY;

		public:
			bool IsReady() { return (m_pVideoProcessor != nullptr); }
			HRESULT SetDevice(ID3D11Device* pDevice);
			HRESULT InitVideoDevice(ID3D11Device* pDevice);
#ifdef DEBUG
			void DumpVideoDevice();
#endif
			void ReleaseVideoDevice();
			HRESULT InitVideoProcessor(const DXGI_FORMAT inputFmt, const UINT width, const UINT height, const bool interlaced, DXGI_FORMAT& outputFmt);
			void ReleaseVideoProcessor();
			HRESULT InitInputTextures();
			ID3D11Texture2D* GetNextInputTexture(const D3D11_VIDEO_FRAME_FORMAT vframeFormat);
			HRESULT Process(ID3D11Texture2D* pRenderTarget, const D3D11_VIDEO_FRAME_FORMAT sampleFormat, const bool second);
			HRESULT SetRectangles(const RECT* pSrcRect, const RECT* pDstRect);
		};

	}
}
