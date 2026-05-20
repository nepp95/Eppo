#include "pch.h"
#include "Renderer/Shader.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Renderer/DeviceManager.h"

namespace Eppo
{
	namespace Utils
	{
		auto NvrhiFormatSize(nvrhi::Format format) -> uint32_t
		{
			switch (format)
			{
				case nvrhi::Format::R32_FLOAT:
					return 4;
				case nvrhi::Format::RG32_FLOAT:
					return 4 * 2;
				case nvrhi::Format::RGB32_FLOAT:
					return 4 * 3;
				case nvrhi::Format::RGBA32_FLOAT:
					return 4 * 4;
				case nvrhi::Format::R8_UNORM:
				case nvrhi::Format::R8_UINT:
					return 1;
				case nvrhi::Format::RG8_UNORM:
					return 1 * 2;
				case nvrhi::Format::RGBA8_UNORM:
					return 1 * 4;
				case nvrhi::Format::R32_UINT:
					return 4;

				default:
				{
					EP_ASSERT(false);
					return 0;
				}
			}
		}
	}

	Shader::Shader(ShaderSpecification spec)
		: m_Specification(std::move(spec))
	{
		Log::Info("Loading shader '{}'", m_Specification.Name);

		EP_ASSERT(DeviceManager::Get()->GetParams().API == RendererAPI::Vulkan);
		EP_ASSERT(!m_Specification.IsCompute);
	}

	auto Shader::GetShaderHandle(const nvrhi::ShaderType type) -> nvrhi::ShaderHandle
	{
		auto it = m_ShaderHandles.find(type);
		if (it != m_ShaderHandles.end())
			return it->second;
		return nullptr;
	}

	auto Shader::Create(ShaderSpecification spec) -> std::shared_ptr<Shader>
	{
		const auto& dm = DeviceManager::Get();

		switch (dm->GetParams().API)
		{
			case RendererAPI::DX11:
			case RendererAPI::DX12:
			{
				EP_ASSERT(false);
				break;
			}

			case RendererAPI::Vulkan:
				return std::make_shared<VulkanShader>(std::move(spec));
		}
	}

	auto Shader::CreateShaderHandles() -> void
	{
		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		nvrhi::ShaderDesc shaderDesc{
			.entryName = "Main",
		};

		for (const auto& [type, bytes] : m_ShaderBytes)
		{
			shaderDesc.shaderType = type;
			m_ShaderHandles[type] = device->createShader(shaderDesc, m_ShaderBytes.at(type).data(), m_ShaderBytes.at(type).size());
		}
	}

	auto Shader::CreateInputLayout() -> void
	{
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();

		std::vector<nvrhi::VertexAttributeDesc> attributeDescs(m_ShaderInputs.size());
		for (uint32_t i = 0; i < m_ShaderInputs.size(); i++)
		{
			auto& input = m_ShaderInputs.at(i);
			auto& attributeDesc = attributeDescs.at(i);

			attributeDesc.name = input.Name;
			attributeDesc.format = input.Type;
			attributeDesc.arraySize = 1;
			attributeDesc.bufferIndex = 0;
			attributeDesc.offset = input.Offset;
			attributeDesc.elementStride = m_InputAttributeStride;
			attributeDesc.isInstanced = false;
		}

		m_InputLayout = device->createInputLayout(attributeDescs.data(), attributeDescs.size(), m_ShaderHandles.at(nvrhi::ShaderType::Vertex));
	}

	auto Shader::CreateBindingLayout() -> void
	{
		const auto& dm = DeviceManager::Get();
		auto device = dm->GetDevice();

		for (const auto& [set, setResources] : m_ShaderResources)
		{
			nvrhi::BindingLayoutDesc bindingLayoutDesc{
				.visibility = nvrhi::ShaderType::All,
			};

			for (const auto& resource : setResources)
			{
				nvrhi::BindingLayoutItem item{
					.slot = resource.Binding,
					.type = resource.Type,
					.size = static_cast<uint16_t>(resource.ArraySize),
				};

				bindingLayoutDesc.addItem(item);
			}

			if (!bindingLayoutDesc.bindings.empty())
				m_BindingLayouts[set] = device->createBindingLayout(bindingLayoutDesc);
		}
	}
}