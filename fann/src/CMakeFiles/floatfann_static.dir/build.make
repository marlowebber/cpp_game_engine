# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/marlo/Documents/deepsea/fann

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/marlo/Documents/deepsea/fann

# Include any dependencies generated for this target.
include src/CMakeFiles/floatfann_static.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/floatfann_static.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/floatfann_static.dir/flags.make

src/CMakeFiles/floatfann_static.dir/floatfann.c.o: src/CMakeFiles/floatfann_static.dir/flags.make
src/CMakeFiles/floatfann_static.dir/floatfann.c.o: src/floatfann.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/deepsea/fann/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/CMakeFiles/floatfann_static.dir/floatfann.c.o"
	cd /home/marlo/Documents/deepsea/fann/src && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/floatfann_static.dir/floatfann.c.o   -c /home/marlo/Documents/deepsea/fann/src/floatfann.c

src/CMakeFiles/floatfann_static.dir/floatfann.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/floatfann_static.dir/floatfann.c.i"
	cd /home/marlo/Documents/deepsea/fann/src && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/marlo/Documents/deepsea/fann/src/floatfann.c > CMakeFiles/floatfann_static.dir/floatfann.c.i

src/CMakeFiles/floatfann_static.dir/floatfann.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/floatfann_static.dir/floatfann.c.s"
	cd /home/marlo/Documents/deepsea/fann/src && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/marlo/Documents/deepsea/fann/src/floatfann.c -o CMakeFiles/floatfann_static.dir/floatfann.c.s

# Object files for target floatfann_static
floatfann_static_OBJECTS = \
"CMakeFiles/floatfann_static.dir/floatfann.c.o"

# External object files for target floatfann_static
floatfann_static_EXTERNAL_OBJECTS =

src/libfloatfann.a: src/CMakeFiles/floatfann_static.dir/floatfann.c.o
src/libfloatfann.a: src/CMakeFiles/floatfann_static.dir/build.make
src/libfloatfann.a: src/CMakeFiles/floatfann_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/marlo/Documents/deepsea/fann/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libfloatfann.a"
	cd /home/marlo/Documents/deepsea/fann/src && $(CMAKE_COMMAND) -P CMakeFiles/floatfann_static.dir/cmake_clean_target.cmake
	cd /home/marlo/Documents/deepsea/fann/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/floatfann_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/floatfann_static.dir/build: src/libfloatfann.a

.PHONY : src/CMakeFiles/floatfann_static.dir/build

src/CMakeFiles/floatfann_static.dir/clean:
	cd /home/marlo/Documents/deepsea/fann/src && $(CMAKE_COMMAND) -P CMakeFiles/floatfann_static.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/floatfann_static.dir/clean

src/CMakeFiles/floatfann_static.dir/depend:
	cd /home/marlo/Documents/deepsea/fann && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/marlo/Documents/deepsea/fann /home/marlo/Documents/deepsea/fann/src /home/marlo/Documents/deepsea/fann /home/marlo/Documents/deepsea/fann/src /home/marlo/Documents/deepsea/fann/src/CMakeFiles/floatfann_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/floatfann_static.dir/depend
