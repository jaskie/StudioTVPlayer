#pragma once
#include <d3d11.h>
namespace TVPlayR
{
	namespace DirectX {

		class VideoTextureBuffer
		{
		private:

			std::vector<ID3D11Texture2D*> textures_;
			std::vector<ID3D11VideoProcessorInputView*> input_views_;

			void ReleaseTextures()
			{
				for (auto& inputview : input_views_)
				{
					inputview->Release();
				}
				input_views_.clear();
				for (auto& texture : textures_)
				{
					texture->Release();
				}
				textures_.clear();
			}

		public:
			~VideoTextureBuffer()
			{
				ReleaseTextures();
			}

			const size_t Size() const
			{
				return textures_.size();
			}

			void Clear()
			{
				ReleaseTextures();
				input_views_.clear();
				textures_.clear();
			}

			void Resize(const unsigned len)
			{
				Clear();
				if (len)
				{
					textures_.resize(len);
					input_views_.resize(len);
				}
			}

			void Rotate()
			{
				assert(textures_.size());
				if (textures_.size() > 1)
				{
					ID3D11Texture2D* pSurface = textures_.front();
					ID3D11VideoProcessorInputView* pInputView = input_views_.front();

					for (size_t i = 1; i < textures_.size(); i++)
					{
						auto pre = i - 1;
						textures_[pre] = textures_[i];
						input_views_[pre] = input_views_[i];
					}

					textures_.back() = pSurface;
					input_views_.back() = pInputView;
				}
			}

			ID3D11Texture2D** GetTexture()
			{
				if (textures_.size())
				{
					return &textures_.back();
				}
				else
				{
					return nullptr;
				}
			}

			ID3D11Texture2D** GetTexture(size_t num)
			{
				if (num < textures_.size())
				{
					return &textures_[num];
				}
				else
				{
					return nullptr;
				}
			}

			ID3D11VideoProcessorInputView** GetInputView(size_t num)
			{
				if (num < input_views_.size())
				{
					return &input_views_[num];
				}
				else
				{
					return nullptr;
				}
			}
		};
	}
}
