#pragma once

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	struct ImageSpecification
	{
		nvrhi::Format ImageFormat = nvrhi::Format::UNKNOWN;
		uint32_t Width = 0;
		uint32_t Height = 0;

		bool IsRenderTarget = true;
		bool AutomaticStateTracking = true;
		nvrhi::ResourceStates InitialState = nvrhi::ResourceStates::ShaderResource;

		void* ExistingImage = nullptr;
		std::string DebugName = "Image";
	};

	class Image
	{
	public:
		Image(ImageSpecification spec);
		~Image() = default;

		[[nodiscard]] auto GetTexture() const -> nvrhi::TextureHandle { return m_Texture; }
		[[nodiscard]] constexpr auto GetWidth() const -> uint32_t { return m_Specification.Width; }
		[[nodiscard]] constexpr auto GetHeight() const -> uint32_t { return m_Specification.Height; }
		[[nodiscard]] constexpr auto GetParams() const -> const ImageSpecification& { return m_Specification; }

	private:
		ImageSpecification m_Specification;

		nvrhi::TextureHandle m_Texture = nullptr;
	};
}