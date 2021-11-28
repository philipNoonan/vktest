file(GLOB CAMGRAB_SRC "*.cpp" "*.hpp" "*.h")
file(GLOB CAMGRAB_HEADERS "*.hpp" "*.h")

find_package(Ximea REQUIRED)
# set as if use opencv
find_package(OpenCV REQUIRED)
# cubert libs?

add_library(camera_grabber STATIC ${CAMGRAB_SRC})

target_include_directories(camera_grabber PUBLIC ${XIMEA_INCLUDE_PATHS})
target_link_libraries(camera_grabber ${XIMEA_LIBRARIES})

# set as if use opencv
target_link_libraries(camera_grabber ${OpenCV_LIBS} )	
target_include_directories(camera_grabber PUBLIC ${OpenCV_INCLUDE_DIRS})

