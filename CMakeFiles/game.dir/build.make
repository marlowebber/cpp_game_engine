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
CMAKE_SOURCE_DIR = /home/marlo/Documents/cpp_game_engine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/marlo/Documents/cpp_game_engine

# Include any dependencies generated for this target.
include CMakeFiles/game.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/game.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/game.dir/flags.make

CMakeFiles/game.dir/src/utilities.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/src/utilities.cpp.o: src/utilities.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/game.dir/src/utilities.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/game.dir/src/utilities.cpp.o -c /home/marlo/Documents/cpp_game_engine/src/utilities.cpp

CMakeFiles/game.dir/src/utilities.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/src/utilities.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/marlo/Documents/cpp_game_engine/src/utilities.cpp > CMakeFiles/game.dir/src/utilities.cpp.i

CMakeFiles/game.dir/src/utilities.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/src/utilities.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/marlo/Documents/cpp_game_engine/src/utilities.cpp -o CMakeFiles/game.dir/src/utilities.cpp.s

CMakeFiles/game.dir/src/main.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/src/main.cpp.o: src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/game.dir/src/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/game.dir/src/main.cpp.o -c /home/marlo/Documents/cpp_game_engine/src/main.cpp

CMakeFiles/game.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/marlo/Documents/cpp_game_engine/src/main.cpp > CMakeFiles/game.dir/src/main.cpp.i

CMakeFiles/game.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/marlo/Documents/cpp_game_engine/src/main.cpp -o CMakeFiles/game.dir/src/main.cpp.s

CMakeFiles/game.dir/src/game.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/src/game.cpp.o: src/game.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/game.dir/src/game.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/game.dir/src/game.cpp.o -c /home/marlo/Documents/cpp_game_engine/src/game.cpp

CMakeFiles/game.dir/src/game.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/src/game.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/marlo/Documents/cpp_game_engine/src/game.cpp > CMakeFiles/game.dir/src/game.cpp.i

CMakeFiles/game.dir/src/game.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/src/game.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/marlo/Documents/cpp_game_engine/src/game.cpp -o CMakeFiles/game.dir/src/game.cpp.s

CMakeFiles/game.dir/src/graphics.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/src/graphics.cpp.o: src/graphics.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/game.dir/src/graphics.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/game.dir/src/graphics.cpp.o -c /home/marlo/Documents/cpp_game_engine/src/graphics.cpp

CMakeFiles/game.dir/src/graphics.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/src/graphics.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/marlo/Documents/cpp_game_engine/src/graphics.cpp > CMakeFiles/game.dir/src/graphics.cpp.i

CMakeFiles/game.dir/src/graphics.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/src/graphics.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/marlo/Documents/cpp_game_engine/src/graphics.cpp -o CMakeFiles/game.dir/src/graphics.cpp.s

CMakeFiles/game.dir/src/menus.cpp.o: CMakeFiles/game.dir/flags.make
CMakeFiles/game.dir/src/menus.cpp.o: src/menus.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/game.dir/src/menus.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/game.dir/src/menus.cpp.o -c /home/marlo/Documents/cpp_game_engine/src/menus.cpp

CMakeFiles/game.dir/src/menus.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/game.dir/src/menus.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/marlo/Documents/cpp_game_engine/src/menus.cpp > CMakeFiles/game.dir/src/menus.cpp.i

CMakeFiles/game.dir/src/menus.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/game.dir/src/menus.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/marlo/Documents/cpp_game_engine/src/menus.cpp -o CMakeFiles/game.dir/src/menus.cpp.s

# Object files for target game
game_OBJECTS = \
"CMakeFiles/game.dir/src/utilities.cpp.o" \
"CMakeFiles/game.dir/src/main.cpp.o" \
"CMakeFiles/game.dir/src/game.cpp.o" \
"CMakeFiles/game.dir/src/graphics.cpp.o" \
"CMakeFiles/game.dir/src/menus.cpp.o"

# External object files for target game
game_EXTERNAL_OBJECTS =

game: CMakeFiles/game.dir/src/utilities.cpp.o
game: CMakeFiles/game.dir/src/main.cpp.o
game: CMakeFiles/game.dir/src/game.cpp.o
game: CMakeFiles/game.dir/src/graphics.cpp.o
game: CMakeFiles/game.dir/src/menus.cpp.o
game: CMakeFiles/game.dir/build.make
game: box2d/build/bin/libbox2d.a
game: /usr/local/lib/libSDL2main.a
game: /usr/local/lib/libSDL2-2.0.so.0.16.0
game: /usr/lib/x86_64-linux-gnu/libOpenGL.so
game: /usr/lib/x86_64-linux-gnu/libGLX.so
game: /usr/lib/x86_64-linux-gnu/libGLU.so
game: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.1.71.0
game: /usr/lib/x86_64-linux-gnu/libboost_thread.so.1.71.0
game: /usr/lib/gcc/x86_64-linux-gnu/9/libgomp.so
game: /usr/lib/x86_64-linux-gnu/libpthread.so
game: /usr/lib/x86_64-linux-gnu/libboost_atomic.so.1.71.0
game: CMakeFiles/game.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/marlo/Documents/cpp_game_engine/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable game"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/game.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/game.dir/build: game

.PHONY : CMakeFiles/game.dir/build

CMakeFiles/game.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/game.dir/cmake_clean.cmake
.PHONY : CMakeFiles/game.dir/clean

CMakeFiles/game.dir/depend:
	cd /home/marlo/Documents/cpp_game_engine && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/marlo/Documents/cpp_game_engine /home/marlo/Documents/cpp_game_engine /home/marlo/Documents/cpp_game_engine /home/marlo/Documents/cpp_game_engine /home/marlo/Documents/cpp_game_engine/CMakeFiles/game.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/game.dir/depend

