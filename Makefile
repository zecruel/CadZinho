# platform detection using OS environment variable for Windows
# and uname output of everthing else
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
else
    PLATFORM := $(shell uname)
endif

SRC_PATH=./src/
CC=gcc
COMPILER_FLAGS = -g -c -DPLATFORM_$(PLATFORM)

ifeq ($(PLATFORM),Darwin)
    OPENGL_LIBS := -framework OpenGL
    EXTRA_INCLUDE_PATHS := -I/usr/local/Cellar/lua/5.4.3/include/lua/
else
    OPENGL_LIBS := -lGL -lGLU
endif

LINKER_FLAGS = `sdl2-config --cflags --libs` -llua -lm $(OPENGL_LIBS) -lGLEW
INCLUDE_PATHS = -I. -I./src/ -I/usr/include/SDL2 -I/mingw64/include $(EXTRA_INCLUDE_PATHS)
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
