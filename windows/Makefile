SRC_PATH=./src/
CC=gcc
COMPILER_FLAGS = -g -c -MD
#LINKER_FLAGS = -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32 -lGLEW32 -llua
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32 -lGLEW32 -llua
INCLUDE_PATHS = -I. -I./src/ -I/usr/include/SDL2
LIBRARY_PATHS = -L/usr/lib -L.
EXE=cadzinho.exe

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))
DEP=$(OBJ:.o=.d)

all: $(SRC) $(EXE)

cz.res: cz.rc
	windres cz.rc -O coff -o cz.res

$(EXE): $(OBJ) cz.res
	$(CC) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) cz.res -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
	rm -rf run $(DEP)

-include $(DEP)