GLM_SRC:= \
glm.hpp

GLM_OBJ:= \
glm.o

# DEF:=-DIMGUI_IMPL_OPENGL_LOADER_GLAD
INC:=-I../

.PHONY: all clean
.SUFFIXES: .o .libk.o .hpp

all: libglm.a
	
libglm.a: $(GLM_OBJ)
	ar rcs $@ $(IMGUI_OBJ)

.hpp.o:
	g++ -MD $(INC) -c $< -o $@

.hpp.libglm.o:
	g++ -MD -c $< -o $@

clean:
	rm -f *.o *.d *.a
