project(Editor)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/*.cpp")
set(INCLUDE_DIRS
    "Source"
    "${CMAKE_SOURCE_DIR}/EppoEngine/Source"
)

add_executable(EppoEditor ${SOURCES})

target_include_directories(EppoEditor PRIVATE ${INCLUDE_DIRS})
target_link_libraries(EppoEditor PRIVATE EppoEngine)

if (MSVC)
    target_compile_options(EppoEditor PRIVATE "/W3")
endif ()

add_custom_command(TARGET EppoEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:EppoEditor> $<TARGET_RUNTIME_DLLS:EppoEditor>
    COMMAND_EXPAND_LISTS
)

# Resource copy
add_custom_command(TARGET EppoEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Fonts" "$<TARGET_FILE_DIR:EppoEditor>/Resources/Fonts"
)

# Shader compilation
# CompileShader(PATH "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Shaders/geometry.hlsl")
# CompileShader(PATH "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Shaders/imgui.hlsl")

get_property(G_SHADER_OUTPUTS GLOBAL PROPERTY G_SHADER_OUTPUTS)
add_custom_target(CompileShaders ALL DEPENDS ${G_SHADER_OUTPUTS})
add_dependencies(EppoEditor CompileShaders)

# add_custom_command(TARGET CompileShaders POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Shaders/Bin" "$<TARGET_FILE_DIR:EppoEditor>/Resources/Shaders"
# )