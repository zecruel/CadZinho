#include <stdio.h>
#include "dxf.h"
#include "graph.h"
#include <math.h>
#include "tt_graph.h"

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

/* number of segments for the curve */
#define N_SEG 5

int utf8_to_codepoint(char *utf8_s, int *uni_c){
	int ofs = 0;
	
	if (!utf8_s || !uni_c) return 0;
	
	char c = utf8_s[ofs];
	if (!c) return 0; /*string end*/
	
	if ( (c & 0x80) == 0 ){
		*uni_c = c;
		ofs++;
	}
	else if ( (c & 0xE0) == 0xC0 ){
		if (!utf8_s[ofs+1]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0x1F) << 6;
		*uni_c |= (utf8_s[ofs+1] & 0x3F);
		ofs += 2;
	}
	else if ( (c & 0xF0) == 0xE0 ){
		if (!utf8_s[ofs+1] || !utf8_s[ofs+2]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0xF) << 12;
		*uni_c |= (utf8_s[ofs+1] & 0x3F) << 6;
		*uni_c |= (utf8_s[ofs+2] & 0x3F);
		ofs += 3;
	}
	else if ( (c & 0xF8) == 0xF0 ){
		if (!utf8_s[ofs+1] || !utf8_s[ofs+2] || !utf8_s[ofs+3]) return 0; /* error -> string end*/
		*uni_c = (utf8_s[ofs] & 0x7) << 18;
		*uni_c |= (utf8_s[ofs+1] & 0x3F) << 12;
		*uni_c |= (utf8_s[ofs+2] & 0x3F) << 6;
		*uni_c |= (utf8_s[ofs+3] & 0x3F);
		ofs += 4;
	}

	return ofs;

}

int codepoint_to_utf8(int uni_c, char utf8_s[5]){
	
	if (!utf8_s) return 0;
	
	int len = 0;
	utf8_s[4] = 0;
	if ( 0 <= uni_c && uni_c <= 0x7f ){
		utf8_s[0] = (char)uni_c;
		len++;
	}
	else if ( 0x80 <= uni_c && uni_c <= 0x7ff ){
		utf8_s[0] = ( 0xc0 | (uni_c >> 6) );
		utf8_s[1] = ( 0x80 | (uni_c & 0x3f) );
		len += 2;
	}
	else if ( 0x800 <= uni_c && uni_c <= 0xffff ){
		utf8_s[0] = ( 0xe0 | (uni_c >> 12) );
		utf8_s[1] = ( 0x80 | ((uni_c >> 6) & 0x3f) );
		utf8_s[2] = ( 0x80 | (uni_c & 0x3f) );
		len += 3;
	}
	else if ( 0x10000 <= uni_c && uni_c <= 0x1fffff ){
		utf8_s[0] = ( 0xf0 | (uni_c >> 18) );
		utf8_s[1] = ( 0x80 | ((uni_c >> 12) & 0x3f) );
		utf8_s[2] = ( 0x80 | ((uni_c >> 6) & 0x3f) );
		utf8_s[3] = ( 0x80 | (uni_c & 0x3f) );
		len += 4;
	}
	
	return len;
}

graph_obj * tt_parse_v(stbtt_vertex *vertices, int num_verts, double scale, int pool_idx, double *curr_pos){
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
		for (i=0; i < num_verts; i++){
			//printf ("vert %d = %d,%d\n", vertices[i].type, vertices[i].x, vertices[i].y);
			px = vertices[i].x * scale;
			py = vertices[i].y * scale;
			if (vertices[i].type == 3){ /*quadratic bezier curve */
				for (j = 1; j <= N_SEG; j++){
					t = (double)j / (double)N_SEG;
					a = pow((1.0 - t), 2.0);
					b = 2.0 * t * (1.0 - t);
					c = pow(t, 2.0);
					
					px = (a * vertices[i-1].x + b * vertices[i].cx + c * vertices[i].x) * scale;
					py = (a * vertices[i-1].y + b * vertices[i].cy + c * vertices[i].y) * scale;
					
					line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
					
					pre_x = px;
					pre_y = py;
				}
			}
			else if (vertices[i].type>1){
				line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
			}
			
			pre_x = px;
			pre_y = py;
		}
		//*curr_pos = px;
	}
	return line_list;
}

int tt_load_font (char *path, stbtt_fontinfo *font, double *scale){
	int ok = 0;
	char *ttf_buffer = NULL;
	long fsize;
	
	ttf_buffer = dxf_load_file(path, &fsize);
	if (ttf_buffer){
		ok = stbtt_InitFont(font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
		*scale = stbtt_ScaleForPixelHeight(font, 1);
	}
	
	return ok;
}

struct tt_glyph * tt_get_glyph (struct tt_font * font, int code_point){
	struct tt_glyph *main_str = NULL;
	if (font){
		main_str = malloc(sizeof(struct tt_glyph));
		if(!main_str) return NULL;
		
		main_str->cp = code_point;
		main_str->num_verts = 0;
		main_str->vertices = NULL;
		main_str->next = NULL;
		
		int g_idx = stbtt_FindGlyphIndex(font->info, code_point);
		main_str->g_idx = g_idx;
		
		//if (g_idx){
			main_str->num_verts = stbtt_GetGlyphShape(font->info, g_idx, &(main_str->vertices));
			int adv;
			stbtt_GetGlyphHMetrics(font->info, g_idx, &adv, NULL);
			main_str->adv = adv * font->scale;
		//}
	
	}
	return main_str;
}

struct tt_font * tt_init (char *path){
	struct tt_font *main_str = NULL;
	if (path){
		main_str = malloc(sizeof(struct tt_font));
		if(!main_str) return NULL;
		
		main_str->info = malloc(sizeof(stbtt_fontinfo));
		if(!main_str->info){
			free(main_str);
			return NULL;
		}
		
		if(!tt_load_font(path, main_str->info, &(main_str->scale))){
			free(main_str->info);
			free(main_str);
			return NULL;
		}
		
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
			for (i = 160; i < 256; i++){
				curr_glyph = tt_get_glyph (main_str, i);
				if(curr_glyph){
					prev_glyph->next = curr_glyph;
					prev_glyph = curr_glyph;
					main_str->end = curr_glyph;
				}
				else break;
			}
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
	if(font){
		struct tt_glyph *curr_glyph = NULL, *next_glyph = NULL;
		curr_glyph = font->list;
		while (curr_glyph){
			next_glyph = curr_glyph->next;
			stbtt_FreeShape((const stbtt_fontinfo *)font, curr_glyph->vertices);
			free(curr_glyph);
			curr_glyph = next_glyph;
		}
		
		if(font->info){
			free(font->info->data);
			free(font->info);
		}
		free(font);
	}
}

struct tt_glyph * tt_find_cp (struct tt_font * font, int code_point){
	if (!font) return NULL;
	
	struct tt_glyph *curr_glyph = NULL;
	if (font->list) curr_glyph = font->list->next;
	while (curr_glyph){
		if (curr_glyph->cp == code_point) return curr_glyph;
		
		curr_glyph = curr_glyph->next;
	}
	
	return NULL;
}

int tt_parse_str(struct tt_font * font, list_node *list_ret, int pool_idx, char *txt){
	if (!font || !list_ret || !txt) return 0;
	
	int ofs = 0, str_start = 0, code_p;
	struct tt_glyph *curr_glyph = NULL, *prev_glyph = NULL;
	graph_obj *curr_graph = NULL;
	
	double ofs_x = 0.0;
	
	while (ofs = utf8_to_codepoint(txt + str_start, &code_p)){
		str_start += ofs;
		/* try to find the glyph previously loaded */
		curr_glyph = tt_find_cp (font, code_p);
		if (!curr_glyph){/* if not found, add the glyph in list*/
			curr_glyph = tt_get_glyph (font, code_p);
			if(curr_glyph){
				if (font->end) font->end->next = curr_glyph;
				font->end = curr_glyph;
			}
		}
		if (curr_glyph){
			if (prev_glyph){
				ofs += stbtt_GetGlyphKernAdvance(font->info,
				prev_glyph->g_idx, curr_glyph->g_idx) * font->scale;
			}
			curr_graph = tt_parse_v(curr_glyph->vertices, curr_glyph->num_verts, font->scale, pool_idx, NULL);
			if (curr_graph){
				graph_modify(curr_graph, ofs_x, 0.0, 1.0, 1.0, 0.0);
				/* store the graph in the return vector */
				list_push(list_ret, list_new((void *)curr_graph, pool_idx));
			}
			ofs_x += curr_glyph->adv;
			prev_glyph = curr_glyph;
		}
	}
	return 1;
}

int tt_parse4(list_node *list_ret, int pool_idx, const char *txt){
	int ok = 0, i = 0;
	
	struct tt_font *font;
	
	//if(tt_load_font("C:/Windows/Fonts/arialbd.ttf", &font, &scale)){
	//if(tt_load_font("Lato-Light.ttf", &font, &scale)){
	if (font = tt_init ("C:/Windows/Fonts/arial.ttf")){
		
		ok = tt_parse_str(font, list_ret, pool_idx, txt);
	}
	tt_font_free(font);
	return ok;
}

int tt_parse3(list_node *list_ret, int pool_idx, const char *txt){
	int ok = 0, i = 0;
	struct tt_glyph *curr_glyph = NULL;
	graph_obj *curr = NULL;
	struct tt_font *font;
	
	//if(tt_load_font("C:/Windows/Fonts/arialbd.ttf", &font, &scale)){
	//if(tt_load_font("Lato-Light.ttf", &font, &scale)){
	if (font = tt_init ("Lato-Light.ttf")){
		double ofs_x = 0.0, ofs_y = 0.0, curr_pos, new_ofs_x = 0.0;
		if (font->list) curr_glyph = font->list->next;
		while (curr_glyph){
			
			curr = tt_parse_v(curr_glyph->vertices, curr_glyph->num_verts, font->scale, pool_idx, &curr_pos);
			
			if (curr){
				
				graph_modify(curr, ofs_x, ofs_y, 1.0, 1.0, 0.0);
				/* store the graph in the return vector */
				list_push(list_ret, list_new((void *)curr, pool_idx));
				
				ofs_x += curr_glyph->adv;
				i++;
				if (i >= 20){
					i=0;
					ofs_x = 0;
					ofs_y -=1;
				}
				
			}
			
			
			curr_glyph = curr_glyph->next;
		}
		ok = 1;
	}
	tt_font_free(font);
	return ok;
}

int tt_parse2(list_node *list_ret, int pool_idx, const char *txt){
	int ok = 0;
	stbtt_fontinfo font;
	double scale;
	graph_obj *curr = NULL;
	
	//if(tt_load_font("C:/Windows/Fonts/arialbd.ttf", &font, &scale)){
	if(tt_load_font("Lato-Light.ttf", &font, &scale)){
		int i, str_len, num_verts = 0;
		wchar_t str_uni[255];
		double ofs_x = 0.0, ofs_y = 0.0, curr_pos, new_ofs_x = 0.0;
		stbtt_vertex *vertices = NULL;
		int prev_glyph, glyph;
	
		//converte o texto em uma string unicode
		str_len = mbstowcs(str_uni, txt, 255);
		
		int adv = 0, ls = 0, kadv = 0;
	
		for(i = 0; i < str_len; i++){
			
			glyph = stbtt_FindGlyphIndex(&font, (int)str_uni[i]);
			
			if ((str_uni[i] != 9) && (str_uni[i] != 32)){
				num_verts = stbtt_GetGlyphShape(&font, glyph, &vertices);
				
				curr = tt_parse_v(vertices, num_verts, scale, pool_idx, &curr_pos);
				
				stbtt_GetGlyphHMetrics(&font, glyph, &adv, &ls);
				if(i>0) kadv = stbtt_GetGlyphKernAdvance(&font, prev_glyph, glyph);
				
				stbtt_FreeShape(&font, vertices);
				if (curr){
					
					//ofs_x += kadv * scale + 0.05;
					//new_ofs_x = ofs_x + curr->ext_max_x;
					
					ofs_x += kadv * scale;
					new_ofs_x = ofs_x + adv * scale;
					
					graph_modify(curr, ofs_x, 0.0, 1.0, 1.0, 0.0);
					/* store the graph in the return vector */
					list_push(list_ret, list_new((void *)curr, pool_idx));
					
					ofs_x = new_ofs_x;
					
					
				}
			}
			else if ((str_uni[i] == 9)){/* TAB*/
				ofs_x += 1.5;
			}
			else if ((str_uni[i] == 32)){ /* SPACE */
				ofs_x += 0.3;
			}
			
			prev_glyph = glyph;
		}
		ok = 1;
	}
	
	free(font.data);
	
	return ok;
}

graph_obj * tt_parse(int pool_idx, const char *txt, double *w)
{
	char *ttf_buffer;
	long fsize;
	
	stbtt_fontinfo font;
	stbtt_vertex *vertices;
	
	int i, j, str_len, num_vert;
	
	wchar_t str_uni[255];
	double pre_x = 0;
	double pre_y = 0;
	double px = 0;
	double py = 0;
	double max_x = 0;
	double max_y = 0;
	double min_x = 0;
	double min_y = 0;
	
	double ofs_x = 0.0, ofs_y = 0.0;
	
	//converte o texto em uma string unicode
	str_len = mbstowcs(str_uni, txt, 255);
	
	ttf_buffer = dxf_load_file("c:/windows/fonts/arialbd.ttf", &fsize);
	stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
	free(ttf_buffer);
	float scale = stbtt_ScaleForPixelHeight(&font, 20);
	
	//cria a lista de retorno
	graph_obj *line_list = graph_new(pool_idx);
	if(line_list) line_list->fill = 1;
	
	for(i = 0; i < str_len; i++){
		num_vert = stbtt_GetCodepointShape(&font, str_uni[i], &vertices);
		//printf("Num verts = %d\n", num_vert);
		

		for (j=0; j < num_vert; j++){

			//printf ("t=%d (%0.2f,%0.2f)\n", vertices[j].type, vertices[j].x*scale, vertices[j].y*scale);
			px = vertices[j].x * scale + ofs_x;
			py = vertices[j].y * scale + ofs_y;
			
			if (vertices[j].type>1) line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
			
			pre_x = px;
			pre_y = py;
			
			if (j>0){
				max_x = (max_x > pre_x) ? max_x : pre_x;
				max_y = (max_y > pre_y) ? max_y : pre_y;
				min_x = (min_x < pre_x) ? min_x : pre_x;
				min_y = (min_y < pre_y) ? min_y : pre_y;
			}
			else{
				max_x = pre_x;
				max_y = pre_y;
				min_x = pre_x;
				min_y = pre_y;
			}
		}
		stbtt_FreeShape(&font, vertices);
		
		ofs_x = max_x;
		
	}
	
	
	
	if (w != NULL) *w = max_x - min_x;

	return line_list;
}