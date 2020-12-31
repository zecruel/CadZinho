#ifndef _DXF_CREATE_LIB
#define _DXF_CREATE_LIB
#define ACT_CHARS 32
#define DO_PAGES 1000
#define DO_PAGE 10000

#define MAX_FIT_PTS 1000

#include "dxf.h"
#include "list.h"
#include "graph.h"
#include "dxf_hatch.h"

struct do_item {
	struct do_item *prev;
	struct do_item *next;
	dxf_node *old_obj;
	dxf_node *new_obj;
};

struct do_entry {
	struct do_entry *prev;
	struct do_entry *next;
	char text[ACT_CHARS];
	struct do_item *list;
	struct do_item *current;
};

struct do_list {
	int count;
	struct do_entry *list;
	struct do_entry *current;
};

enum do_pool_action{
	ADD_DO_ITEM,
	ZERO_DO_ITEM,
	ADD_DO_ENTRY,
	ZERO_DO_ENTRY,
	FREE_DO_ALL
};

struct do_pool_slot{
	void *pool[DO_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};

void * do_mem_pool(enum dxf_pool_action action);

int do_add_item(struct do_entry *entry, dxf_node *old_obj, dxf_node *new_obj);

int do_add_entry(struct do_list *list, char *text);

int init_do_list(struct do_list *list);

int do_undo(struct do_list *list);

int do_redo(struct do_list *list);

void dxf_obj_transverse(dxf_node *source);

dxf_node * dxf_new_line (double x0, double y0, double z0,
double x1, double y1, double z1,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_lwpolyline (double x0, double y0, double z0,
double bulge, int color, char *layer, char *ltype, int lw, int paper, int pool);

int dxf_lwpoly_append (dxf_node * poly,
double x0, double y0, double z0, double bulge, int pool);

int dxf_lwpoly_remove (dxf_node * poly, int idx);

dxf_node * dxf_new_circle (double x0, double y0, double z0,
double r, int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_text (double x0, double y0, double z0, double h,
char *txt, int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_mtext (double x0, double y0, double z0, double h,
char *txt[], int num_txt, int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_attdef (double x0, double y0, double z0, double h,
char *txt, char *tag, int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_attrib (double x0, double y0, double z0, double h,
char *txt, char *tag, int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_attdef_cpy (dxf_node *text, char *tag, double x0, double y0, double z0, int hide, int pool);

dxf_node * dxf_attrib_cpy (dxf_node *attdef, double x0, double y0, double z0, 
double scale, double rotation, int pool);

dxf_node * dxf_new_seqend (char *layer, int pool);

dxf_node * dxf_new_endblk (char *layer, char *owner, int pool);

dxf_node * dxf_new_begblk (char *name, char *layer, char *owner, int pool);

dxf_node * dxf_new_blkrec (char *name, int pool);

int dxf_block_append(dxf_node *blk, dxf_node *obj);

int dxf_new_block(dxf_drawing *drawing, char *name, char *descr,
	double x, double y, double z,
	int txt2attr, char *mark, char *hide_mark, 
	char *layer, list_node *list,
	struct do_list *list_do, int pool);

dxf_node * dxf_new_insert (char *name, double x0, double y0, double z0,
int color, char *layer, char *ltype, int lw, int paper, int pool);

int dxf_insert_append(dxf_drawing *drawing, dxf_node *ins, dxf_node *obj, int pool);

int dxf_new_layer (dxf_drawing *drawing, char *name, int color, char *ltype);

int dxf_new_ltype (dxf_drawing *drawing, dxf_ltype *line_type);

int dxf_new_tstyle (dxf_drawing *drawing, char *name);

int dxf_new_tstyle_shp (dxf_drawing *drawing, char *name);

dxf_node * dxf_new_hatch (struct h_pattern *pattern, graph_obj *bound,
int solid, int assoc,
int style, /* 0 = normal odd, 1 = outer, 2 = ignore */
int type, /* 0 = user, 1 = predefined, 2 =custom */
double rot, double scale,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_hatch2 (struct h_pattern *pattern, list_node *bound_list,
int solid, int assoc,
int style, /* 0 = normal odd, 1 = outer, 2 = ignore */
int type, /* 0 = user, 1 = predefined, 2 =custom */
double rot, double scale,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_spline (dxf_node *poly, int degree, int closed,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_spline2 (dxf_node *poly, int closed,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_arc (double x0, double y0, double z0,
double r, double start, double end,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_ellipse (double x0, double y0, double z0,
double x1, double y1, double z1,
double r, double start, double end,
int color, char *layer, char *ltype, int lw, int paper, int pool);

dxf_node * dxf_new_imgdef (char *path, int pool);

dxf_node * dxf_new_image (dxf_drawing *drawing,
double x0, double y0, double z0,
double u[3], double v[3], double w, double h,
char *path,
int color, char *layer, char *ltype, int lw, int paper, int pool);

#endif