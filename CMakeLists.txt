# root level cmakelists.txt for deepsea project 

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(game)

# To make a debug build


# set(CMAKE_CXX_FLAGS "-g ") 					# this one makes a regular build and it is done quickly
#set(CMAKE_CXX_FLAGS "-g -O2 -flto") 					# 

 set(CMAKE_CXX_FLAGS " -Ofast -flto") 					# this one makes a crazy fast build but it takes ages. it's my favorite to use.

# set(CMAKE_CXX_FLAGS " -Ofast -flto -fsanitize=address") 					# this one makes a crazy fast build but it takes ages. it's my favorite to use.



 #set(CMAKE_CXX_FLAGS " -Ofast -flto -fprofile-generate")               # use profiling instrumentation (use this first and run the program once)
 #set(CMAKE_CXX_FLAGS "-Ofast -flto -fprofile-use -fprofile-correction") # apply what was learned from profiling (use this after profiling instrumentation, for best results)


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
	src/main.cpp
	src/main.h
	src/graphics.cpp
	src/graphics.h
	src/utilities.h
	src/utilities.cpp
	src/TinyErode.h
	src/untitled_marlo_project.cpp
	src/untitled_marlo_project.h
	src/SimplexNoise.cpp
	src/SimplexNoise.h
	src/content.h
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
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}
)


