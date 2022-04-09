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
	src/untitled_marlo_project.cpp
	src/untitled_marlo_project.h
)

link_directories(
	${Boost_LIBRARY_DIRS}
)

include_directories (
	${SDL2_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

add_executable(game ${SRCS} )

target_link_libraries (
	game
	OpenMP::OpenMP_CXX
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}
)


