#include "pch.h"
#include "Renderer/Shader.h"

#include "Renderer/DeviceManager.h"

namespace Eppo
{
	Shader::Shader(const std::string& name)
		: m_Name(name)
	{
		Log::Info("Compiling shader '{}'", m_Name);

		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		std::filesystem::path vertPath;
		std::filesystem::path pixelPath;
		
		switch (dm->GetParams().API)
		{
			case RendererAPI::DX11:
			case RendererAPI::DX12:
			{
				EP_ASSERT(false);
				break;
			}

			case RendererAPI::Vulkan:
			{
				vertPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.vert.spv", m_Name);
				pixelPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.pixel.spv", m_Name);

				break;
			}
		}

		const auto vertBytes = FS::ReadBytes(vertPath);
		const auto pixelBytes = FS::ReadBytes(pixelPath);

		nvrhi::ShaderDesc shaderDesc = nvrhi::ShaderDesc()
			.setShaderType(nvrhi::ShaderType::Vertex)
			.setEntryName("VSMain");

		m_VertexShader = device->createShader(shaderDesc, vertBytes.data(), vertBytes.size());

		shaderDesc.setShaderType(nvrhi::ShaderType::Pixel)
			.setEntryName("PSMain");

		m_PixelShader = device->createShader(shaderDesc, pixelBytes.data(), pixelBytes.size());
	}
}