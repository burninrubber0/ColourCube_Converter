cmake_minimum_required(VERSION 3.14)
project(ColourCube_Converter CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
	${SOURCES}
	src/main.cpp
	src/Converter.cpp
	)

set(HEADERS
	${HEADERS}
	include/Converter.h
	)

add_executable(ColourCube_Converter ${SOURCES} ${HEADERS})

# libbinaryio
set(LIBBINARYIO_ROOT ${ROOT}\\3rdparty\\libbinaryio)
add_subdirectory(3rdparty/libbinaryio "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libbinaryio" EXCLUDE_FROM_ALL)
target_include_directories(ColourCube_Converter PRIVATE ${LIBBINARYIO_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libbinaryio")

# Triggers to glTF
target_include_directories(ColourCube_Converter PRIVATE "${ROOT}/include")

target_link_libraries(ColourCube_Converter PRIVATE libbinaryio)

# VS stuff
set_property(DIRECTORY ${ROOT} PROPERTY VS_STARTUP_PROJECT ColourCube_Converter)
source_group(TREE ${ROOT} FILES ${SOURCES} ${HEADERS})