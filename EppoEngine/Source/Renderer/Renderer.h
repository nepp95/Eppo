#pragma once

#include "Renderer/ShaderLibrary.h"

namespace Eppo
{
	class Renderer
	{
	public:
		Renderer();

		[[nodiscard]] auto GetShader(const std::string& name) const -> Ref<Shader>;

	private:
		ShaderLibrary m_ShaderLibrary;
	};
}