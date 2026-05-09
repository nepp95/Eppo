#pragma once

#include <EppoEngine.h>

namespace Eppo
{
	class Panel
	{
	public:
		virtual auto RenderGui() -> void = 0;

	protected:
		auto GetSceneContext() -> std::shared_ptr<Scene>;
	};
}