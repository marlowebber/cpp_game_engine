# Install script for directory: /home/marlo/Documents/deepsea/fann/src/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/fann.h;/usr/local/include/doublefann.h;/usr/local/include/fann_internal.h;/usr/local/include/floatfann.h;/usr/local/include/fann_data.h;/usr/local/include/fixedfann.h;/usr/local/include/fann_activation.h;/usr/local/include/fann_cascade.h;/usr/local/include/fann_error.h;/usr/local/include/fann_train.h;/usr/local/include/fann_io.h;/usr/local/include/fann_cpp.h;/usr/local/include/fann_data_cpp.h;/usr/local/include/fann_training_data_cpp.h;/usr/local/include/parallel_fann.h;/usr/local/include/parallel_fann.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include" TYPE FILE FILES
    "/home/marlo/Documents/deepsea/fann/src/include/fann.h"
    "/home/marlo/Documents/deepsea/fann/src/include/doublefann.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_internal.h"
    "/home/marlo/Documents/deepsea/fann/src/include/floatfann.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_data.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fixedfann.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_activation.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_cascade.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_error.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_train.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_io.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_cpp.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_data_cpp.h"
    "/home/marlo/Documents/deepsea/fann/src/include/fann_training_data_cpp.h"
    "/home/marlo/Documents/deepsea/fann/src/include/parallel_fann.h"
    "/home/marlo/Documents/deepsea/fann/src/include/parallel_fann.hpp"
    )
endif()

