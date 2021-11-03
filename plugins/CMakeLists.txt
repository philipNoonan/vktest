# Function for building single plugin
function(buildPlugin PLUGIN_NAME)
	SET(PLUGIN_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME})
	message(STATUS "Generating project file for plugin in ${PLUGIN_FOLDER}")
	# Main
	file(GLOB SOURCE *.cpp ${BASE_HEADERS} ${PLUGIN_FOLDER}/*.cpp)
	SET(MAIN_CPP ${PLUGIN_FOLDER}/${PLUGIN_NAME}.cpp)
	if(EXISTS ${PLUGIN_FOLDER}/main.cpp)
		SET(MAIN_CPP ${PLUGIN_FOLDER}/main.cpp)
	ENDIF()
	if(EXISTS ${PLUGIN_FOLDER}/${PLUGIN_NAME}.h)
		SET(MAIN_HEADER ${PLUGIN_FOLDER}/${PLUGIN_NAME}.h)
	ENDIF()
	#find_package(OpenMP)
	# # imgui example requires additional source files
	# IF(${PLUGIN_NAME} STREQUAL "imgui")
    # we use imgui a lot...
		file(GLOB ADD_SOURCE "../external/imgui/*.cpp")
	 	SET(SOURCE ${SOURCE} ${ADD_SOURCE})
	# ENDIF()

	# wayland requires additional source files
	IF(USE_WAYLAND_WSI)
		SET(SOURCE ${SOURCE} ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
	ENDIF()
	# Add shaders
	set(SHADER_DIR_GLSL "../data/shaders/glsl/${PLUGIN_NAME}")
	file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/*.vert" "${SHADER_DIR_GLSL}/*.frag" "${SHADER_DIR_GLSL}/*.comp" "${SHADER_DIR_GLSL}/*.geom" "${SHADER_DIR_GLSL}/*.tesc" "${SHADER_DIR_GLSL}/*.tese" "${SHADER_DIR_GLSL}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${SHADER_DIR_GLSL}/*.rgen" "${SHADER_DIR_GLSL}/*.rchit" "${SHADER_DIR_GLSL}/*.rmiss" "${SHADER_DIR_GLSL}/*.rcall")

    source_group("Shaders\\glsl" FILES ${SHADERS_GLSL})

    # Add optional readme / tutorial
	file(GLOB README_FILES "${PLUGIN_FOLDER}/*.md")
	if(WIN32)
		add_executable(${PLUGIN_NAME} WIN32 ${MAIN_CPP} ${SOURCE} ${MAIN_HEADER} ${SHADERS_GLSL} ${README_FILES})
		target_link_libraries(${PLUGIN_NAME} base ${Vulkan_LIBRARY} ${WINLIBS})
		target_include_directories(${PLUGIN_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
	else(WIN32)
		add_executable(${PLUGIN_NAME} ${MAIN_CPP} ${SOURCE} ${MAIN_HEADER} ${SHADERS_GLSL} ${README_FILES})
		target_link_libraries(${PLUGIN_NAME} base )
		target_include_directories(${PLUGIN_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
	endif(WIN32)

    # CHANGE THE ABOVE TO ADD_LIBRARY not ADD_EXECUTABLE FOR THE PLUGINS !!!!!

	set_target_properties(${PLUGIN_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

	# if(OpenMP_CXX_FOUND)
	# 	target_compile_options(${PLUGIN_NAME} PRIVATE ${OpenMP_CXX_FLAGS})
	# 	IF(${EXAMPLE_NAME} STREQUAL "texture3d")
	# 		if(OpenMP_CXX_FOUND)
    # 			target_link_libraries(${EXAMPLE_NAME} OpenMP::OpenMP_CXX)
	# 		endif()	
	# 	endif()
	# endif()

	IF(${PLUGIN_NAME} STREQUAL "calibration")
	    target_link_libraries(${PLUGIN_NAME} ${OpenCV_LIBS} )	
		target_include_directories(${PLUGIN_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
	endif()


	if(RESOURCE_INSTALL_DIR)
		install(TARGETS ${PLUGIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()

endfunction(buildPlugin)

# Build all examples
function(buildPlugins)
	foreach(PLUGIN ${PLUGINS})
		buildPlugin(${PLUGIN})
	endforeach(PLUGIN)
endfunction(buildPlugins)

set(PLUGINS
    calibration
    demosaic
    color_correction
    super_resolution
    srgb_reconstruction	
)

buildPlugins()
