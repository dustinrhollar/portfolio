#-----------------------------------------------------------------------#
# Program Structure
#
# EXECUTABLE_NAME
#	Name of the produced executable
# DIR_SOURCE
#	Directory containing source files
# DIR_INCLUDE
#	Directory containing header files
# DIR_EXTERNAL
#	Directory containing external libraries
#
#
# Source files are obtained by the Makefile using wildcards (glob)
# Header files are included in produced executable.
#
#
# Compile Options
#
# C_FLAGS
#	Compile commands
# EXT
#	Include directories for runtime execution. By default, EXTERNAL
#	and SOURCE are added.
# LIBS
#	Linked libraries for the program.
#
# 
# Build Settings: Debug Mode
#
# DBGDIR
#	Desired build directory. Default is "build/debug"
# DBGEXE
#	Desire executable name for debug. Default is DBGDIR + / + EXECUTABLE_NAME 
# DBGCFLAGS
#	Debug compile flags. Default is "-g -DDEBUG"
#
# 
# Build Settings: Release Mode
#
# RELDIR
#	Desired build directory. Default is "build/release"
# RELEXE
#	Desire executable name for debug. Default is RELDIR + / + EXECUTABLE_NAME 
# RELCFLAGS
#	Release compile flags. Default is "-DNDEBUG"
#
#-----------------------------------------------------------------------#
DIRECTORY != ./dir.sh
HOST ?= $(DIRECTORY)

#
# Directory names
#
EXECUTABLE_NAME = terrain
DIR_SOURCE      = src
DIR_INCLUDE     = inc
BUILD_INLCUDE   = $(HOST)/build/include 
DIR_EXTERNAL    = lib


#
# Source files
#
SOURCE  = $(DIR_SOURCE)/AppSource.cpp
HEADERS = $(wildcard $(DIR_INCLUDE)/*.h)
OBJ     = ${SOURCE:.cpp=.o}

#
# Compiler Flags
#
C_FLAGS = g++ -std=c++17
EXT     = -I$(DIR_INCLUDE) -I$(DIR_EXTERNAL) -I$(BUILD_INLCUDE)

#
# Library Dependencies
#
RUNTIME  = -Wl,-rpath,$(HOST)/build/lib 
LINK     = -L$(HOST)/build/lib
EXT_LIBS = libimgui.a
# removed: -lassimp
STANDARD = -ldl -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lm -lglfw
LIBS     = $(RUNTIME) $(LINK) $(EXT_LIBS) $(STANDARD)

#
# Debug Build Settings
#
DBGDIR    = $(HOST)/build/debug
DBGEXE    = $(DBGDIR)/$(EXECUTABLE_NAME)
DBGOBJS   = $(addprefix $(DBGDIR)/, $(OBJ))
DBGCFLAGS = -Wall -g -DDEBUG

#
# Release Build Settings
#
RELDIR    = $(HOST)/build/release
RELEXE    = $(RELDIR)/$(EXECUTABLE_NAME)
RELOBJS   = $(addprefix $(RELDIR)/, $(OBJ))
RELCFLAGS = -DNDEBUG


#-----------------------------------------------------------------------#
# Build commands:
#
# make
#	 Preps the build directory and then builds the program in debug mode
# make prep
#	 Creates the build directories for debug and release mode
# make debug
#     Builds the program with debug tools
# make release
#     Builds the program for release
# make run
#     Runs the program without debug info
# make test
#	 Runs the program with debug info
# make val
#     Runs the program with valgrind output.
# make clean
#     Removes all build files but not the build directories
# make remake
#	 Cleans build directory and remakes the program in debug mode
# make build_deps
#	Builds all external dependencies. Should be run after a fresh clone.
# make prep_deps
#    Installs includes and libs for ext deps. Should be run after build_deps
#-----------------------------------------------------------------------#

.PHONY: all clean      \
	    debug release  \
 	   prep remake    \
		test val run   \
		prep_deps build_deps

all: release

#
# Debug Rules
#
debug: $(DBGEXE)

$(DBGEXE) : $(DBGOBJS)
	$(C_FLAGS) $(DBGCFLAGS) $(EXT) $^ -o $@ $(LIBS)

$(DBGDIR)/%.o : %.cpp $(HEADERS)
	$(C_FLAGS) $(DBGCFLAGS) $(EXT) -c $< -o $@

#
# Release Rules
#
release: $(RELEXE)

$(RELEXE) : $(RELOBJS)
	$(C_FLAGS) $(RELCFLAGS) $(EXT) $^ -o $@ $(LIBS)

$(RELDIR)/%.o : %.cpp $(HEADERS)
	$(C_FLAGS) $(RELCFLAGS) $(EXT) -c $< -o $@

#
# Execution rules
#
run: $(DBGEXE)
	cd $(DBGDIR); ./$(EXECUTABLE_NAME)

test: $(DBGEXE)
	cd ./$(DBGDIR); ./$(EXECUTABLE_NAME)

val: $(DBGEXE)
	cd $(DBGDIR); valgrind ./$(EXECUTABLE_NAME)

#
# Dependencies
#
build_deps:
	(cd lib/imgui; make all; cp libimgui.a ../../; make clean)
	(cd lib/glm; make all; cp libglm.a ../../; make clean)

prep_deps:
	mkdir -p build/lib build/include
	@cp libimgui.a build/lib
	@cp libglm.a build/lib

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR)/src $(RELDIR)/src
	@cp -r resources/shaders $(DBGDIR)/
	@cp -r resources/shaders $(RELDIR)/
	@cp -r resources/images $(DBGDIR)/
	@cp -r resources/images $(RELDIR)/
	

remake: clean prep all

clean:
	@rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS)
	@rm -rf $(RELDIR)/shaders
	@rm -rf $(RELDIR)/images
	@rm -f $(RELDIR)/*.a $(DBGDIR)/*.a
