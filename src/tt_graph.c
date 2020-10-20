#include <stdio.h>
#include "dxf.h"
#include "graph.h"
#include <math.h>
#include "tt_graph.h"

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

/* number of segments for the curve */
#define N_SEG 5

extern int cp1252[];

graph_obj * tt_parse_v(stbtt_vertex *vertices, int num_verts, double scale, int pool_idx, double *curr_pos){
/* convert vertices information from Stb_truetype to graph object*/
	int i, j;
	graph_obj *line_list = NULL;
	
	if (vertices){
		double pre_x = 0;
		double pre_y = 0;
		double px = 0;
		double py = 0;
		double cx = 0;
		double cy = 0;
		double t, a, b, c;
		
		line_list = graph_new(pool_idx);
		if(line_list) line_list->fill = 1;
		for (i=0; i < num_verts; i++){ /*sweep the vertex list*/
			px = (double) vertices[i].x * scale;
			py = (double) vertices[i].y * scale;
			
			if (vertices[i].type == 3){ /*quadratic bezier curve */
				for (j = 1; j <= N_SEG; j++){
					t = (double)j / (double)N_SEG;
					a = pow((1.0 - t), 2.0);
					b = 2.0 * t * (1.0 - t);
					c = pow(t, 2.0);
					
					px = (a * (double) vertices[i-1].x + b * (double) vertices[i].cx + c * (double) vertices[i].x) * scale;
					py = (a * (double) vertices[i-1].y + b * (double) vertices[i].cy + c * (double) vertices[i].y) * scale;
					
					line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
					
					pre_x = px;
					pre_y = py;
				}
			}
			else if (vertices[i].type > 1){ /* single segment */
				
				line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
			}
			
			pre_x = px;
			pre_y = py;
		}
		//*curr_pos = px;
	}
	return line_list;
}

int tt_load_font (char *path, stbtt_fontinfo *font, double *scale, double *asc, double *desc, double *lgap){
/* Load a truetype type font from path. Return the font metrics to unit size.*/
	int ok = 0;
	char *ttf_buffer = NULL;
	long fsize;
	
	ttf_buffer = load_file(path, &fsize); /* load file in buffer */
	if (ttf_buffer){
		/* init font */
		ok = stbtt_InitFont(font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
		if (ok) { /* get and calcule font metrics for unit size*/
			int ascent, descent, lineGap;
			stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
			*scale = 1.0/(double) (ascent + descent + lineGap);
			*asc = *scale * ascent;
			*desc = *scale * descent;
			*lgap = *scale * lineGap;
		}
	}
	
	return ok;
}

struct tt_glyph * tt_get_glyph (struct tt_font * font, int code_point){
/* Init and parse a glyph form a code point (UTF). */
	struct tt_glyph *main_str = NULL;
	if (font){
		main_str = malloc(sizeof(struct tt_glyph));
		if(!main_str) return NULL;
		
		main_str->cp = code_point;
		main_str->num_verts = 0;
		main_str->vertices = NULL;
		main_str->next = NULL;
		
		/* get the glyph index for speed */
		int g_idx = stbtt_FindGlyphIndex(font->info, code_point);
		main_str->g_idx = g_idx;
		
		//if (g_idx){
			/* get the graphics information*/
			main_str->num_verts = stbtt_GetGlyphShape(font->info, g_idx, &(main_str->vertices));
			int adv;
			/* get glyph advance */
			stbtt_GetGlyphHMetrics(font->info, g_idx, &adv, NULL);
			main_str->adv = (double) adv * font->scale;
		//}
	
	}
	return main_str;
}

struct tt_font * tt_init (char *path){
/*Init the font structure from path. For speed, the most useful glyphs are parsed too*/
	
	struct tt_font *main_str = NULL;
	if (path){
		/* alloc the structures*/
		main_str = malloc(sizeof(struct tt_font));
		if(!main_str) return NULL;
		
		main_str->info = malloc(sizeof(stbtt_fontinfo));
		if(!main_str->info){
			free(main_str);
			return NULL;
		}
		/* load font info*/
		if(!tt_load_font(path, main_str->info, &(main_str->scale), &(main_str->ascent), &(main_str->descent), &(main_str->line_gap))){
			free(main_str->info);
			free(main_str);
			return NULL;
		}
		
		/* For speed, parse the most useful glyphs (in english) and add to list */
		if(main_str->list = tt_get_glyph (main_str, 0)){
			main_str->end = main_str->list;
			int i;
			struct tt_glyph *curr_glyph = NULL, *prev_glyph = main_str->list;
			
			for (i = 32; i < 127; i++){
				curr_glyph = tt_get_glyph (main_str, i);
				if(curr_glyph){
					prev_glyph->next = curr_glyph;
					prev_glyph = curr_glyph;
					main_str->end = curr_glyph;
				}
				else break;
			}
			/* For speed, parse the most useful glyphs (latin) and add to list */
			for (i = 160; i < 256; i++){
				curr_glyph = tt_get_glyph (main_str, i);
				if(curr_glyph){
					prev_glyph->next = curr_glyph;
					prev_glyph = curr_glyph;
					main_str->end = curr_glyph;
				}
				else break;
			}
			/* For speed, parse the most useful glyphs (greek) and add to list */
			/*for (i = 913; i < 970; i++){
				curr_glyph = tt_get_glyph (main_str, i);
				if(curr_glyph){
					prev_glyph->next = curr_glyph;
					prev_glyph = curr_glyph;
					main_str->end = curr_glyph;
				}
				else break;
			}*/
		}
	}
	return main_str;
}

void tt_font_free(struct tt_font * font){
/* free memory of font*/
	if(font){
		struct tt_glyph *curr_glyph = NULL, *next_glyph = NULL;
		curr_glyph = font->list;
		/*first, free all glyphs*/
		while (curr_glyph){
			next_glyph = curr_glyph->next;
			stbtt_FreeShape((const stbtt_fontinfo *)font, curr_glyph->vertices);
			free(curr_glyph);
			curr_glyph = next_glyph;
		}
		/* free the font info*/
		if(font->info){
			free(font->info->data);
			free(font->info);
		}
		/* last, free the main struct */
		free(font);
	}
}

struct tt_glyph * tt_find_cp (struct tt_font * font, int code_point){
/* try to find the glyph previously loaded by its code point (UTF) */
	if (!font) return NULL;
	
	struct tt_glyph *curr_glyph = NULL;
	if (font->list) curr_glyph = font->list->next;
	while (curr_glyph){ /* sweep the list*/
		if (curr_glyph->cp == code_point) return curr_glyph; /* glyph found*/
		
		curr_glyph = curr_glyph->next;
	}
	
	return NULL; /* fail the search*/
}

int tt_parse_str(struct tt_font * font, list_node *list_ret, int pool_idx, char *txt, double *w, int force_ascii){
/* parse full string to graph*/
	if (!font || !list_ret || !txt) return 0;
	
	int ofs = 0, str_start = 0, code_p, num_graph = 0;
	struct tt_glyph *curr_glyph = NULL, *prev_glyph = NULL;
	graph_obj *curr_graph = NULL;
	
	double ofs_x = 0.0;
	
	/*sweep the string, decoding utf8 or ascii (cp1252)*/
	if (force_ascii){ /* decode ascii cp1252 */
		code_p = (int) cp1252[(unsigned char)txt[str_start]];
		if (code_p > 0) ofs = 1;
		else ofs = 0;
	}
	else{ /* decode utf-8 */
		ofs = utf8_to_codepoint(txt + str_start, &code_p);
	}
	while (ofs){
		
		/* try to find the glyph previously loaded */
		curr_glyph = tt_find_cp (font, code_p);
		
		if (!curr_glyph){/* if not found, add the glyph in list*/
			curr_glyph = tt_get_glyph (font, code_p);
			if(curr_glyph){
				if (font->end) font->end->next = curr_glyph;
				font->end = curr_glyph;
			}
		}
		
		if (curr_glyph){ /* glyph found*/
			
			if (prev_glyph){/* get additional custom advance, relative to previous glyph*/
				ofs_x += stbtt_GetGlyphKernAdvance(font->info,
				prev_glyph->g_idx, curr_glyph->g_idx) * font->scale;
			}
			/* get final graphics of each glyph*/
			curr_graph = tt_parse_v(curr_glyph->vertices, curr_glyph->num_verts, font->scale, pool_idx, NULL);
			if (curr_graph){
				graph_modify(curr_graph, ofs_x, 0.0, 1.0, 1.0, 0.0);
				/* store the graph in the return vector */
				if (list_push(list_ret, list_new((void *)curr_graph, pool_idx))) num_graph++;
			}
			/* update the ofset */
			ofs_x += curr_glyph->adv;
			prev_glyph = curr_glyph;
		}
		/* get next codepoint */
		str_start += ofs;
		if (force_ascii){ /* decode ascii cp1252 */
			code_p = (int) cp1252[(unsigned char)txt[str_start]];
			if (code_p > 0) ofs = 1;
			else ofs = 0;
		}
		else{/* decode utf-8 */
			ofs = utf8_to_codepoint(txt + str_start, &code_p);
		}
	}
	if (w != NULL) *w = ofs_x; /* return the text width*/
	return num_graph;
}

int tt_w_str(struct tt_font * font, char *txt, double *w, int force_ascii){
/* get string width*/
	if (!font || !txt) return 0;
	
	int ofs = 0, str_start = 0, code_p, num_graph = 0;
	struct tt_glyph *curr_glyph = NULL, *prev_glyph = NULL;
	
	
	double ofs_x = 0.0;
	
	/*sweep the string, decoding utf8 or ascii (cp1252)*/
	if (force_ascii){ /* decode ascii cp1252 */
		code_p = (int) cp1252[(unsigned char)txt[str_start]];
		if (code_p > 0) ofs = 1;
		else ofs = 0;
	}
	else{ /* decode utf-8 */
		ofs = utf8_to_codepoint(txt + str_start, &code_p);
	}
	while (ofs){
		
		/* try to find the glyph previously loaded */
		curr_glyph = tt_find_cp (font, code_p);
		
		if (!curr_glyph){/* if not found, add the glyph in list*/
			curr_glyph = tt_get_glyph (font, code_p);
			if(curr_glyph){
				if (font->end) font->end->next = curr_glyph;
				font->end = curr_glyph;
			}
		}
		
		if (curr_glyph){ /* glyph found*/
			
			//if (prev_glyph){/* get additional custom advance, relative to previous glyph*/
			//	ofs_x += stbtt_GetGlyphKernAdvance(font->info,
			//	prev_glyph->g_idx, curr_glyph->g_idx) * font->scale;
			//}
			/* update the ofset */
			ofs_x += curr_glyph->adv;
			prev_glyph = curr_glyph;
		}
		/* get next codepoint */
		str_start += ofs;
		if (force_ascii){ /* decode ascii cp1252 */
			code_p = (int) cp1252[(unsigned char)txt[str_start]];
			if (code_p > 0) ofs = 1;
			else ofs = 0;
		}
		else{/* decode utf-8 */
			ofs = utf8_to_codepoint(txt + str_start, &code_p);
		}
	}
	if (w != NULL) *w = ofs_x; /* return the text width*/
	return 1;
}

graph_obj * tt_parse_cp(struct tt_font * font, int cp, int prev_cp, int pool_idx, double *w){
/* parse single code point to graph*/
	if (!font) return 0;
	
	struct tt_glyph *curr_glyph = NULL, *prev_glyph = NULL;
	graph_obj *curr_graph = NULL;
	
	double ofs_x = 0.0;
	
	/* try to find the glyph previously loaded */
	curr_glyph = tt_find_cp (font, cp);
	prev_glyph = tt_find_cp (font, prev_cp);
	
	if (!curr_glyph){/* if not found, add the glyph in list*/
		curr_glyph = tt_get_glyph (font, cp);
		if(curr_glyph){
			if (font->end) font->end->next = curr_glyph;
			font->end = curr_glyph;
		}
	}
	
	if (curr_glyph){ /* glyph found*/
		
		if (prev_glyph){/* get additional custom advance, relative to previous glyph*/
			ofs_x += stbtt_GetGlyphKernAdvance(font->info,
			prev_glyph->g_idx, curr_glyph->g_idx) * font->scale;
		}
		/* get final graphics of  glyph*/
		curr_graph = tt_parse_v(curr_glyph->vertices, curr_glyph->num_verts, font->scale, pool_idx, NULL);
		
		/* update the ofset */
		ofs_x += curr_glyph->adv;
	}
	
	if (w != NULL) *w = ofs_x; /* return the text width*/
	return curr_graph;
}