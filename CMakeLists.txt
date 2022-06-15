# root level cmakelists.txt for deepsea project 

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(game)

# To make a debug build
set(CMAKE_CXX_FLAGS "-g")

find_package(OpenMP REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem thread)

# set options before add_subdirectory
# available options : TRACY_ENABLE , TRACY_ON_DEMAND , TRACY_NO_BROADCAST ,
# TRACY_NO_CODE_TRANSFER , ...
option ( TRACY_ENABLE "" ON)
option ( TRACY_ON_DEMAND "" ON)
add_subdirectory ( tracy ) # target : TracyClient or alias Tracy :: TracyClient


# Define the framework files.
set(SRCS
	src/game.h
	src/main.cpp
	src/main.h
	src/game.cpp
	src/graphics.cpp
	src/graphics.h
	src/menus.h
	src/menus.cpp
	src/utilities.h
	src/utilities.cpp
	src/neuro.cpp
	src/neuro.h
	src/TinyErode.h
	src/untitled_marlo_project.cpp
	src/untitled_marlo_project.h
	src/SimplexNoise.cpp
	src/SimplexNoise.h
	tracy/TracyClient.cpp
	tracy/Tracy.hpp
)

link_directories(
	${Boost_LIBRARY_DIRS}
	${CMAKE_SOURCE_DIR}/fann/src/
)

include_directories (
	${SDL2_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/fann/src/include/
)

add_executable(game ${SRCS} )

target_link_libraries (

	game
    Tracy::TracyClient 
	OpenMP::OpenMP_CXX
	${CMAKE_SOURCE_DIR}/fann/src/libfann.a
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}
)


