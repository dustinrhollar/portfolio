# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.12.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.12.1/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/dustinhollar/Desktop/glfw-3.2.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/dustinhollar/Desktop/glfw-3.2.1

# Include any dependencies generated for this target.
include tests/CMakeFiles/joysticks.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/joysticks.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/joysticks.dir/flags.make

tests/CMakeFiles/joysticks.dir/joysticks.c.o: tests/CMakeFiles/joysticks.dir/flags.make
tests/CMakeFiles/joysticks.dir/joysticks.c.o: tests/joysticks.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dustinhollar/Desktop/glfw-3.2.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/joysticks.dir/joysticks.c.o"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/joysticks.dir/joysticks.c.o   -c /Users/dustinhollar/Desktop/glfw-3.2.1/tests/joysticks.c

tests/CMakeFiles/joysticks.dir/joysticks.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/joysticks.dir/joysticks.c.i"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/dustinhollar/Desktop/glfw-3.2.1/tests/joysticks.c > CMakeFiles/joysticks.dir/joysticks.c.i

tests/CMakeFiles/joysticks.dir/joysticks.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/joysticks.dir/joysticks.c.s"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/dustinhollar/Desktop/glfw-3.2.1/tests/joysticks.c -o CMakeFiles/joysticks.dir/joysticks.c.s

tests/CMakeFiles/joysticks.dir/__/deps/glad.c.o: tests/CMakeFiles/joysticks.dir/flags.make
tests/CMakeFiles/joysticks.dir/__/deps/glad.c.o: deps/glad.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dustinhollar/Desktop/glfw-3.2.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/joysticks.dir/__/deps/glad.c.o"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/joysticks.dir/__/deps/glad.c.o   -c /Users/dustinhollar/Desktop/glfw-3.2.1/deps/glad.c

tests/CMakeFiles/joysticks.dir/__/deps/glad.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/joysticks.dir/__/deps/glad.c.i"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/dustinhollar/Desktop/glfw-3.2.1/deps/glad.c > CMakeFiles/joysticks.dir/__/deps/glad.c.i

tests/CMakeFiles/joysticks.dir/__/deps/glad.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/joysticks.dir/__/deps/glad.c.s"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/dustinhollar/Desktop/glfw-3.2.1/deps/glad.c -o CMakeFiles/joysticks.dir/__/deps/glad.c.s

# Object files for target joysticks
joysticks_OBJECTS = \
"CMakeFiles/joysticks.dir/joysticks.c.o" \
"CMakeFiles/joysticks.dir/__/deps/glad.c.o"

# External object files for target joysticks
joysticks_EXTERNAL_OBJECTS =

tests/joysticks: tests/CMakeFiles/joysticks.dir/joysticks.c.o
tests/joysticks: tests/CMakeFiles/joysticks.dir/__/deps/glad.c.o
tests/joysticks: tests/CMakeFiles/joysticks.dir/build.make
tests/joysticks: src/libglfw3.a
tests/joysticks: tests/CMakeFiles/joysticks.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dustinhollar/Desktop/glfw-3.2.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable joysticks"
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/joysticks.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/joysticks.dir/build: tests/joysticks

.PHONY : tests/CMakeFiles/joysticks.dir/build

tests/CMakeFiles/joysticks.dir/clean:
	cd /Users/dustinhollar/Desktop/glfw-3.2.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/joysticks.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/joysticks.dir/clean

tests/CMakeFiles/joysticks.dir/depend:
	cd /Users/dustinhollar/Desktop/glfw-3.2.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dustinhollar/Desktop/glfw-3.2.1 /Users/dustinhollar/Desktop/glfw-3.2.1/tests /Users/dustinhollar/Desktop/glfw-3.2.1 /Users/dustinhollar/Desktop/glfw-3.2.1/tests /Users/dustinhollar/Desktop/glfw-3.2.1/tests/CMakeFiles/joysticks.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/joysticks.dir/depend

