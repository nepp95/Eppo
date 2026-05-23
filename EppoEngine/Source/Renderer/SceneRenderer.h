#pragma once

#include "Renderer/Camera/EditorCamera.h"
#include "Renderer/Pipeline.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <nvrhi/nvrhi.h>

namespace Eppo
{
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<Scene>& scene, uint32_t width = 0, uint32_t height = 0);

		auto BeginScene(const ScopedPtr<EditorCamera>& camera) -> void;
		auto EndScene() -> void;

		auto SubmitMesh() -> void;

		auto Resize(uint32_t width, uint32_t height) -> void;

	private:
		auto GeometryPass() -> void;

	private:
		std::shared_ptr<Scene> m_Scene = nullptr;
		nvrhi::CommandListHandle m_CommandList = nullptr;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		std::shared_ptr<Pipeline> m_GeometryPipeline = nullptr;

		struct DrawKey
		{
			UUID ID;

			bool operator<(const DrawKey& other) const
			{
				if (ID < other.ID)
					return true;
			}
		};

		struct DrawCommand
		{

		};

		std::map<DrawKey, DrawCommand> m_DrawCommands;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::mat4 ViewProjection;
			glm::vec4 Position;
		} m_CameraData{};

		struct DrawStatistics
		{
			uint32_t DrawCalls = 0;
			uint32_t Meshes = 0;
			uint32_t Instances = 0;
		} m_DrawStatistics;
	};
}