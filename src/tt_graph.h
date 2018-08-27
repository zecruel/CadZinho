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
	double scale;
	
	struct tt_glyph *list, *end;
};

struct tt_font * tt_init (char *path);

void tt_font_free(struct tt_font * font);

int tt_parse_str(struct tt_font * font, list_node *list_ret, int pool_idx, char *txt);

#endif