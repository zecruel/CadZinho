#ifndef _DXF_LIB
#define _DXF_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "list.h"
#include "util.h"

#define DXF_MAX_LAYERS 1000
#define DXF_MAX_LTYPES 1000
#define DXF_MAX_FONTS 1000
#define DXF_MAX_CHARS 250
#define DXF_MAX_PAT 12
#define DXF_POOL_PAGES 1000

#define CZ_VERSION "0.2.0 - 2022"

struct sort_by_idx{
	long int idx;
	void *data;
};

/* supportable graphic entities */
enum dxf_graph {
	DXF_NONE 		= 0x0,
	DXF_LINE 		= 0x1,
	DXF_POINT 		= 0x2,
	DXF_CIRCLE 		= 0x4,
	DXF_ARC 			= 0x8,
	DXF_TRACE 		= 0x10,
	DXF_SOLID 		= 0x20,
	DXF_TEXT 		= 0x40,
	DXF_SHAPE 		= 0x80,
	DXF_INSERT 		= 0x100,
	DXF_ATTRIB 		= 0x200,
	DXF_POLYLINE 		= 0x400,
	DXF_VERTEX 		= 0x800,
	DXF_LWPOLYLINE 	= 0x1000,
	DXF_3DFACE 		= 0x2000,
	DXF_VIEWPORT 	= 0x4000,
	DXF_DIMENSION 	= 0x8000,
	DXF_ELLIPSE 		= 0x10000,
	DXF_MTEXT 		= 0x20000,
	DXF_BLK 			= 0x40000,
	DXF_ENDBLK 		= 0x80000,
	DXF_HATCH 		= 0x100000,
	DXF_DIMSTYLE 	= 0x200000,
	DXF_IMAGE 		= 0x400000,
	DXF_IMAGE_DEF 	= 0x800000,
	DXF_SPLINE 		= 0x1000000,
};

enum dxf_pool_action{
	ADD_DXF,
	ZERO_DXF,
	FREE_DXF
};

enum dxf_pool_life{
	DWG_LIFE = 0,
	FRAME_LIFE = 1,
	ONE_TIME = 2,
	PRG_LIFE = 3,
	SEL_LIFE = 4
};

enum dxf_ltyp_typ {
	LTYP_SIMPLE,
	LTYP_SHAPE,
	LTYP_STRING
};

struct Dxf_pool_slot{
	void *pool[DXF_POOL_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};
typedef struct Dxf_pool_slot dxf_pool_slot;

struct Dxf_node{
	struct Dxf_node *master; /* entity to which it is contained */
	struct Dxf_node *next;    /* next node (double linked list) */
	struct Dxf_node *prev;    /* previous node (double linked list) */
	struct Dxf_node *end; /*last object in list*/
	
	/* defines whether it is an DXF entity (obj) or an attribute (value) */
	enum {DXF_ENT, DXF_ATTR} type;
	
	union{
		struct {
			/* == entity dxf especific */
			char name[DXF_MAX_CHARS+1]; /* standardized DXF name of entity */
			int layer;
			int pool;
			void * graphics; /* graphics information */
			
			struct Dxf_node *content; /* the content is a list */
		} obj;
		
		struct {
			/* ==group dxf especific */
			int group; /* standardized DXF group */
			/* the group defines the type of data, which can be: */
			enum {DXF_FLOAT, DXF_INT, DXF_STR} t_data;
			union {
				double d_data; /* a float number, */
				int i_data; /* a integer number, */
				char s_data[DXF_MAX_CHARS+1]; /* or a string. */
			};
		} value;
	};
}; 
typedef struct Dxf_node dxf_node;

struct Dxf_layer{
	char name[DXF_MAX_CHARS+1];
	int color;
	char ltype[DXF_MAX_CHARS+1];
	int line_w;
	int frozen;
	int lock;
	int off;
	int num_el;
	dxf_node *obj;
};
typedef struct Dxf_layer dxf_layer;


struct Dxf_ltyp_pat {
	double dash;
	enum dxf_ltyp_typ type;
	char sty[30];
	int sty_i;
	int abs_rot;
	double rot;
	double scale;
	double ofs_x;
	double ofs_y;
	union {
		long num;
		char str[30];
	};
};
typedef struct Dxf_ltyp_pat dxf_ltyp_pat;

struct Dxf_ltype{
	char name[DXF_MAX_CHARS+1];
	char descr[DXF_MAX_CHARS+1];
	int size;
	//double pat[DXF_MAX_PAT];
	dxf_ltyp_pat dashes[DXF_MAX_PAT];
	double length;
	int num_el;
	dxf_node *obj;
};
typedef struct Dxf_ltype dxf_ltype;

struct Dxf_tstyle{
	char name[DXF_MAX_CHARS+1];
	char file[DXF_MAX_CHARS+1];
	char big_file[DXF_MAX_CHARS+1];
	char subst_file[DXF_MAX_CHARS+1];
	
	int flags1;
	int flags2;
	int num_el;
	
	double fixed_h;
	double width_f;
	double oblique;
	
	void *font;
	
	dxf_node *obj;
};
typedef struct Dxf_tstyle dxf_tstyle;

struct Dxf_dimsty{
	char name[DXF_MAX_CHARS+1];
	char post[DXF_MAX_CHARS+1]; /* custom text (sufix/prefix) for annotation */
	char a_type[DXF_MAX_CHARS+1]; /* arrow type */
	
	double scale; /* global scale for render */
	double a_size; /* arrow size */
	double ext_ofs; /* extension line offset (gap) between measure point */
	double ext_e; /*extension of extention line :D */
	double txt_size; /* annotation text size */
	double an_scale; /* annotation scale - apply to measure */
	double gap; /* space between text and base line */
	
	int dec; /* number of decimal places */
	int num_el;
	
	int tstyle; /* text style (index) */
	dxf_node *obj;
};
typedef struct Dxf_dimsty dxf_dimsty;

struct Dxf_drawing{
	int pool; /* memory pool location */
	/* DXF main sections */
	dxf_node 	
	*head,  /* header with drawing variables */
	*tabs,  /* tables - see next */
	*blks, /* blocks definitions */
	*ents, /* entities - grahics elements */
	*objs; /* objects - non grahics elements */
	
	/* DXF tables */
	dxf_node 
	*t_ltype, /* line types */ 
	*t_layer,  /* layers */
	*t_style,  /* text styles, text fonts */
	*t_view,  /* views */
	*t_ucs,   /* UCS - coordinate systems */
	*t_vport,  /* viewports (useful in layout mode) */
	*t_dimst, /* dimension styles*/
	*blks_rec, /* blocks records */
	*t_appid; /* third part application indentifier */
	
	/* complete structure access */
	dxf_node *main_struct;
	
	dxf_node *hand_seed; /* handle generator */
	
	dxf_layer layers[DXF_MAX_LAYERS];
	int num_layers;
	
	dxf_ltype ltypes[DXF_MAX_LTYPES];
	int num_ltypes;
	double ltscale;
	double celtscale;
	
	dxf_tstyle text_styles[DXF_MAX_FONTS];
	int num_tstyles;
	
	void *font_list;
	void *dflt_font;
	char *dflt_fonts_path;
	
	void *img_list;
	void *xref_list;
	
	int version;
	
	double dimlfac;
	double dimscale;
	char dimpost [DXF_MAX_CHARS + 1];
	char dimtxsty [DXF_MAX_CHARS + 1];
	char dimblk [DXF_MAX_CHARS + 1];
	int dimdec;
	
};
typedef struct Dxf_drawing dxf_drawing;

struct dxf_xref {
	char path [DXF_MAX_CHARS + 1];
	dxf_drawing *drwg;
};

/* functions*/

void dxf_ent_print2 (dxf_node *ent);

void dxf_ent_print_f (dxf_node *ent, char *path);

dxf_node * dxf_obj_new (char *name, int pool);

int dxf_ident_attr_type (int group);

int dxf_ident_ent_type (dxf_node *obj);

dxf_node * dxf_attr_new (int group, int type, void *value, int pool);

//vector_p dxf_find_attr(dxf_node * obj, int attr);

dxf_node * dxf_find_attr2(dxf_node * obj, int attr);

dxf_node * dxf_find_attr_i(dxf_node * obj, int attr, int idx);

dxf_node * dxf_find_attr_i2(dxf_node * start, dxf_node * end, int attr, int idx);

dxf_node * dxf_find_attr_nxt(dxf_node * obj, dxf_node ** next, int attr);

dxf_node * dxf_find_obj_nxt(dxf_node * obj, dxf_node ** next, char *name);

int dxf_count_attr(dxf_node * obj, int attr);

//vector_p dxf_find_obj(dxf_node * obj, char *name);

int dxf_count_obj(dxf_node * obj, char *name);

dxf_node * dxf_find_obj_i(dxf_node * obj, char *name, int idx);

//vector_p dxf_find_obj_descr(dxf_node * obj, char *name, char *descr);

dxf_node * dxf_find_obj_descr2(dxf_node * obj, char *name, char *descr);

void dxf_layer_assemb (dxf_drawing *drawing);

void dxf_ltype_assemb (dxf_drawing *drawing);

void dxf_tstyles_assemb (dxf_drawing *drawing);

int dxf_lay_idx (dxf_drawing *drawing, char *name);

int dxf_ltype_idx (dxf_drawing *drawing, char *name);

int dxf_tstyle_idx (dxf_drawing *drawing, char *name);

int dxf_save (char *path, dxf_drawing *drawing);

int dxf_read (dxf_drawing *drawing, char *buf, long fsize, int *prog);

dxf_node * dxf_find_obj2(dxf_node * obj, char *name);

void * dxf_mem_pool(enum dxf_pool_action action, int idx);

void dxf_append(dxf_node *master, dxf_node *new_node);

int dxf_find_head_var(dxf_node *obj, char *var, dxf_node **start, dxf_node **end);

dxf_drawing *dxf_drawing_new(int pool);

int dxf_drawing_clear (dxf_drawing *drawing);

int dxf_obj_subst(dxf_node *orig, dxf_node *repl);

int dxf_obj_append(dxf_node *master, dxf_node *obj);

int dxf_obj_detach(dxf_node *obj);

int dxf_attr_append(dxf_node *master, int group, void *value, int pool);

int dxf_attr_insert_before(dxf_node *attr, int group, void *value, int pool);

dxf_node * dxf_attr_insert_after(dxf_node *attr, int group, void *value, int pool);

int dxf_attr_change(dxf_node *master, int group, void *value);

int dxf_attr_change_i(dxf_node *master, int group, void *value, int idx);

int dxf_find_ext_appid(dxf_node *obj, char *appid, dxf_node **start, dxf_node **end);

int dxf_ext_append(dxf_node *master, char *appid, int group, void *value, int pool);

int ent_handle(dxf_drawing *drawing, dxf_node *element);

dxf_node *dxf_find_handle(dxf_node *source, long int handle);

void drawing_ent_append(dxf_drawing *drawing, dxf_node *element);

list_node * dxf_ents_list(dxf_drawing *drawing, int pool_idx);

dxf_drawing * dxf_xref_list(dxf_drawing *drawing, char *xref_path);

int dxf_xref_clear_list(dxf_drawing *drawing);

void dxf_xref_assemb (dxf_drawing *drawing);

int dxf_find_last_blk (dxf_drawing *drawing, char mark[3]);

#endif