SRC_PATH=./src/
CC=gcc
COMPILER_FLAGS = -g -c
LINKER_FLAGS = `sdl2-config --cflags --libs` -llua -lm
INCLUDE_PATHS = -I. -I./src/ -I/usr/include/SDL2
LIBRARY_PATHS = -L/usr/lib/ -L.
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
