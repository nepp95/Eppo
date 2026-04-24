include(FetchContent)

# Vulkan
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif ()

# RmlUi
find_package(RmlUi REQUIRED)

# Entt
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.15.0
)

# Glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# Glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
)
set(GLM_ENABLE_CXX_20 ON)

# Nvrhi
FetchContent_Declare(
    nvrhi
    GIT_REPOSITORY https://github.com/NVIDIA-RTX/NVRHI.git
    GIT_TAG 54100464714de88a5a5059d25808f5ccb914ad7d # 26 February 2026
)
set(NVRHI_BUILD_SHARED ON)

# Spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(entt glfw glm nvrhi spdlog)