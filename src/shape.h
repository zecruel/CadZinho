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

struct Shp_typ{
	long num;
	char *name;
	int unicode;
	unsigned char *cmds;
	unsigned int cmd_size;
	struct Shp_typ *next;
}; 
typedef struct Shp_typ shp_typ;

void shp_font_add(shp_typ *shx_font, long num, char *name, unsigned char *cmds, unsigned int cmd_size);

void shp_font_free(shp_typ *shx_font);

shp_typ *shp_font_find(shp_typ *shx_font, long num);

shp_typ *shp_font_open(char *path);

shp_typ *shp_font_load(char *buf);

int shp_parse_str(shp_typ *font, list_node *list_ret, int pool_idx, char *txt, double *w, int force_ascii);

graph_obj *shp_parse_cp(shp_typ *shp_font, int pool_idx, int cp, double *w);

#endif