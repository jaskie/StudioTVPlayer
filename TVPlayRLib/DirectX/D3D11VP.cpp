#include "../pch.h"
#include "D3D11VP.h"
#include "../Common/Debug.h"
#include <DirectXMath.h>
#include "Utils.h"
#include "VideoTextureBuffer.h"
#include "DX11Helper.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3d11.lib")

namespace TVPlayR
{
	namespace DirectX {

		D3D11VP::D3D11VP()
				: Common::DebugTarget(Common::DebugSeverity::trace, "DirectX Video Processor")
			{
				CreateDevice();
#ifdef DEBUG
				DumpVideoDevice();
#endif
			}

		D3D11VP::~D3D11VP()
			{
				ReleaseVideoProcessor();
				ReleaseVideoDevice();
			}

			void D3D11VP::CreateDevice()
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

			HRESULT D3D11VP::SetDevice(ID3D11Device* pDevice)
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

			HRESULT D3D11VP::InitVideoDevice(ID3D11Device* pDevice)
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

			void D3D11VP::ReleaseVideoDevice()
			{
				m_pVideoContext.Release();
				m_pVideoDevice.Release();
				m_pDeviceContext.Release();
				m_pDevice.Release();
			}

#ifdef DEBUG
			void D3D11VP::DumpVideoDevice()
			{
				D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc = { D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE, {}, 1920, 1080, {}, 1920, 1080, D3D11_VIDEO_USAGE_OPTIMAL_QUALITY };
				CComPtr<ID3D11VideoProcessorEnumerator> pVideoProcEnum;
				if (S_OK == m_pVideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &pVideoProcEnum))
				{
					std::string input = "Supported input DXGI formats (for 1080p):";
					std::string output = "Supported output DXGI formats (for 1080p):";
					for (int fmt = DXGI_FORMAT_R32G32B32A32_TYPELESS; fmt <= DXGI_FORMAT_B4G4R4A4_UNORM; fmt++)
					{
						UINT uiFlags;
						if (S_OK == pVideoProcEnum->CheckVideoProcessorFormat((DXGI_FORMAT)fmt, &uiFlags))
						{
							if (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT)
							{
								input.append("\n  ");
								input.append(DXGIFormatToString((DXGI_FORMAT)fmt));
							}
							if (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT)
							{
								output.append("\n  ");
								output.append(DXGIFormatToString((DXGI_FORMAT)fmt));
							}
						}
					}
					DebugPrintLine(Common::DebugSeverity::debug, input);
					DebugPrintLine(Common::DebugSeverity::debug, output);
				}
			}
#endif

			HRESULT D3D11VP::InitVideoProcessor(const DXGI_FORMAT inputFmt, const UINT width, const UINT height, const bool interlaced, DXGI_FORMAT& outputFmt)
			{
				ReleaseVideoProcessor();

				// create VideoProcessorEnumerator
				D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
				ZeroMemory(&ContentDesc, sizeof(ContentDesc));
				ContentDesc.InputFrameFormat = interlaced ? D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST : D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
				ContentDesc.InputWidth = width;
				ContentDesc.InputHeight = height;
				ContentDesc.OutputWidth = ContentDesc.InputWidth;
				ContentDesc.OutputHeight = ContentDesc.InputHeight;
				ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

				HRESULT hr = m_pVideoDevice->CreateVideoProcessorEnumerator(&ContentDesc, &m_pVideoProcessorEnum);
				if (FAILED(hr))
				{
					DebugWindowsError("InitVideoProcessor(): CreateVideoProcessorEnumerator() failed", hr);
					return hr;
				}
				// check input format
				UINT uiFlags;
				hr = m_pVideoProcessorEnum->CheckVideoProcessorFormat(inputFmt, &uiFlags);
				if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT))
				{
					DebugPrintLineFmt(Common::DebugSeverity::error, "InitVideoProcessor(): {} is not supported for D3D11 VP input.", DXGIFormatToString(inputFmt));
					return E_INVALIDARG;
				}

				// get VideoProcessorCaps
				hr = m_pVideoProcessorEnum->GetVideoProcessorCaps(&m_VPCaps);
				if (FAILED(hr))
				{
					DebugWindowsError("InitVideoProcessor(): GetVideoProcessorCaps() failed with error {}", hr);
					return hr;
				}

#ifdef DEBUG
				std::string dbgstr = "VideoProcessorCaps:";
				dbgstr += std::format("\n  Device YCbCr matrix conversion: {}", (m_VPCaps.DeviceCaps & D3D11_VIDEO_PROCESSOR_DEVICE_CAPS_YCbCr_MATRIX_CONVERSION) ? "supported" : "NOT supported");
				dbgstr += std::format("\n  Device YUV nominal range      : {}", (m_VPCaps.DeviceCaps & D3D11_VIDEO_PROCESSOR_DEVICE_CAPS_NOMINAL_RANGE) ? "supported" : "NOT supported");
				dbgstr += std::format("\n  Feature LEGACY                : {}", (m_VPCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_LEGACY) ? "Yes" : "No");
				dbgstr += std::format("\n  Feature Shader usage          : {}", (m_VPCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_SHADER_USAGE) ? "supported" : "NOT supported");
				dbgstr += std::format("\n  Feature Metadata HDR10        : {}", (m_VPCaps.FeatureCaps & D3D11_VIDEO_PROCESSOR_FEATURE_CAPS_METADATA_HDR10) ? "supported" : "NOT supported");
				dbgstr.append("\n  Filter capabilities           :");
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_BRIGHTNESS) { dbgstr.append(" Brightness,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_CONTRAST) { dbgstr.append(" Contrast,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_HUE) { dbgstr.append(" Hue,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_SATURATION) { dbgstr.append(" Saturation,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_NOISE_REDUCTION) { dbgstr.append(" Noise reduction,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_EDGE_ENHANCEMENT) { dbgstr.append(" Edge enhancement,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_ANAMORPHIC_SCALING) { dbgstr.append(" Anamorphic scaling,"); }
				if (m_VPCaps.FilterCaps & D3D11_VIDEO_PROCESSOR_FILTER_CAPS_STEREO_ADJUSTMENT) { dbgstr.append(" Stereo adjustment"); }
				dbgstr += std::format("\n  InputFormat interlaced RGB    : {}", (m_VPCaps.InputFormatCaps & D3D11_VIDEO_PROCESSOR_FORMAT_CAPS_RGB_INTERLACED) ? "supported" : "NOT supported");
				dbgstr += std::format("\n  InputFormat RGB ProcAmp       : {}", (m_VPCaps.InputFormatCaps & D3D11_VIDEO_PROCESSOR_FORMAT_CAPS_RGB_PROCAMP) ? "supported" : "NOT supported");
				dbgstr.append("\n  AutoStream image processing   :");
				if (!m_VPCaps.AutoStreamCaps)
				{
					dbgstr.append(" None");
				}
				else 
				{
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_DENOISE) { dbgstr.append(" Denoise,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_DERINGING) { dbgstr.append(" Deringing,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_EDGE_ENHANCEMENT) { dbgstr.append(" Edge enhancement,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_COLOR_CORRECTION) { dbgstr.append(" Color correction,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_FLESH_TONE_MAPPING) { dbgstr.append(" Flesh tone mapping,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_IMAGE_STABILIZATION) { dbgstr.append(" Image stabilization,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_SUPER_RESOLUTION) { dbgstr.append(" Super resolution,"); }
					if (m_VPCaps.AutoStreamCaps & D3D11_VIDEO_PROCESSOR_AUTO_STREAM_CAPS_ANAMORPHIC_SCALING) { dbgstr.append(" Anamorphic scaling"); }
				}
				DebugPrintLine(Common::DebugSeverity::debug, dbgstr);
#endif
				// select output format
				// always overriding the output format because there are problems with FLOAT
				outputFmt = GetBitDepth(inputFmt) <= 8 && (outputFmt == DXGI_FORMAT_B8G8R8A8_UNORM || outputFmt == DXGI_FORMAT_UNKNOWN)
					? DXGI_FORMAT_B8G8R8A8_UNORM
					: DXGI_FORMAT_R10G10B10A2_UNORM;
			
				hr = m_pVideoProcessorEnum->CheckVideoProcessorFormat(outputFmt, &uiFlags);
				if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT))
				{
					if (outputFmt != DXGI_FORMAT_B8G8R8A8_UNORM)
					{
						DebugPrintLineFmt(Common::DebugSeverity::warning, "InitVideoProcessor(): {} is not supported for D3D11 VP output. DXGI_FORMAT_B8G8R8A8_UNORM will be used.", DXGIFormatToString(outputFmt));

						outputFmt = DXGI_FORMAT_B8G8R8A8_UNORM;
						hr = m_pVideoProcessorEnum->CheckVideoProcessorFormat(outputFmt, &uiFlags);
						if (FAILED(hr) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT))
						{
							hr = E_INVALIDARG;
						}
					}

					if (FAILED(hr))
					{
						DebugPrintLineFmt(Common::DebugSeverity::error, "InitVideoProcessor(): {} is not supported for D3D11 VP output.", DXGIFormatToString(outputFmt));
						return E_INVALIDARG;
					}
				}

				m_RateConvIndex = 0;
				if (interlaced)
				{
					// try to find best processor
					const UINT preferredDeintCaps = D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BLEND
						| D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB
						| D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE
						| D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION;
					UINT maxProcCaps = 0;

					D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS convCaps = {};
					for (UINT i = 0; i < m_VPCaps.RateConversionCapsCount; i++)
					{
						if (S_OK == m_pVideoProcessorEnum->GetVideoProcessorRateConversionCaps(i, &convCaps))
						{
							// check only deinterlace caps
							if ((convCaps.ProcessorCaps & preferredDeintCaps) > maxProcCaps)
							{
								m_RateConvIndex = i;
								maxProcCaps = convCaps.ProcessorCaps & preferredDeintCaps;
							}
						}
					}

					DebugPrintLineIf(!maxProcCaps, Common::DebugSeverity::warning, "InitVideoProcessor(): deinterlace caps not supported");
					if (maxProcCaps) {
						if (S_OK == m_pVideoProcessorEnum->GetVideoProcessorRateConversionCaps(m_RateConvIndex, &m_RateConvCaps))
						{
#ifdef DEBUG
							dbgstr = std::format("RateConversionCaps[{}]:", m_RateConvIndex);
							dbgstr.append("\n  ProcessorCaps:");
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BLEND) { dbgstr.append(" Blend,"); }
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_BOB) { dbgstr.append(" Bob,"); }
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_ADAPTIVE) { dbgstr.append(" Adaptive,"); }
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_DEINTERLACE_MOTION_COMPENSATION) { dbgstr.append(" Motion Compensation,"); }
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_INVERSE_TELECINE) { dbgstr.append(" Inverse Telecine,"); }
							if (m_RateConvCaps.ProcessorCaps & D3D11_VIDEO_PROCESSOR_PROCESSOR_CAPS_FRAME_RATE_CONVERSION) { dbgstr.append(" Frame Rate Conversion"); }
							dbgstr += std::format("\n  PastFrames   : {}", m_RateConvCaps.PastFrames);
							dbgstr += std::format("\n  FutureFrames : {}", m_RateConvCaps.FutureFrames);
							dbgstr += std::format("\n  CustomRateCount: {}", m_RateConvCaps.CustomRateCount);
							if (m_RateConvCaps.CustomRateCount)
							{
								for (UINT i = 0; i < m_RateConvCaps.CustomRateCount; i++)
								{
									D3D11_VIDEO_PROCESSOR_CUSTOM_RATE rate;
									if (S_OK == m_pVideoProcessorEnum->GetVideoProcessorCustomRate(m_RateConvIndex, i, &rate))
									{
										dbgstr += std::format("\n    {:2d}: {}/{}, input {} OutputFrames {} InputFramesOrFields {}", 
											i, rate.CustomRate.Numerator, rate.CustomRate.Denominator, rate.InputInterlaced ? "interlaced " : "progressive", rate.OutputFrames, rate.InputFramesOrFields);
									}
								}
							}
							DebugPrintLine(Common::DebugSeverity::debug, dbgstr);
#endif
						}
					}
				}

				hr = m_pVideoDevice->CreateVideoProcessor(m_pVideoProcessorEnum, m_RateConvIndex, &m_pVideoProcessor);
				if (FAILED(hr))
				{
					DebugWindowsError("InitVideoProcessor(): CreateVideoProcessor() failed", hr);
					return hr;
				}
				m_pVideoProcessorEnum->QueryInterface(IID_PPV_ARGS(&m_pVideoProcessorEnum1));
				DebugPrintLineIf(!m_pVideoProcessorEnum1, Common::DebugSeverity::warning, "InitVideoProcessor(): ID3D11VideoProcessorEnumerator1 unavailable");

				for (UINT i = 0; i < std::size(m_VPFilters); i++)
				{
					auto& filter = m_VPFilters[i];
					filter.support = m_VPCaps.FilterCaps & (1u << i);
					m_VPFilters[i].range = {};

					HRESULT hr2 = E_FAIL;
					if (filter.support)
					{
						hr2 = m_pVideoProcessorEnum->GetVideoProcessorFilterRange((D3D11_VIDEO_PROCESSOR_FILTER)i, &filter.range);

						if (FAILED(hr2))
						{
							DebugPrintLineFmt(Common::DebugSeverity::warning, "InitVideoProcessor(): GetVideoProcessorFilterRange({}) failed with error {}", i, std::system_category().message(hr2));
							filter.support = 0;
						}
						else
						{
							DebugPrintLineFmt(Common::DebugSeverity::debug, "InitVideoProcessor(): FilterRange {:18s}: min={:5d}, def={:3d}, max={:4d}, mux={:.3f}",
								D3DFilterToString((D3D11_VIDEO_PROCESSOR_FILTER)i), filter.range.Minimum, filter.range.Default, filter.range.Maximum, filter.range.Multiplier);
						}

						if (i >= D3D11_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION)
						{
							filter.value = filter.range.Default; // disable it
						}
					}
				}

				// Output rate (repeat frames)
				m_pVideoContext->VideoProcessorSetStreamOutputRate(m_pVideoProcessor, 0, D3D11_VIDEO_PROCESSOR_OUTPUT_RATE_NORMAL, TRUE, nullptr);
				// disable automatic video quality by driver
				m_pVideoContext->VideoProcessorSetStreamAutoProcessingMode(m_pVideoProcessor, 0, FALSE);
				// Output background color (black)
				static const D3D11_VIDEO_COLOR backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };
				m_pVideoContext->VideoProcessorSetOutputBackgroundColor(m_pVideoProcessor, FALSE, &backgroundColor);
				// other
				m_pVideoContext->VideoProcessorSetOutputTargetRect(m_pVideoProcessor, FALSE, nullptr);
				m_pVideoContext->VideoProcessorSetStreamRotation(m_pVideoProcessor, 0, m_Rotation ? TRUE : FALSE, m_Rotation);

				m_srcFormat = inputFmt;
				m_srcWidth = width;
				m_srcHeight = height;

				m_dstFormat = outputFmt;

				return hr;

			}

			void D3D11VP::ReleaseVideoProcessor()
			{
				m_VideoTextures.Clear();
				m_pVideoProcessor.Release();
				m_pVideoProcessorEnum1.Release();
				m_pVideoProcessorEnum.Release();
				m_VPCaps = {};
				m_RateConvIndex = 0;
				m_RateConvCaps = {};
				m_srcFormat = DXGI_FORMAT_UNKNOWN;
				m_srcWidth = 0;
				m_srcHeight = 0;
			}

			HRESULT D3D11VP::InitInputTextures()
			{
#if ENABLE_FUTUREFRAMES
				m_VideoTextures.Resize(1 + m_RateConvCaps.PastFrames + m_RateConvCaps.FutureFrames);
#else
				m_VideoTextures.Resize(1 + m_RateConvCaps.PastFrames);
#endif

				HRESULT hr = E_NOT_VALID_STATE;

				for (size_t i = 0; i < m_VideoTextures.Size(); i++)
				{
					ID3D11Texture2D** ppTexture = m_VideoTextures.GetTexture(i);
					D3D11_TEXTURE2D_DESC texdesc = CreateTex2DDesc(m_srcFormat, m_srcWidth, m_srcHeight, Tex2D_Default);

					hr = m_pDevice->CreateTexture2D(&texdesc, nullptr, ppTexture);
					if (S_OK == hr)
					{
						D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = {};
						inputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
						hr = m_pVideoDevice->CreateVideoProcessorInputView(*ppTexture, m_pVideoProcessorEnum, &inputViewDesc, m_VideoTextures.GetInputView(i));
					}
				}
				return hr;
			}

			ID3D11Texture2D* D3D11VP::GetNextInputTexture(const D3D11_VIDEO_FRAME_FORMAT vframeFormat)
			{
				if (m_VideoTextures.Size())
				{
					if (vframeFormat == D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE)
					{
						m_nInputFrameOrField++;
					}
					else
					{
						m_nInputFrameOrField += 2;
					}
					if (!m_bPresentFrame)
					{
						m_bPresentFrame = true;
					}
#if ENABLE_FUTUREFRAMES
					else if (m_nFutureFrames < m_RateConvCaps.FutureFrames)
					{
						m_nFutureFrames++;
					}
#endif
					else if (m_nPastFrames < m_RateConvCaps.PastFrames)
					{
						m_nPastFrames++;
					}

					m_VideoTextures.Rotate();
				}

				return *m_VideoTextures.GetTexture();
			}

			HRESULT D3D11VP::Process(ID3D11Texture2D* pRenderTarget, const D3D11_VIDEO_FRAME_FORMAT sampleFormat, const bool second)
			{
				assert(m_pVideoDevice);
				assert(m_pVideoContext);

				if (!second)
				{
					m_pVideoContext->VideoProcessorSetStreamFrameFormat(m_pVideoProcessor, 0, sampleFormat);

					if (m_bUpdateFilters)
					{
						for (UINT i = 0; i < std::size(m_VPFilters); i++) {
							auto& filter = m_VPFilters[i];
							if (filter.support)
							{
								BOOL bEnable = (filter.value != filter.range.Default);
								m_pVideoContext->VideoProcessorSetStreamFilter(m_pVideoProcessor, 0, (D3D11_VIDEO_PROCESSOR_FILTER)i, bEnable, filter.value);
							}
						}
						m_bUpdateFilters = false;
					}
				}

				D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc = {};
				OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
				CComPtr<ID3D11VideoProcessorOutputView> pOutputView;
				HRESULT hr = m_pVideoDevice->CreateVideoProcessorOutputView(pRenderTarget, m_pVideoProcessorEnum, &OutputViewDesc, &pOutputView);
				if (FAILED(hr))
				{
					DebugWindowsError("Process(): CreateVideoProcessorOutputView() failed", hr);
					return hr;
				}

				D3D11_VIDEO_PROCESSOR_STREAM streamData = {};
				streamData.Enable = TRUE;
				streamData.InputFrameOrField = m_nInputFrameOrField;
				if (second)
				{
					streamData.OutputIndex = 1;
				}
				else
				{
					streamData.InputFrameOrField--;
				}
				if (m_VideoTextures.Size())
				{
					UINT idx = static_cast<UINT>(m_VideoTextures.Size());
					if (m_nFutureFrames)
					{
						idx -= m_nFutureFrames;
						streamData.FutureFrames = m_nFutureFrames;
						streamData.ppFutureSurfaces = m_VideoTextures.GetInputView(idx);
					}
					idx--;
					streamData.pInputSurface = *m_VideoTextures.GetInputView(idx);
					if (m_nPastFrames)
					{
						idx -= m_nPastFrames;
						streamData.PastFrames = m_nPastFrames;
						streamData.ppPastSurfaces = m_VideoTextures.GetInputView(idx);
					}
				}
				hr = m_pVideoContext->VideoProcessorBlt(m_pVideoProcessor, pOutputView, streamData.InputFrameOrField, 1, &streamData);
				if (FAILED(hr))
				{
					DebugWindowsError("Process() : VideoProcessorBlt() failed", hr);
				}
				return hr;
			}

			HRESULT D3D11VP::SetRectangles(const RECT* pSrcRect, const RECT* pDstRect)
			{
				if (!m_pVideoContext)
					return E_ABORT;

				m_pVideoContext->VideoProcessorSetStreamSourceRect(m_pVideoProcessor, 0, pSrcRect ? TRUE : FALSE, pSrcRect);
				m_pVideoContext->VideoProcessorSetStreamDestRect(m_pVideoProcessor, 0, pDstRect ? TRUE : FALSE, pDstRect);

				return S_OK;
			}
	}
}