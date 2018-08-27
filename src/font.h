#ifndef _FONT_LIB
#define _FONT_LIB

#include "dxf.h"
#include "list.h"
#include "shape2.h"
#include "tt_graph.h"

enum font_type{
	FONT_SHP,
	FONT_TT,
	FONT_HER
};

struct tfont{
	char name[DXF_MAX_CHARS];
	char path[DXF_MAX_CHARS];
	enum font_type type;
	double std_size;
	
	void *data;
};

int add_font_list(list_node *list, char *path);

#endif