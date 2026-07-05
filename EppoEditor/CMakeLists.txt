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

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DOTNET_EXE "$ENV{DOTNET_ROOT}/dotnet.exe")

    file(GLOB_RECURSE MANAGED_SOURCES CONFIGURE_DEPENDS "${EppoScriptCore_MANAGED_SOURCE_DIR}/Source/*.cs")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/EppoScriptCore.Managed.dll"
        COMMAND ${DOTNET_EXE} build "${EppoScriptCore_MANAGED_SOURCE_DIR}/EppoScriptCore.Managed.csproj" -c $<CONFIG> -o ${CMAKE_CURRENT_BINARY_DIR} -v m --nologo
        COMMENT "Building EppoScriptCore.Managed"
        DEPENDS ${MANAGED_SOURCES} "${EppoScriptCore_MANAGED_SOURCE_DIR}/EppoScriptCore.Managed.csproj"
        VERBATIM
    )

    add_custom_target(EppoScriptCoreManaged ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/EppoScriptCore.Managed.dll")
    add_dependencies(EppoEditor EppoScriptCoreManaged)

    add_custom_command(TARGET EppoEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/runtimeconfig.json" $<TARGET_FILE_DIR:EppoEditor>
        COMMENT "Copying runtimeconfig.json to target directory"
    )
endif ()