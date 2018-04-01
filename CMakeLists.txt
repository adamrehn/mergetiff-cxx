cmake_minimum_required(VERSION 3.8)
project(mergetiff)

# Provide an option to install headers only
option(HEADER_ONLY "disables building the mergetiff executable and only installs library headers" OFF)
if (NOT HEADER_ONLY)
	
	# Set the C++ standard to C++11
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
	
	# Enable all compiler warnings
	if (MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MD")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	endif()
	
	# Under Linux and macOS, link against pthreads and libdl
	if (UNIX)
		set(LIBRARIES ${LIBRARIES} pthread dl)
	endif()
	
	# If a custom directory has been specified for GDAL, propagate it to FindGDAL.cmake,
	# which uses the environment variables GDAL_DIR and GDAL_ROOT as hints
	set(ENV{GDAL_ROOT} "${GDAL_ROOT}")
	set(ENV{GDAL_DIR} "${GDAL_DIR}")
	
	# Link against GDAL
	find_package(GDAL REQUIRED)
	set(LIBRARIES ${LIBRARIES} "${GDAL_LIBRARY}")
	include_directories("${GDAL_INCLUDE_DIR}" SYSTEM)
	
	# Build mergetiff
	add_executable(mergetiff source/cli/mergetiff.cpp)
	target_link_libraries(mergetiff ${LIBRARIES})
	
endif()

# Installation rules
install(DIRECTORY source/lib/ DESTINATION include/mergetiff FILES_MATCHING PATTERN "*.h")
if (NOT HEADER_ONLY)
	install(TARGETS mergetiff RUNTIME DESTINATION bin)
endif()
