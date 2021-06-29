# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_SOURCE_DIR = /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator

# Include any dependencies generated for this target.
include lib/glm/glm/CMakeFiles/glm_shared.dir/depend.make

# Include the progress variables for this target.
include lib/glm/glm/CMakeFiles/glm_shared.dir/progress.make

# Include the compile flags for this target's objects.
include lib/glm/glm/CMakeFiles/glm_shared.dir/flags.make

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o: lib/glm/glm/CMakeFiles/glm_shared.dir/flags.make
lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o: lib/glm/glm/detail/glm.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o"
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glm_shared.dir/detail/glm.cpp.o -c /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm/detail/glm.cpp

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glm_shared.dir/detail/glm.cpp.i"
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm/detail/glm.cpp > CMakeFiles/glm_shared.dir/detail/glm.cpp.i

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glm_shared.dir/detail/glm.cpp.s"
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm/detail/glm.cpp -o CMakeFiles/glm_shared.dir/detail/glm.cpp.s

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.requires:

.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.requires

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.provides: lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.requires
	$(MAKE) -f lib/glm/glm/CMakeFiles/glm_shared.dir/build.make lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.provides.build
.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.provides

lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.provides.build: lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o


# Object files for target glm_shared
glm_shared_OBJECTS = \
"CMakeFiles/glm_shared.dir/detail/glm.cpp.o"

# External object files for target glm_shared
glm_shared_EXTERNAL_OBJECTS =

lib/glm/glm/libglm_shared.so: lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o
lib/glm/glm/libglm_shared.so: lib/glm/glm/CMakeFiles/glm_shared.dir/build.make
lib/glm/glm/libglm_shared.so: lib/glm/glm/CMakeFiles/glm_shared.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libglm_shared.so"
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glm_shared.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/glm/glm/CMakeFiles/glm_shared.dir/build: lib/glm/glm/libglm_shared.so

.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/build

lib/glm/glm/CMakeFiles/glm_shared.dir/requires: lib/glm/glm/CMakeFiles/glm_shared.dir/detail/glm.cpp.o.requires

.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/requires

lib/glm/glm/CMakeFiles/glm_shared.dir/clean:
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm && $(CMAKE_COMMAND) -P CMakeFiles/glm_shared.dir/cmake_clean.cmake
.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/clean

lib/glm/glm/CMakeFiles/glm_shared.dir/depend:
	cd /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm /home/drhollar/Documents/TerrainGeneratorOpenGL/TerrainGenerator/lib/glm/glm/CMakeFiles/glm_shared.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/glm/glm/CMakeFiles/glm_shared.dir/depend

