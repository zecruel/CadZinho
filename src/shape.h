#ifndef _SHAPE_LIB
#define _SHAPE_LIB

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <locale.h>
#include "graph.h"
#include "dxf.h"

enum shp_file_type{
	SHP_NONE,
	SHP_SHAPES,
	SHP_UNIFONT,
	SHP_BIGFONT
};

struct Shape{
	long num;
	char *name;
	int unicode;
	unsigned char *cmds;
	unsigned int cmd_size;
	struct Shape *next;
}; 
typedef struct Shape shape;

void shp_font_add(shape *shx_font, long num, char *name, unsigned char *cmds, unsigned int cmd_size);

void shp_font_free(shape *shx_font);

shape *shp_font_find(shape *shx_font, long num);

shape *shp_font_open(char *path);

int shp_parse_str(shape *font, list_node *list_ret, int pool_idx, char *txt, double *w);

#endif