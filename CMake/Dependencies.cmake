find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif ()

find_package(entt REQUIRED)
if (NOT entt_FOUND)
    message(FATAL_ERROR "entt not found!")
endif ()

find_package(glm REQUIRED)
if (NOT glm_FOUND)
    message(FATAL_ERROR "glm not found!")
endif ()

find_package(imgui REQUIRED)
if (NOT imgui_FOUND)
    message(FATAL_ERROR "imgui not found!")
endif ()

find_package(nvrhi REQUIRED)
if (NOT nvrhi_FOUND)
    message(FATAL_ERROR "nvrhi not found!")
endif ()

find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found!")
endif ()