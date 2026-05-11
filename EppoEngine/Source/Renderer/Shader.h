#pragma once

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	struct ShaderSpecification
	{
		std::string Name;
		bool IsCompute = false;
	};

	class Shader
	{
	public:
		Shader(ShaderSpecification spec);
		~Shader() = default;

		[[nodiscard]] auto GetVertexShaderHandle() -> nvrhi::ShaderHandle { return m_VertexShader; }
		[[nodiscard]] auto GetPixelShaderHandle() -> nvrhi::ShaderHandle { return m_PixelShader; }

		[[nodiscard]] constexpr auto GetName() const -> const std::string& { return m_Specification.Name; }

	private:
		auto CompileOrGetCache() -> std::unordered_map<nvrhi::ShaderType, std::vector<char>>;
		auto Compile(std::unordered_map<nvrhi::ShaderType, std::vector<char>>& bytes, nvrhi::ShaderType type, const std::string& source) -> void;

	private:
		ShaderSpecification m_Specification;

		nvrhi::ShaderHandle m_VertexShader = nullptr;
		nvrhi::ShaderHandle m_PixelShader = nullptr;
	};
}