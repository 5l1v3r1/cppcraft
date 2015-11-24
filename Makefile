#######################
#  cppcraft makefile  #
#######################

# output file
OUTPUT = ./Debug/cppcraft
SERVER = ./Debug/server

# code folder
SOURCE =  source source/db source/gui source/meshes source/sound
SOURCE += source/generator source/generator/biomegen source/generator/terrain
SOURCE += source/generator/processing source/generator/objects

# server folders
SDIRS = source/server

# resource file
ifeq ($(OS),Windows_NT)
CCRES = res/cppcraft.o
SRRES = res/server.o
endif

##############################################################
CC = g++
# compiler
CC += $(BUILDOPT) -std=gnu++11 -mstackrealign
# compiler flags
CFLAGS = -c -MMD -Wall -Wextra -pedantic -Iinc
# linker flags
LFLAGS  = -static-libgcc -static-libstdc++ -Llib -L. \
	-library -lbass -llzo2 -l:libGLEW.a -DGLEW_STATIC `pkg-config --static --libs glfw3` -Wl,-rpath,../lib
ifeq ($(OS),Windows_NT)
	LFLAGS  = -Llib -static -library -lpthread -lbassdll -lglfw3 -lgdi32 -lglew32s -lopengl32 -llzo2 -lws2_32
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
	$(CC) $(CFLAGS) $< -o $@

# compile each .cpp to .o
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

# recipe for building .o from .rc files
%.o : %.rc
	$(RES) $(RFLAGS) $< -o $@

# convert .c to .o
CCOBJS  = $(CCMODS:.c=.o)
# convert .cpp to .o
CXXOBJS = $(CXXMODS:.cpp=.o)
# convert .o to .d
DEPENDS = $(CXXOBJS:.o=.d) $(CCOBJS:.o=.d)

.PHONY: clean all debug fast

fast: BUILDOPT = -O3 -ffast-math -march=native
fast: all

debug: BUILDOPT = -ggdb3 -fstack-protector -DDEBUG
debug: all

prof: BUILDOPT = -O2 -march=native -pg
prof: all

# link all OBJS using CC and link with LFLAGS, then output to OUTPUT
all: $(CXXOBJS) $(CCOBJS) $(CCRES)
	$(CC) $(CXXOBJS) $(CCOBJS) $(CCRES) $(LFLAGS) -o $(OUTPUT)

server: $(SERVER_OBJS) $(SRRES)
	$(CC) $(SERVER_OBJS) $(SRRES) $(SLFLAGS) -o $(SERVER)

# remove each known .o file, and output
clean:
	$(RM) $(CXXOBJS) $(CCOBJS) $(CCRES) $(DEPENDS) *~ $(OUTPUT).*

-include $(DEPENDS)
