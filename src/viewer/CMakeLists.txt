file(GLOB VIEWER_SRC "*.cpp" "*.hpp" "*.h")
file(GLOB VIEWER_HEADERS "*.hpp" "*.h")

find_package(Ximea REQUIRED)
# set as if use opencv
find_package(OpenCV REQUIRED)
# cubert libs?

add_executable(viewer WIN32 ${VIEWER_SRC} ${VIEWER_HEADERS} ${BASE_HEADERS} ${PIPELINE_HEADERS})
target_link_libraries(viewer pipeline base ${WINLIBS})

target_include_directories(viewer PUBLIC ${XIMEA_INCLUDE_PATHS})
target_link_libraries(viewer ${XIMEA_LIBRARIES})

# set as if use opencv
target_link_libraries(viewer ${OpenCV_LIBS} )	
target_include_directories(viewer PUBLIC ${OpenCV_INCLUDE_DIRS})

#set_target_properties(viewer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(RESOURCE_INSTALL_DIR)
install(TARGETS viewer DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (MSVC)
	add_custom_command(TARGET viewer
					   POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy_if_different
					   ${XIMEA_ROOT_DIR}/API/xiAPI/xiapi64.dll
					   ${PROJECT_BINARY_DIR}/bin/$(Configuration))
endif (MSVC)