#######################
#  cppcraft makefile  #
#######################

# build options
# -Ofast -mfpmath=both -march=native
# -ggdb3 -fstack-protector
BUILDOPT = -ggdb3 -fstack-protector #-Ofast -ffast-math -march=native
# output file
OUTPUT   = ./Debug/cppcraft

# code folder
SOURCE = source source/generator source/generator/biomegen

# resource file
ifeq ($(OS),Windows_NT)
RESOURCES = res/cppcraft.rc
endif

##############################################################

# compiler
CC = g++ $(BUILDOPT) -std=c++11
# compiler flags
CCFLAGS = -c -MMD -Wall -Wextra -pedantic -Iinc
# linker flags
LFLAGS  = -static-libgcc -static-libstdc++ -Llib \
	-llibrary -lbass -llzo2 -llattice -lGLEW -DGLEW_STATIC `pkg-config --static --libs glfw3` -Wl,-rpath,../lib
ifeq ($(OS),Windows_NT)
	LFLAGS  = -Llib -static -llibrary -lpthread -lbassdll -lglfw3 -lgdi32 -lglew32s -lopengl32 -llzo2 -llattice -lws2_32
endif
# resource builder
RES = windres
# resource builder flags
RFLAGS = -O coff

##############################################################

# make pipeline
DIRECTORIES = $(SOURCE)
CCDIRS  = $(foreach dir, $(DIRECTORIES), $(dir)/*.c)
CCMODS  = $(wildcard $(CCDIRS))
CXXDIRS = $(foreach dir, $(DIRECTORIES), $(dir)/*.cpp)
CXXMODS = $(wildcard $(CXXDIRS))

# compile each .c to .o
.c.o:
	$(CC) $(CCFLAGS) $< -o $@

# compile each .cpp to .o
.cpp.o:
	$(CC) $(CCFLAGS) $< -o $@

# recipe for building .o from .rc files
%.o : %.rc
	$(RES) $(RFLAGS) $< -o $@

# convert .c to .o
CCOBJS  = $(CCMODS:.c=.o)
# convert .cpp to .o
CXXOBJS = $(CXXMODS:.cpp=.o)
# convert .o to .d
DEPENDS = $(CXXOBJS:.o=.d) $(CCOBJS:.o=.d)
# resource .rc to .o
CCRES   = $(RESOURCES:.rc=.o)

.PHONY: clean all

# link all OBJS using CC and link with LFLAGS, then output to OUTPUT
all: $(CXXOBJS) $(CCOBJS) $(CCRES)
	$(CC) $(CXXOBJS) $(CCOBJS) $(CCRES) $(LFLAGS) -o $(OUTPUT)

# remove each known .o file, and output
clean:
	$(RM) $(CXXOBJS) $(CCOBJS) $(CCRES) $(DEPENDS) *~ $(OUTPUT).*

-include $(DEPENDS)
