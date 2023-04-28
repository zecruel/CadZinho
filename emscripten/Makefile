SRC_PATH=./src/
CC=emcc
COMPILER_FLAGS = -c
LINKER_FLAGS = -s USE_SDL=2 -s TOTAL_MEMORY=200MB -s STACK_SIZE=10MB -s ALLOW_MEMORY_GROWTH --preload-file lang --preload-file script --preload-file samples --shell-file shell_minimal.html
INCLUDE_PATHS = -I. -I./src/ -s USE_SDL=2
LIBRARY_PATHS = -L.
EXE=cadzinho.html

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBRARY_PATHS) $(OBJ) $(LINKER_FLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
