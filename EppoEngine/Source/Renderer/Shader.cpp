#include "pch.h"
#include "Renderer/Shader.h"

#include "Renderer/DeviceManager.h"

#if defined(EP_PLATFORM_WINDOWS)
#include <atlbase.h>
#include <dxcapi.h>
#include <d3d12shader.h>
// TODO: Setup transip vps to test if this works on other systems?

#endif

namespace Eppo
{
	Shader::Shader(ShaderSpecification spec)
		: m_Specification(std::move(spec))
	{
		Log::Info("Loading shader '{}'", m_Specification.Name);

		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		const auto bytes = CompileOrGetCache();

		if (m_Specification.IsCompute)
		{
			EP_ASSERT(false);
		}
		else
		{
			nvrhi::ShaderDesc shaderDesc = nvrhi::ShaderDesc()
				.setShaderType(nvrhi::ShaderType::Vertex)
				.setEntryName("VSMain");

			m_VertexShader = device->createShader(shaderDesc, bytes.at(nvrhi::ShaderType::Vertex).data(), bytes.at(nvrhi::ShaderType::Vertex).size());

			shaderDesc.setShaderType(nvrhi::ShaderType::Pixel)
				.setEntryName("PSMain");

			m_PixelShader = device->createShader(shaderDesc, bytes.at(nvrhi::ShaderType::Pixel).data(), bytes.at(nvrhi::ShaderType::Pixel).size());
		}
	}

	auto Shader::CompileOrGetCache() -> std::unordered_map<nvrhi::ShaderType, std::vector<char>>
	{
		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		std::unordered_map<nvrhi::ShaderType, std::vector<char>> bytes;

		if (m_Specification.IsCompute)
		{
			EP_ASSERT(false);
		}
		else
		{
			const std::filesystem::path cacheDir = FS::GetShaderCacheDirectory();
			const std::filesystem::path vertPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.vert", m_Specification.Name);
			const std::filesystem::path pixelPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.frag", m_Specification.Name);
			const std::filesystem::path vertSpvPath = FS::GetShaderCacheDirectory() / std::format("{}.vert.spv", m_Specification.Name);
			const std::filesystem::path pixelSpvPath = FS::GetShaderCacheDirectory() / std::format("{}.frag.spv", m_Specification.Name);
			const std::filesystem::path vertDxilPath = FS::GetShaderCacheDirectory() / std::format("{}.vert.dxil", m_Specification.Name);
			const std::filesystem::path pixelDxilPath = FS::GetShaderCacheDirectory() / std::format("{}.frag.dxil", m_Specification.Name);
			const std::filesystem::path vertCacheHashFile = FS::GetShaderCacheDirectory() / std::format("{}.vert.hash", m_Specification.Name);
			const std::filesystem::path pixelCacheHashFile = FS::GetShaderCacheDirectory() / std::format("{}.frag.hash", m_Specification.Name);
			const auto vertSource = FS::ReadText(vertPath);
			const auto pixelSource = FS::ReadText(pixelPath);

			bool invalidate = true;
			if (FS::Exists(vertSpvPath) && FS::Exists(vertDxilPath) && FS::Exists(vertCacheHashFile))
			{
				std::string vertHash = std::to_string(Hash::GenerateFnv(vertSource));
				std::string pixelHash = std::to_string(Hash::GenerateFnv(pixelSource));
				std::string vertCacheHash = FS::ReadText(vertCacheHashFile);
				std::string pixelCacheHash = FS::ReadText(pixelCacheHashFile);

				if (vertHash == vertCacheHash && pixelHash == pixelCacheHash)
					invalidate = false;
			}
			
			if (!invalidate)
			{
				Log::Info("Loading shader cache for '{}'", m_Specification.Name);

				std::vector<char> vertBytes;
				std::vector<char> pixelBytes;

				switch (dm->GetParams().API)
				{
					case RendererAPI::DX11:
					case RendererAPI::DX12:
					{
						bytes[nvrhi::ShaderType::Vertex] = FS::ReadBytes(vertDxilPath);
						bytes[nvrhi::ShaderType::Pixel] = FS::ReadBytes(pixelDxilPath);
						break;
					}

					case RendererAPI::Vulkan:
					{
						bytes[nvrhi::ShaderType::Vertex] = FS::ReadBytes(vertSpvPath);
						bytes[nvrhi::ShaderType::Pixel] = FS::ReadBytes(pixelSpvPath);
						break;
					}
				}
			}
			else
			{
				Log::Info("Compiling shader '{}'", m_Specification.Name);
				Compile(bytes, nvrhi::ShaderType::Vertex, vertSource);
				Compile(bytes, nvrhi::ShaderType::Pixel, pixelSource);
			}
		}

		return bytes;
	}

	auto Shader::Compile(std::unordered_map<nvrhi::ShaderType, std::vector<char>>& bytes, const nvrhi::ShaderType type, std::string_view source) -> void
	{

	}
}