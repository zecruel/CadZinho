#ifndef _TTFONT_LIB
#define _TTFONT_LIB

#include "stb_truetype.h"

struct tt_glyph{
	int cp; /* unicode code point */
	int g_idx; /*glyph index */
	
	int num_verts;
	stbtt_vertex *vertices;
	
	double adv;
	
	struct tt_glyph *next;
};

struct tt_font{
	char *name;
	stbtt_fontinfo *info;
	double scale, ascent, descent, line_gap;
	
	struct tt_glyph *list, *end;
};

struct tt_glyph * tt_find_cp (struct tt_font * font, int code_point);

struct tt_glyph * tt_get_glyph (struct tt_font * font, int code_point);

struct tt_font * tt_init (char *path);

void tt_font_free(struct tt_font * font);

int tt_parse_str(struct tt_font * font, list_node *list_ret, int pool_idx, char *txt, double *w, int force_ascii);

int tt_w_str(struct tt_font * font, char *txt, double *w, int force_ascii);

graph_obj * tt_parse_cp(struct tt_font * font, int cp, int prev_cp, int pool_idx, double *w);

#endif