# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /home/marlo/Documents/cpp_game_engine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/marlo/Documents/cpp_game_engine

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/marlo/Documents/cpp_game_engine/CMakeFiles /home/marlo/Documents/cpp_game_engine/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/marlo/Documents/cpp_game_engine/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named game

# Build rule for target.
game: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 game
.PHONY : game

# fast build rule for target.
game/fast:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/build
.PHONY : game/fast

src/game.o: src/game.cpp.o

.PHONY : src/game.o

# target to build an object file
src/game.cpp.o:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/game.cpp.o
.PHONY : src/game.cpp.o

src/game.i: src/game.cpp.i

.PHONY : src/game.i

# target to preprocess a source file
src/game.cpp.i:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/game.cpp.i
.PHONY : src/game.cpp.i

src/game.s: src/game.cpp.s

.PHONY : src/game.s

# target to generate assembly for a file
src/game.cpp.s:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/game.cpp.s
.PHONY : src/game.cpp.s

src/graphics.o: src/graphics.cpp.o

.PHONY : src/graphics.o

# target to build an object file
src/graphics.cpp.o:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/graphics.cpp.o
.PHONY : src/graphics.cpp.o

src/graphics.i: src/graphics.cpp.i

.PHONY : src/graphics.i

# target to preprocess a source file
src/graphics.cpp.i:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/graphics.cpp.i
.PHONY : src/graphics.cpp.i

src/graphics.s: src/graphics.cpp.s

.PHONY : src/graphics.s

# target to generate assembly for a file
src/graphics.cpp.s:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/graphics.cpp.s
.PHONY : src/graphics.cpp.s

src/main.o: src/main.cpp.o

.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i

.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s

.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

src/utilities.o: src/utilities.cpp.o

.PHONY : src/utilities.o

# target to build an object file
src/utilities.cpp.o:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/utilities.cpp.o
.PHONY : src/utilities.cpp.o

src/utilities.i: src/utilities.cpp.i

.PHONY : src/utilities.i

# target to preprocess a source file
src/utilities.cpp.i:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/utilities.cpp.i
.PHONY : src/utilities.cpp.i

src/utilities.s: src/utilities.cpp.s

.PHONY : src/utilities.s

# target to generate assembly for a file
src/utilities.cpp.s:
	$(MAKE) -f CMakeFiles/game.dir/build.make CMakeFiles/game.dir/src/utilities.cpp.s
.PHONY : src/utilities.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... game"
	@echo "... src/game.o"
	@echo "... src/game.i"
	@echo "... src/game.s"
	@echo "... src/graphics.o"
	@echo "... src/graphics.i"
	@echo "... src/graphics.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
	@echo "... src/utilities.o"
	@echo "... src/utilities.i"
	@echo "... src/utilities.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

