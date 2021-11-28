# - Find ximea
# Find the native xiAPI includes and libraries
#
#  XIMEA_INCLUDE_DIR - where to find xiAPI.h, etc.
#  XIMEA_LIBRARIES   - List of libraries when using xiapi64.
#  XIMEA_FOUND       - True if xiapi64 found.
# m3api/xiApi.h for linux
# xiApi.h for win32

if (UNIX)
set(XIMEA_ROOT_DIR "/opt/XIMEA/")

endif()

if (WIN32)
set(XIMEA_ROOT_DIR "C:/XIMEA/")
endif()

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${XIMEA_ROOT_DIR}/API/xiAPI/
	)

if (UNIX)

find_library(XIMEA_LIBRARY 
	NAMES m3api
	PATHS ${LIBRARY_PATHS}
	)
endif()

if (WIN32)
find_library(XIMEA_LIBRARY 
	NAMES xiapi64
	PATHS ${LIBRARY_PATHS}
	)	

endif()

message(STATUS "xim static_lib " ${XIMEA_LIBRARY})	

if (UNIX)

find_path(XIMEA_INCLUDE_PATH m3api/xiApi.h
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${XIMEA_ROOT_DIR}/API/xiAPI/
	)

endif()

if (WIN32)

find_path(XIMEA_INCLUDE_PATH xiApi.h
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${XIMEA_ROOT_DIR}/API/xiAPI/
	)

endif()

message(STATUS "xim dirs " ${XIMEA_INCLUDE_PATH})	

if(XIMEA_LIBRARY AND XIMEA_INCLUDE_PATH)
	set(XIMEA_FOUND TRUE)
	set(XIMEA_INCLUDE_PATHS ${XIMEA_INCLUDE_PATH} CACHE STRING "The include paths needed to use the ximea api")
    set(XIMEA_LIBRARIES ${XIMEA_LIBRARY} CACHE STRING "The libraries needed to use the ximea api")
endif()

mark_as_advanced(
    XIMEA_INCLUDE_PATH
    XIMEA_LIBRARY
	)
