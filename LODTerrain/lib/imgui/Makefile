IMGUI_SRC:= \
imgui_impl_glfw.cpp \
imgui_impl_opengl3.cpp \
imgui_draw.cpp \
imgui_widgets.cpp \
imgui.cpp \
imgui_demo.cpp

IMGUI_OBJ:= \
imgui_draw.o \
imgui_widgets.o \
imgui_impl_glfw.o \
imgui_impl_opengl3.o \
imgui.o \
imgui_demo.o

DEF:=-DIMGUI_IMPL_OPENGL_LOADER_GLAD
INC:=-I../

.PHONY: all clean
.SUFFIXES: .o .libk.o .cpp

all: libimgui.a
	
libimgui.a: $(IMGUI_OBJ)
	ar rcs $@ $(IMGUI_OBJ)

.cpp.o:
	g++ -MD -DIMGUI_IMPL_OPENGL_LOADER_GLAD $(INC) -c $< -o $@

.cpp.libimgui.o:
	g++ -MD -c $< -o $@

clean:
	rm -f *.o *.d *.a
