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

int add_font_list(list_node *list, char *path, char *opt_dirs);

int free_font_list(list_node *list);

struct tfont * get_font_list(list_node *list, char *name);

int font_parse_str(struct tfont * font, list_node *list_ret, int pool_idx, char *txt, double *w);

int font_str_w(struct tfont * font, char *txt, double *w);

#endif