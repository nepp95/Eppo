project(Engine)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/*.cpp")
set(INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

add_library(EppoEngine STATIC ${SOURCES})

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
    target_compile_definitions(EppoEngine PUBLIC EP_PLATFORM_WINDOWS EP_HAS_DX12 EP_HAS_VULKAN)
    target_compile_options(EppoEngine PUBLIC /utf-8)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME}")
    target_compile_definitions(EppoEngine PUBLIC EP_PLATFORM_LINUX EP_HAS_VULKAN)
else()
    message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}' detected!")
endif()

target_include_directories(EppoEngine PUBLIC ${INCLUDE_DIRS})
target_link_directories(EppoEngine PUBLIC "$ENV{VULKAN_SDK}/Lib")
target_link_libraries(EppoEngine PUBLIC EnTT::EnTT glm::glm imgui::imgui nvrhi spdlog::spdlog Vulkan::Vulkan Vulkan::dxc_lib)
target_link_libraries(EppoEngine PRIVATE glfw)
target_precompile_headers(EppoEngine PRIVATE "Source/pch.h")
target_compile_definitions(EppoEngine PUBLIC
    $<$<CONFIG:Debug>:EP_DEBUG> $<$<CONFIG:RelWithDebInfo>:EP_RELEASE> $<$<CONFIG:Release>:EP_DIST>
)

if (MSVC)
    target_compile_options(EppoEngine PRIVATE "/W3")
endif ()