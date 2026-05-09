find_program(DXC_EXE dxc HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)

function(CompileShader)
	cmake_parse_arguments(ARG "" "PATH" "" ${ARGN})
	message(STATUS "Compiling shader '${ARG_PATH}'")

	get_filename_component(SHADER_NAME "${ARG_PATH}" NAME_WE)

	foreach (STAGE IN ITEMS vs ps)
		if (STAGE STREQUAL "vs")
			set(SHADER_PROFILE "vs_6_0")
			set(SHADER_ENTRY "VSMain")
			set(SHADER_SUFFIX "vert")
		else ()
			set(SHADER_PROFILE "ps_6_0")
			set(SHADER_ENTRY "PSMain")
			set(SHADER_SUFFIX "pixel")
		endif ()

		# Vulkan binary (SPIR-V)
		set(SPV_OUTPUT "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Shaders/Bin/${SHADER_NAME}.${SHADER_SUFFIX}.spv")
		add_custom_command(
			OUTPUT ${SPV_OUTPUT}
			COMMAND "${DXC_EXE}" -T "${SHADER_PROFILE}" -E "${SHADER_ENTRY}" -spirv -fvk-t-shift 0 0 -fvk-s-shift 128 0 -fvk-b-shift 256 0 -fvk-u-shift 384 0 "${ARG_PATH}" -Fo "${SPV_OUTPUT}"
			DEPENDS ${ARG_PATH}
			VERBATIM
		)

		set_property(GLOBAL APPEND PROPERTY G_SHADER_OUTPUTS ${SPV_OUTPUT})

		# DirectX binary (DXIL)
		if (WIN32)
			set(DXIL_OUTPUT "${CMAKE_SOURCE_DIR}/EppoEditor/Resources/Shaders/Bin/${SHADER_NAME}.${SHADER_SUFFIX}.dxil")
			add_custom_command(
				OUTPUT ${DXIL_OUTPUT}
				COMMAND "${DXC_EXE}" -T "${SHADER_PROFILE}" -E "${SHADER_ENTRY}" "${ARG_PATH}" -Fo "${DXIL_OUTPUT}"
				DEPENDS ${ARG_PATH}
				VERBATIM
			)

			set_property(GLOBAL APPEND PROPERTY G_SHADER_OUTPUTS ${DXIL_OUTPUT})
		endif ()
	endforeach ()
endfunction()