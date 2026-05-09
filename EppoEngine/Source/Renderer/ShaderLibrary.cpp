#include "pch.h"
#include "Renderer/ShaderLibrary.h"

namespace Eppo
{
	auto ShaderLibrary::Load(const std::string& name) -> void
	{
		if (m_Shaders.contains(name))
			Log::Warn("Shader with name '{}' already exists, reloading shader!", name);
	
		m_Shaders[name] = std::make_shared<Shader>(name);
	}

	auto ShaderLibrary::Get(const std::string& name) const -> const std::shared_ptr<Shader>&
	{
		if (m_Shaders.contains(name))
			return m_Shaders.at(name);

		Log::Error("Shader with name '{}' not found!", name);
		return nullptr;
	}
}