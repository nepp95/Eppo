#include "pch.h"
#include "Renderer/Renderer.h"

namespace Eppo
{
	Renderer::Renderer()
	{
		//m_ShaderLibrary.Load("geometry");
		m_ShaderLibrary.Load("imgui");
	}

	auto Renderer::GetShader(const std::string& name) const -> const std::shared_ptr<Shader>&
	{
		return m_ShaderLibrary.Get(name);
	}
}