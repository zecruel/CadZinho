SRC_PATH=./src/
CC=gcc
#COMPILER_FLAGS = -g -c
COMPILER_FLAGS = -c -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LINKER_FLAGS = `pkg-config --libs sdl2` -lm -lGL -lGLU -lGLEW -lSDL2_net
INCLUDE_PATHS = -I. -I./src/ `pkg-config --cflags sdl2`
LIBRARY_PATHS = -L/usr/lib -L.
EXE=cadzinho

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
