SRC_PATH=./src/
CC=gcc
CPP=g++
#COMPILER_FLAGS = -g -c -MD
COMPILER_FLAGS = -c -MD
LINKER_FLAGS = -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lSDL2_net -lopengl32 -lm -lGLU32 -lGLEW32 -lole32 -lmanifoldc -lmanifold
#LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32 -lGLEW32 -llua
INCLUDE_PATHS = -I. -I./src/ -I/mingw64/include -I/mingw64/include/SDL2 -I./include
LIBRARY_PATHS = -L/mingw64/lib -L. -L./lib
EXE=cadzinho.exe

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))
DEP=$(OBJ:.o=.d)

all: $(SRC) $(EXE)

cz.res: cz.rc
	windres cz.rc -O coff -o cz.res

$(EXE): $(OBJ) cz.res
	$(CPP) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) cz.res -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
	rm -rf run $(DEP)

-include $(DEP)
