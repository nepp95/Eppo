#pragma once

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	class Shader
	{
	public:
		Shader(const std::string& name);
		~Shader() = default;

		[[nodiscard]] auto GetVertexShaderHandle() -> nvrhi::ShaderHandle { return m_VertexShader; }
		[[nodiscard]] auto GetPixelShaderHandle() -> nvrhi::ShaderHandle { return m_PixelShader; }

	private:
		std::string m_Name;

		nvrhi::ShaderHandle m_VertexShader = nullptr;
		nvrhi::ShaderHandle m_PixelShader = nullptr;
	};
}