file(GLOB TEST_SRC "*.cpp" "*.hpp" "*.h")
file(GLOB TEST_HEADERS "*.hpp" "*.h")

set(TEST_NAME "colorize")

# wayland requires additional source files
IF(USE_WAYLAND_WSI)
	SET(SOURCE ${SOURCE} ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
ENDIF()

# Add shaders
set(SHADER_DIR_GLSL "../../data/shaders/glsl/pipeline")
file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/${TEST_NAME}.vert" "${SHADER_DIR_GLSL}/${TEST_NAME}.frag" "${SHADER_DIR_GLSL}/${TEST_NAME}.comp" "${SHADER_DIR_GLSL}/${TEST_NAME}.geom")
   source_group("Shaders\\glsl" FILES ${SHADERS_GLSL})


pybind11_add_module(${TEST_NAME} SHARED ${TEST_SRC} ${TEST_HEADERS} ${BASE_HEADERS} ${SHADERS_GLSL})
target_link_libraries(${TEST_NAME} PRIVATE base camera_grabber ${Vulkan_LIBRARY} ${WINLIBS} pybind11::module)
target_include_directories(${TEST_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})




#target_include_directories(${PROJ_NAME} PUBLIC ${XIMEA_INCLUDE_PATHS})
#target_link_libraries(${PROJ_NAME} ${XIMEA_LIBRARIES})

# set as if use opencv
#target_link_libraries(${PROJ_NAME} ${OpenCV_LIBS} )	
#target_include_directories(${PROJ_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})

set_target_properties(${TEST_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/tests)

if(RESOURCE_INSTALL_DIR)
install(TARGETS ${TEST_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()