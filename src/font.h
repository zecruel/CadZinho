#ifndef _FONT_LIB
#define _FONT_LIB

#include "dxf.h"
#include "list.h"
#include "shape.h"
//#include "shape2.h"
#include "tt_graph.h"
#include "graph.h"

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
	double above, below;
	
	void *data;
};

struct tfont * add_font_list(list_node *list, char *path, char *opt_dirs);

struct tfont * add_shp_font_list(list_node *list, char *name, char *buf);

int free_font_list(list_node *list);

struct tfont * get_font_list(list_node *list, char *name);

int font_parse_str(struct tfont * font, list_node *list_ret, int pool_idx, char *txt, double *w);

int font_str_w(struct tfont * font, char *txt, double *w);

graph_obj * font_parse_cp(struct tfont * font, int cp, int prev_cp, int pool_idx, double *w);

#endif