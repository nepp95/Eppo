#pragma once

#include <nvrhi/nvrhi.h>

#include <variant>

namespace Eppo
{
	struct Buffer;
	using ImageSource = std::variant<std::filesystem::path, Buffer>;

	struct ImageSpecification
	{
		nvrhi::Format ImageFormat = nvrhi::Format::UNKNOWN;
		uint32_t Width = 0;
		uint32_t Height = 0;

		bool IsRenderTarget = false;
		bool AutomaticStateTracking = true;
		nvrhi::ResourceStates InitialState = nvrhi::ResourceStates::ShaderResource;

		std::string DebugName = "Image";
	};

	class Image
	{
	public:
		Image(const ImageSpecification& spec, void* ExistingImage);
		Image(const ImageSpecification& spec, ImageSource source, const nvrhi::CommandListHandle& cmdList = nullptr);
		Image(const ImageSpecification& spec, const nvrhi::CommandListHandle& cmdList = nullptr);
		~Image() = default;

		auto SetData(const void* data, uint64_t size, const nvrhi::CommandListHandle& cmdList = nullptr) -> void;

		[[nodiscard]] auto GetTexture() const -> nvrhi::TextureHandle { return m_Texture; }
		[[nodiscard]] constexpr auto GetWidth() const -> uint32_t { return m_Width; }
		[[nodiscard]] constexpr auto GetHeight() const -> uint32_t { return m_Height; }
		[[nodiscard]] auto GetFormat() const -> nvrhi::Format { return m_Specification.ImageFormat; }
		[[nodiscard]] auto IsDepthImage() const -> bool;

	private:
		[[nodiscard]] auto DecodeImageData(const ImageSource& source) -> void*;
		auto SelectFormat(uint32_t channels, bool isHdr = false) -> nvrhi::Format;
		constexpr auto GetStride(const nvrhi::Format format) const -> uint32_t;

	private:
		ImageSpecification m_Specification;
		nvrhi::TextureHandle m_Texture = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_Stride = 0;
	};
}