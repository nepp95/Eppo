#pragma once

#include "Renderer/Camera/EditorCamera.h"
#include "Scene/Scene.h"

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<Scene>& scene);

		auto BeginScene(const ScopedPtr<EditorCamera>& camera) -> void;
		auto EndScene() -> void;

		auto Resize(uint32_t width, uint32_t height) -> void;

	private:
		std::shared_ptr<Scene> m_Scene = nullptr;
		nvrhi::CommandListHandle m_CommandList = nullptr;

		nvrhi::GraphicsPipelineHandle m_GeometryPipeline = nullptr;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::mat4 ViewProjection;
			glm::vec4 Position;
		} m_CameraData{};
	};
}