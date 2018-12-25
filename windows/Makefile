SRC_PATH=./src/
CC=gcc
COMPILER_FLAGS = -g -c
LINKER_FLAGS = -lSDL2main -lSDL2 -lcomdlg32 -lole32 -lmingw32 -llua
INCLUDE_PATHS = -I. -I./src/ -I/usr/include/SDL2
LIBRARY_PATHS = -L./lua-5.3.4/src/lib
EXE=teste9.exe

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
