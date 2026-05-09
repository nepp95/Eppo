#pragma once

#include "Panels/Panel.h"

namespace Eppo
{
	class SceneHierarchyPanel : public Panel
	{
	public:
		auto RenderGui() -> void override;
	};
}