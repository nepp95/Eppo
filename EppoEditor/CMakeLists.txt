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