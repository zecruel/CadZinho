SRC_PATH = ./src/
EXT_SRC_PATH = ./ext_src/
INCLUDE_PATHS = $(SRC_PATH) $(EXT_SRC_PATH) $(wildcard $(EXT_SRC_PATH)*/)

CC = gcc
CPP = g++
LINKER = g++

CPP_COMPILER_FLAGS = -c -DMANIFOLD_PAR=-1 -DMANIFOLD_CROSS_SECTION
C_COMPILER_FLAGS = -c -MD -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LINKER_FLAGS = -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lopengl32 -lm -lGLU32 -lGLEW32
INCLUDE = $(patsubst %,-I%,. /ucrt64/include/ /ucrt64/include/SDL2/ $(INCLUDE_PATHS))
LIBRARY_PATHS = -L/ucrt64/lib/ -L.
EXE=cadzinho.exe

SRC_C=$(wildcard $(SRC_PATH)*.c) $(wildcard $(EXT_SRC_PATH)*/*.c)
SRC_CPP=$(wildcard $(SRC_PATH)*.cpp) $(wildcard $(EXT_SRC_PATH)*/*.cpp)
OBJ_C=$(SRC_C:.c=.o)
OBJ_CPP=$(SRC_CPP:.cpp=.obj)
DEP_C=$(OBJ_C:.o=.d)

all: $(SRC_C) $(SRC_CPP) $(EXE)

cz.res: cz.rc
	windres cz.rc -O coff -o cz.res

$(EXE): $(OBJ_CPP) $(OBJ_C) cz.res
	$(LINKER) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ_C) $(OBJ_CPP) $(LINKER_FLAGS) cz.res -o $@

$(OBJ_C):
	$(CC) $(INCLUDE) $(C_COMPILER_FLAGS) -o $@ $(@:.o=.c)

$(OBJ_CPP):
	$(CPP) $(INCLUDE) $(CPP_COMPILER_FLAGS) -o $@ $(@:.obj=.cpp)

clean:
	rm -rf run $(OBJ_C)
	rm -rf run $(OBJ_CPP)
	rm -rf run $(DEP_C)

-include $(DEP_C)
