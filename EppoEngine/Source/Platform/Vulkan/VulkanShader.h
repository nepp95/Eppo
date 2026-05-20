#pragma once

#include "Renderer/Shader.h"

namespace Eppo
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(ShaderSpecification spec);

	private:
		auto CompileOrGetCache() -> void;
		auto Compile(nvrhi::ShaderType type) -> void;
		auto Reflect(nvrhi::ShaderType type) -> void;
	};
}