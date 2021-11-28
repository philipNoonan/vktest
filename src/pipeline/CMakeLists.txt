file(GLOB PIPELINE_SRC "*.cpp" "*.hpp" "*.h")
file(GLOB PIPELINE_HEADERS "*.hpp" "*.h")

set(PROJ_NAME "pipeline")

#find_package(Ximea REQUIRED)
# set as if use opencv
#find_package(OpenCV REQUIRED)
# cubert libs?

#add_library(${PROJ_NAME} STATIC ${PIPELINE_SRC} ${BASE_HEADERS})



# wayland requires additional source files
IF(USE_WAYLAND_WSI)
	SET(SOURCE ${SOURCE} ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
ENDIF()

# Add shaders
set(SHADER_DIR_GLSL "../../data/shaders/glsl/${PROJ_NAME}")
file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/*.vert" "${SHADER_DIR_GLSL}/*.frag" "${SHADER_DIR_GLSL}/*.comp" "${SHADER_DIR_GLSL}/*.geom" "${SHADER_DIR_GLSL}/*.tesc" "${SHADER_DIR_GLSL}/*.tese" "${SHADER_DIR_GLSL}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${SHADER_DIR_GLSL}/*.rgen" "${SHADER_DIR_GLSL}/*.rchit" "${SHADER_DIR_GLSL}/*.rmiss" "${SHADER_DIR_GLSL}/*.rcall")
   source_group("Shaders\\glsl" FILES ${SHADERS_GLSL})


add_library(${PROJ_NAME} ${PIPELINE_SRC} ${PIPELINE_HEADERS} ${BASE_HEADERS} ${SHADERS_GLSL})
target_link_libraries(${PROJ_NAME} base camera_grabber ${Vulkan_LIBRARY} ${WINLIBS})
target_include_directories(${PROJ_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})


#target_include_directories(${PROJ_NAME} PUBLIC ${XIMEA_INCLUDE_PATHS})
#target_link_libraries(${PROJ_NAME} ${XIMEA_LIBRARIES})

# set as if use opencv
#target_link_libraries(${PROJ_NAME} ${OpenCV_LIBS} )	
#target_include_directories(${PROJ_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})

# CHANGE THE ABOVE TO ADD_LIBRARY not ADD_EXECUTABLE FOR THE PLUGINS !!!!!
set_target_properties(${PROJ_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(RESOURCE_INSTALL_DIR)
install(TARGETS ${PROJ_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
    

