SRC_PATH=./src/
CC=gcc
LL=g++
#COMPILER_FLAGS = -g -c
COMPILER_FLAGS = -c -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LINKER_FLAGS = `pkg-config --libs sdl2` -lm -lGL -lGLU -lGLEW -lSDL2_net -lmanifoldc -lmanifold -lClipper2
INCLUDE_PATHS = -I. -I./src/ `pkg-config --cflags sdl2` -I./include
LIBRARY_PATHS = -L/usr/lib -L. -L./lib
EXE=cadzinho

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(LL) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
