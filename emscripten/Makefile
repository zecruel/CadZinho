SRC_PATH = ./src/
EXT_SRC_PATH = ./ext_src/
INCLUDE_PATHS = $(SRC_PATH) $(EXT_SRC_PATH) $(wildcard $(EXT_SRC_PATH)*/)
CC = emcc
CPP = em++
LINKER = em++

CPP_COMPILER_FLAGS = -c -DMANIFOLD_PAR=-1 -DMANIFOLD_CROSS_SECTION
C_COMPILER_FLAGS = -c -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LINKER_FLAGS = -s USE_SDL=2 -s TOTAL_MEMORY=200MB -sFULL_ES2 -s STACK_SIZE=10MB -s ALLOW_MEMORY_GROWTH -s OFFSCREEN_FRAMEBUFFER --preload-file lang --preload-file script --preload-file samples --shell-file shell_minimal.html
INCLUDE = $(patsubst %,-I%,. $(INCLUDE_PATHS)) -s USE_SDL=2
LIBRARY_PATHS = -L.
EXE=cadzinho.html

SRC_C=$(wildcard $(SRC_PATH)*.c) $(wildcard $(EXT_SRC_PATH)*/*.c)
SRC_CPP=$(wildcard $(SRC_PATH)*.cpp) $(wildcard $(EXT_SRC_PATH)*/*.cpp)
OBJ_C=$(SRC_C:.c=.o)
OBJ_CPP=$(SRC_CPP:.cpp=.obj)

all: $(SRC_C) $(SRC_CPP) $(EXE)

$(EXE): $(OBJ_CPP) $(OBJ_C)
	$(LINKER) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ_C) $(OBJ_CPP) -o $@

$(OBJ_C):
	$(CC) $(INCLUDE) $(C_COMPILER_FLAGS) -o $@ $(@:.o=.c)

$(OBJ_CPP):
	$(CPP) $(INCLUDE) $(CPP_COMPILER_FLAGS) -o $@ $(@:.obj=.cpp)

clean:
	rm -rf run $(OBJ_C)
	rm -rf run $(OBJ_CPP)
