#include "gui.h"

void gui_scr_coord (gui_obj *gui, int scr_x, int scr_y, double *x, double *y){
	*x = (double) scr_x / gui->zoom + gui->ofs_x;
	*y = (double) scr_y / gui->zoom + gui->ofs_y;
}

void gui_coord_scr (gui_obj *gui, double x, double y, int *scr_x, int *scr_y){
	*scr_x = (int) (x - gui->ofs_x) * gui->zoom;
	*scr_y = (int) (y - gui->ofs_y) * gui->zoom;
}

int gui_scr_visible (gui_obj *gui, double x, double y){
	int scr_x = 0, scr_y = 0;
	gui_coord_scr (gui, x, y, &scr_x, &scr_y);
	
	return (scr_x > 0 && scr_x < gui->win_w && scr_y > 90 && scr_y < gui->win_h - 90);
	
}

void gui_scr_centralize(gui_obj *gui, double x, double y){
	double ofs_x = gui->win_w / 2 / gui->zoom;
	double ofs_y = gui->win_h / 2 / gui->zoom;
	
	gui->ofs_x = x - ofs_x;
	gui->ofs_y = y - ofs_y;
	
}

void sel_list_clear (gui_obj *gui){
	list_clear(gui->sel_list);
	list_mem_pool(ZERO_LIST, SEL_LIFE);
}

/* ***********************************************************
the fowling functions are for rendering text in gui, by rastering each
glyphs in a monochrome image in memory. */

static void plot_(unsigned char rast[][4], int w, int h, int x, int y, double a){
	
	
	y = h - y;
	
	if (x < 0 || y < 0 || w < 0) return;
	if (x > 24 || y > 19 || w > 24) return;
	
	int ofs = y * w + x;
	int alpha = rast[ofs][3] + a * 255;
	
	rast[ofs][0] = 255;
	rast[ofs][1] = 255;
	rast[ofs][2] = 255;
	rast[ofs][3] = (alpha < 255) ? alpha : 255;
}

/* Xiaolin Wu antialising line algorithm
font: http://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C */
#define ipart_(X) ((int)(X))
#define round_(X) ((int)(((double)(X))+0.5))
#define fpart_(X) (((double)(X))-(double)ipart_(X))
#define rfpart_(X) (1.0-fpart_(X))
#ifdef _MSC_VER
//silly,stupid,i know,msvc does not support this feature now
#define swap_(a, b) do{ double tmp;  tmp = a; a = b; b = tmp; }while(0)
#else
#define swap_(a, b) do{ __typeof__(a) tmp;  tmp = a; a = b; b = tmp; }while(0)
#endif
static void line_antialias( unsigned char rast[][4], int w, int h, double x1, double y1, double x2, double y2){
	double dx = x2 - x1;
	double dy = y2 - y1;
	
	if ( fabs(dx) > fabs(dy) ) {
		if ( x2 < x1 ) {
			swap_(x1, x2);
			swap_(y1, y2);
		}
		double gradient = dy / dx;
		double xend = round_(x1);
		double yend = y1 + gradient*(xend - x1);
		double xgap = rfpart_(x1 + 0.5);
		int xpxl1 = xend;
		int ypxl1 = ipart_(yend);
		plot_(rast, w, h, xpxl1, ypxl1, rfpart_(yend)*xgap);
		plot_(rast, w, h, xpxl1, ypxl1+1, fpart_(yend)*xgap);
		double intery = yend + gradient;

		xend = round_(x2);
		yend = y2 + gradient*(xend - x2);
		xgap = fpart_(x2+0.5);
		int xpxl2 = xend;
		int ypxl2 = ipart_(yend);
		plot_(rast, w, h, xpxl2, ypxl2, rfpart_(yend) * xgap);
		plot_(rast, w, h, xpxl2, ypxl2 + 1, fpart_(yend) * xgap);

		int x;
		for(x=xpxl1+1; x < xpxl2; x++) {
			plot_(rast, w, h, x, ipart_(intery), rfpart_(intery));
			plot_(rast, w, h, x, ipart_(intery) + 1, fpart_(intery));
			intery += gradient;
		}
	} else {
		if ( y2 < y1 ) {
			swap_(x1, x2);
			swap_(y1, y2);
		}
		double gradient = dx / dy;
		double yend = round_(y1);
		double xend = x1 + gradient*(yend - y1);
		double ygap = rfpart_(y1 + 0.5);
		int ypxl1 = yend;
		int xpxl1 = ipart_(xend);
		plot_(rast, w, h, xpxl1, ypxl1, rfpart_(xend)*ygap);
		plot_(rast, w, h, xpxl1 + 1, ypxl1, fpart_(xend)*ygap);
		double interx = xend + gradient;

		yend = round_(y2);
		xend = x2 + gradient*(yend - y2);
		ygap = fpart_(y2+0.5);
		int ypxl2 = yend;
		int xpxl2 = ipart_(xend);
		plot_(rast, w, h, xpxl2, ypxl2, rfpart_(xend) * ygap);
		plot_(rast, w, h, xpxl2 + 1, ypxl2, fpart_(xend) * ygap);

		int y;
		for(y=ypxl1+1; y < ypxl2; y++) {
			plot_(rast, w, h, ipart_(interx), y, rfpart_(interx));
			plot_(rast, w, h, ipart_(interx) + 1, y, fpart_(interx));
			interx += gradient;
		}
	}
}
#undef swap_
//#undef plot_
#undef ipart_
#undef fpart_
#undef round_
#undef rfpart_

static void graph_aa(graph_obj * master, unsigned char rast[][4], int w, int h, double scale){
	if ((master == NULL) || (rast == NULL)) return;
	double xd0, yd0, xd1, yd1;
	line_node *current = master->list->next;
	int ok = 1;

	/* draw the lines */
	while(current){ /*sweep the list content */
		/* apply the scale and offset */
		ok = 1;
		
		ok &= !(isnan(xd0 = ((current->x0 ) * scale) + 0.5));
		ok &= !(isnan(yd0 = ((current->y0 ) * scale) + 0.5));
		ok &= !(isnan(xd1 = ((current->x1 ) * scale) + 0.5));
		ok &= !(isnan(yd1 = ((current->y1 ) * scale) + 0.5));
		
		if (ok){
			line_antialias(rast, w, h, xd0, yd0, xd1, yd1);
		}
		current = current->next; /* go to next */
	}
}


struct gui_font * gui_new_font (struct nk_user_font *base_font){
	/* create a new font structure */
	/* memory allocation */
	struct gui_font *font = calloc(1, sizeof(struct gui_font));
	if (font){
		/* initial parameters */
		font->font = base_font;
		font->glyphs = NULL;
		font->next = NULL;
	}
	return font;
}

struct gui_font * gui_font_add (struct gui_font *list, struct nk_user_font *base_font){
	/* create a new font structute and append in list*/
	if (!list) return NULL;
	/* create font */
	struct gui_font *font = gui_new_font (base_font);
	if (font){
		struct gui_font *curr = list;
		/* find the list tail */
		while (curr->next){
			curr = curr->next;
		}
		/*atach the new font at list tail */
		curr->next = font;
	}
	return font;
}

struct gui_font * gui_get_font (struct gui_font *list, struct nk_user_font *base_font){
	/* return a font structure in list, by its pointer */
	if (!list) return NULL;
	
	/* try to find the font previously stored */
	struct gui_font *curr = list->next;
	while(curr){ /*sweep list */
		if (curr->font == base_font) return curr; /* success */
		curr = curr->next;
	}
	
	/* if not found, add the font in list*/
	return gui_font_add (list, base_font);
}

struct gui_glyph * gui_new_glyph (struct gui_font *gfont, int code_p){
	/* create a new glyph structute */
	if (!gfont) return NULL;
	
	/* memory allocation */
	struct gui_glyph *glyph = calloc(1, sizeof(struct gui_glyph));
	if (glyph){
		/* initial parameters */
		glyph->code_p = code_p;
		glyph->w = 0;
		glyph->h = 0;
		glyph->adv = 0.0;
		memset(glyph->rast, 0, 500);
		glyph->next = NULL;
	}
	//else return NULL;
	/* -------- raster the glyph and get its size parameters ----------*/
	struct tfont *font = (struct tfont *)gfont->font->userdata.ptr;
	if (font->type != FONT_TT){ /* shape font*/
		double w = 0.0, h = 0.0;
		
		/* rendering text by default general drawing engine */
		graph_obj * graph = font_parse_cp(font, code_p, 0, FRAME_LIFE, &w);
		h = gfont->font->height;
		w = w * h + 1;
		if (graph) {
			graph_aa(graph, glyph->rast, w, h+1, h);
		}
		/* update glyph parameters */
		glyph->x = 0;
		glyph->y = -h;
		glyph->w = w;
		glyph->h = h*1.5;
		glyph->adv = w-0.5;
		
	}
	else { /* True Type font*/
		struct tt_font *tt_fnt = font->data;
		float scale = tt_fnt->scale * gfont->font->height;
		int x0, y0, x1, y1, w, h;
		
		/* try to find tt glyph */
		struct tt_glyph *curr_glyph =  tt_find_cp (tt_fnt, code_p);
		if (!curr_glyph){/* if not found, add the glyph in font for future use*/
			curr_glyph = tt_get_glyph (tt_fnt, code_p);
			if(curr_glyph){
				if (tt_fnt->end) tt_fnt->end->next = curr_glyph;
				tt_fnt->end = curr_glyph;
			}
		}
		if (curr_glyph){ /* glyph found */
			/* find size parameters */
			stbtt_GetGlyphBitmapBox((stbtt_fontinfo *) tt_fnt->info, curr_glyph->g_idx, scale, scale, &x0, &y0, &x1, &y1);
			w = x1 - x0; h = y1 - y0;
			/* size limits */
			w = (w < 25)? w : 25;
			h = (h < 20)? h : 20;
			/* rasterize glyph*/
			unsigned char rast[20*25];
			stbtt_MakeGlyphBitmap((stbtt_fontinfo *) tt_fnt->info, rast, w, h, 25, scale, scale, curr_glyph->g_idx);
			
			/* copy rasterized glyph in main image */
			int i = 0, j = 0;
			for (j = 0; j < h; j++){
				for (i = 0; i < w; i++){ /* pixel by pixel */
					//img->frg.a = (curr_glyph->rast[j * 25 + i] * color.a) / 255;
					//bmp_point_raw (img, x + i, y + j);
					glyph->rast[j*w+i][0] = 255;
					glyph->rast[j*w+i][1] = 255;
					glyph->rast[j*w+i][2] = 255;
					glyph->rast[j*w+i][3] = rast[j * 25 + i] ;
				}
			}
			
			/* update glyph parameters */
			glyph->x = x0;
			glyph->y = y0;
			glyph->w = w;
			glyph->h = h;
			glyph->adv = (double)curr_glyph->adv * (double)gfont->font->height;
			
		}
	}
	/* ------------------------------------------------------------ */
	return glyph;
}

struct gui_glyph * gui_glyph_add (struct gui_font *font, int code_p){
	/* create a new glyph structute and append in font*/
	if (!font) return NULL;
	/* create glyph */
	struct gui_glyph *glyph = gui_new_glyph (font, code_p);
	if (glyph){
		struct gui_glyph *curr = font->glyphs;
		/* find the font's tail */
		if (curr){
			while (curr->next){
				curr = curr->next;
			}
			/*atach the new glyph at font's tail */
			curr->next = glyph;
		}
		else{ /* if is the first glyph in font */ 
			font->glyphs = glyph;
		}
	}
	
	return glyph;
}

struct gui_glyph * gui_get_glyph (struct gui_font *font, int code_p){
	/* return a glyph structure in font, by its code point (unicode) */
	if (!font) return NULL;
	
	/* try to find the glyph previously stored */
	struct gui_glyph *curr = font->glyphs;
	while(curr){ /* sweep font */
		if (curr->code_p == code_p) return curr; /* success */
		curr = curr->next;
	}
	/* if not found, add the glyph in font*/
	return gui_glyph_add (font, code_p);
}

int gui_list_font_free (struct gui_font *list){
	/* free list of fonts and all its childs structures */
	if (!list) return 0;
	struct gui_font *curr_font = list->next;
	struct gui_font *next_font;
	struct gui_glyph *curr_glyph, *next_glyph;
	
	while(curr_font){ /* sweep font list */
		next_font = curr_font->next;
		curr_glyph = curr_font->glyphs;
	
		while(curr_glyph){ /* sweep current font */
			next_glyph = curr_glyph->next;
			free(curr_glyph); /* free each glyph */
			curr_glyph = next_glyph;
		}
		free (curr_font); /* free each font */
		curr_font = next_font;
	}
	
	free (list); /* free main list structure */
}

float gui_str_width(nk_handle handle, float height, const char *text, int len){
	/* get width of a rendered text */
	/* callback function for Nuklear internal calculations */
	struct tfont *font = (struct tfont *)handle.ptr;
	if ((text== NULL) || (font==NULL))  return 0.0;
	int ofs = 0, str_start = 0, code_p;
	
	float width = 0.0;
	double w = 0.0;
	struct tt_glyph *curr_glyph = NULL;
	graph_obj * graph = NULL;
	
	while (str_start < len){ /* parse each UTF8 codepoint in text */
		ofs = utf8_to_codepoint((char *)text + str_start, &code_p);
		str_start += ofs;
		
		if (font->type != FONT_TT) /* shape font*/{	
			w = 0.0;
			
			/* rendering text by default general drawing engine */
			graph = font_parse_cp(font, code_p, 0, FRAME_LIFE, &w);
			
			w = w * height + 1; /* width is proportional to height */
			width += w-0.5; /* update width */
			
		}
		else{ /* true type font */
			w = 0.0;
			curr_glyph = NULL;
			curr_glyph = tt_find_cp ((struct tt_font *) font->data, code_p); 
			if (curr_glyph){
				w = curr_glyph->adv; /* get advance in stored glyph */
			}
			/* width is proportional to height */
			width += w * height; /* update width */
		}
	}
	return width;
}
/* ************************************************************* */

int gui_selectable (gui_obj *gui, const char *title, int active){
	struct nk_style_button *sel_type;
	
	/* verify if active or not */
	sel_type = &gui->b_icon_unsel;
	if (active) sel_type = &gui->b_icon_sel;
	/* do the button */
	int r = nk_button_label_styled(gui->ctx, sel_type, title);
	
	return r;
}

int gui_tab (gui_obj *gui, const char *title, int active){
	const struct nk_user_font *f = gui->ctx->style.font;
	struct nk_style_button *sel_type;
	
	/* calcule button size */ 
	float text_width = f->width(f->userdata, f->height, title, nk_strlen(title));
	float widget_width = text_width + 6 * gui->ctx->style.button.padding.x;
	/* verify if active or not */
	sel_type = &gui->b_icon_unsel;
	if (active) sel_type = &gui->b_icon_sel;
	/* do the button */
	nk_layout_row_push(gui->ctx, widget_width);
	int r = nk_button_label_styled(gui->ctx, sel_type, title);
	
	return r;
}

int gui_tab_img (gui_obj *gui, bmp_img *img, int active, int w){
	
	struct nk_style_button *sel_type;
	
	/* verify if active or not */
	sel_type = &gui->b_icon_unsel;
	if (active) sel_type = &gui->b_icon_sel;
	/* do the button */
	nk_layout_row_push(gui->ctx, w);
	int r = nk_button_image_styled(gui->ctx, sel_type, nk_image_ptr(img));
	
	return r;
}

/* ************************************************** */

int gui_create_modal_cur(gui_obj *gui){
	SDL_Surface *surface;
	
	int cur_img[MODAL_SIZE];
	cur_img[SELECT] = SVG_CURSOR;
	cur_img[LINE] = SVG_LINE;
	cur_img[POLYLINE] = SVG_PLINE;
	cur_img[CIRCLE] = SVG_CIRCLE;
	cur_img[RECT] = SVG_RECT;
	cur_img[TEXT] = SVG_TEXT;
	cur_img[MTEXT] = SVG_I_TEXT;
	cur_img[ARC] = SVG_ARC;
	cur_img[DUPLI] = SVG_DUPLI;
	cur_img[MOVE] = SVG_MOVE;
	cur_img[SCALE] = SVG_SCALE;
	cur_img[ROTATE] = SVG_ROT;
	cur_img[NEW_BLK] = SVG_BLOCK;
	cur_img[HATCH] = SVG_HATCH;
	cur_img[INSERT] = SVG_BOOK;
	cur_img[PASTE] = SVG_PASTE;
	cur_img[MIRROR] = SVG_MIRROR;
	cur_img[ED_TEXT] = SVG_TEXT_E;
	cur_img[ED_ATTR] = SVG_TAG_E;
	cur_img[SCRIPT] = SVG_CURSOR;
	cur_img[SPLINE] = SVG_SPLINE;
	cur_img[ELLIPSE] = SVG_ELIPSE;
	cur_img[IMAGE] = SVG_IMAGE;
	cur_img[ADD_ATTRIB] = SVG_TAG;
	cur_img[ADD_XDATA] = SVG_TAG;
	cur_img[EXPLODE] = SVG_EXPLODE;
	cur_img[MEASURE] = SVG_RULER;
	cur_img[FIND] = SVG_FIND;
	cur_img[PROP] = SVG_STYLE;
	cur_img[TXT_PROP] = SVG_TEXT_STY;
	cur_img[VERTEX] = SVG_EDIT;
	cur_img[DIM_LINEAR] = SVG_DIM_LINEAR;
	cur_img[DIM_ANGULAR] = SVG_DIM_ANGULAR;
	cur_img[DIM_RADIUS] = SVG_DIM_RADIUS;
	cur_img[DIM_ORDINATE] = SVG_DIM_ORDINATE;
	cur_img[ZOOM] = SVG_ZOOM_W;
	int i;
	for (i = 0; i < MODAL_SIZE; i++){
		surface = SDL_CreateRGBSurfaceFrom(
			(void *)gui->svg_bmp[cur_img[i]]->buf,
			gui->svg_bmp[cur_img[i]]->width,
			gui->svg_bmp[cur_img[i]]->height,
			32, gui->svg_bmp[cur_img[i]]->width * 4,
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
			
		gui->modal_cursor[i] = SDL_CreateColorCursor(surface, 0, 0);
		SDL_FreeSurface(surface);
	}
	
	return 1;
}

int gui_free_modal_cur(gui_obj *gui){
	int i;
	for (i = 0; i < MODAL_SIZE; i++){
		SDL_FreeCursor(gui->modal_cursor[i]);
	}
	
	return 1;
}

int gui_default_modal(gui_obj *gui){
	if (gui->modal == NEW_BLK)
		gui->show_blk_mng = 1;
	
	gui->modal = SELECT;
	gui->sel_ent_filter = ~DXF_NONE;
	
	return gui_first_step(gui);
}

int gui_first_step(gui_obj *gui){
	gui->en_distance = 0;
	gui->draw_tmp = 0;
	gui->element = NULL;
	gui->draw = 1;
	gui->step = 0;
	gui->draw_phanton = 0;
	gui->draw_vert = 0;
	gui->vert_idx = -1;
	return gui_next_step(gui);
}
int gui_next_step(gui_obj *gui){
	gui->free_sel = 0;
	gui->lock_ax_x = 0;
	gui->lock_ax_y = 0;
	gui->user_flag_x = 0;
	gui->user_flag_y = 0;

	gui->draw = 1;
	return 1;
}

void set_style(gui_obj *gui, enum theme theme){
	struct nk_color table[NK_COLOR_COUNT];
	if (theme == THEME_WHITE) {
		table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
		table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
		table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_RED) {
		table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
		table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
		table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
		table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
		table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_BLUE) {
		table[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
		table[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
		table[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
		table[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
		table[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_DARK) {
		table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
		table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
		table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
		table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_GREEN) {
		table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(57, 71, 58, 215);
		table[NK_COLOR_HEADER] = nk_rgba(52, 57, 52, 220);
		table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(48, 112, 54, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(71, 161, 80, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(89, 201, 100, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(73, 84, 72, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 112, 54, 255);
		table[NK_COLOR_SELECT] = nk_rgba(58, 67, 57, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 112, 54, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 112, 54, 245);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(59, 115, 53, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(71, 161, 80, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_EDIT] = nk_rgba(50, 61, 50, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_COMBO] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_CHART] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 112, 54, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 61, 50, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 112, 54, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(59, 115, 53, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(71, 161, 80, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 112, 54, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_BROWN) {
		table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(71, 67, 57, 215);
		table[NK_COLOR_HEADER] = nk_rgba(56, 51, 51, 220);
		table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(111, 83, 48, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(121, 93, 58, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(126, 98, 63, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(56, 53, 45, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(111, 83, 48, 255);
		table[NK_COLOR_SELECT] = nk_rgba(61, 67, 57, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(111, 83, 48, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(111, 83, 48, 245);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(116, 88, 53, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(121, 93, 58, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_EDIT] = nk_rgba(61, 58, 50, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_COMBO] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_CHART] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(111, 83, 48, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(0, 0, 255, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(61, 58, 50, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(111, 83, 48, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(116, 88, 53, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(121, 93, 58, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(111, 83, 48, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_PURPLE) {
		table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(67, 57, 71, 215);
		table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
		table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(83, 48, 111, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(93, 58, 121, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(98, 63, 126, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(53, 45, 56, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(83, 48, 111, 255);
		table[NK_COLOR_SELECT] = nk_rgba(67, 57, 61, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(83, 48, 111, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(83, 48, 111, 245);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(88, 53, 116, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(93, 58, 121, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_EDIT] = nk_rgba(58, 50, 61, 225);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
		table[NK_COLOR_COMBO] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_CHART] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(83, 48, 111, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(0, 255, 0, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(58, 50, 61, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(83, 48, 111, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(88, 53, 116, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(93, 58, 121, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(83, 48, 111, 255);
		nk_style_from_table(gui->ctx, table);
	} else if (theme == THEME_DRACULA) {
		table[NK_COLOR_TEXT] = nk_rgba(248, 248, 242, 255);
		table[NK_COLOR_WINDOW] = nk_rgba(68, 71, 90, 255);
		table[NK_COLOR_HEADER] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
		table[NK_COLOR_BUTTON] = nk_rgba(98, 114, 164, 255);
		table[NK_COLOR_BUTTON_HOVER] = nk_rgba(255, 121, 198, 255);
		table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(255, 85, 85, 255);
		table[NK_COLOR_TOGGLE] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(98, 114, 164, 255);
		table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(255, 85, 85, 255);
		table[NK_COLOR_SELECT] = nk_rgba(98, 114, 164, 255);
		table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(255, 85, 85, 255);
		table[NK_COLOR_SLIDER] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(98, 114, 164, 255);
		table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(255, 121, 198, 255);
		table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(255, 85, 85, 255);
		table[NK_COLOR_PROPERTY] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_EDIT] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_EDIT_CURSOR] = nk_rgba(255, 184, 108, 255);
		table[NK_COLOR_COMBO] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_CHART] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_CHART_COLOR] = nk_rgba(68, 71, 90, 255);
		table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 184, 108, 255);
		table[NK_COLOR_SCROLLBAR] = nk_rgba(40, 42, 54, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(98, 114, 164, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(255, 121, 198, 255);
		table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(255, 85, 85, 255);
		table[NK_COLOR_TAB_HEADER] = nk_rgba(255, 85, 85, 255);
		nk_style_from_table(gui->ctx, table);
	/*} else if (theme == THEME_TEST) {
		
		nk_style_from_table(gui->ctx, table);*/
	} else {
		nk_style_default(gui->ctx);
	}

	gui->ctx->style.edit.padding = nk_vec2(4, -6);
	//gui->ctx->style.button.rounding = 10.0;
	
	gui->b_icon = gui->ctx->style.button;
	
	gui->b_icon.image_padding.x = -4;
	gui->b_icon.image_padding.y = -4;
	
	/* style for toggle buttons (or select buttons) with image */
	//struct nk_style_button gui->b_icon_sel, gui->b_icon_unsel;
	
	gui->b_icon_sel = gui->ctx->style.button;
	gui->b_icon_unsel = gui->ctx->style.button;
	
	//gui->b_icon_unsel.normal = nk_style_item_color(nk_rgba(58, 67, 57, 255));
	gui->b_icon_unsel.normal = gui->ctx->style.checkbox.normal;
	//gui->b_icon_unsel.hover = nk_style_item_color(nk_rgba(73, 84, 72, 255));
	gui->b_icon_unsel.hover =  gui->ctx->style.checkbox.hover;
	
	//gui->b_icon_unsel.active = nk_style_item_color(nk_rgba(81, 92, 80, 255));
	gui->b_icon_sel.image_padding.x = -4;
	gui->b_icon_sel.image_padding.y = -4;
	gui->b_icon_unsel.image_padding.x = -4;
	gui->b_icon_unsel.image_padding.y = -4;
    
    
}

bmp_color nk_to_bmp_color(struct nk_color color){
	bmp_color ret_color = {.r = color.r, .g = color.g, .b = color.b, .a = color.a };
	return ret_color;
}

void nk_to_gl_color(struct ogl *gl_ctx, struct nk_color color){
	gl_ctx->fg[0] = color.r;
	gl_ctx->fg[1] = color.g;
	gl_ctx->fg[2] = color.b;
	gl_ctx->fg[3] = color.a;
}

int gui_check_draw(gui_obj *gui){
	int draw = 0;
	
	draw = 0;
	if (gui){
		//gui->draw = 0;
		if (gui->ctx){
			void *cmds = nk_buffer_memory(&(gui->ctx->memory));
			if (cmds){
				if (memcmp(cmds, gui->last, gui->ctx->memory.allocated)) {
					memcpy(gui->last, cmds, gui->ctx->memory.allocated);
					//gui->draw = 1;
					draw = 1;
				}
			}
		}
	}
	return draw;
}

int nk_gl_render(gui_obj *gui) {
	if (!gui) return 0;
	
	struct ogl *gl_ctx = &(gui->gl_ctx);
	const struct nk_command *cmd = NULL;
	
	gl_ctx->flip_y = 1;
	gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	gl_ctx->vert_count = 0;
	gl_ctx->elem_count = 0;
	glUniform1i(gl_ctx->tex_uni, 0);
	
	
	gl_ctx->transf[0][0] = 1.0;
	gl_ctx->transf[0][1] = 0.0;
	gl_ctx->transf[0][2] = 0.0;
	gl_ctx->transf[0][3] = 0.0;
	gl_ctx->transf[1][0] = 0.0;
	gl_ctx->transf[1][1] = 1.0;
	gl_ctx->transf[1][2] = 0.0;
	gl_ctx->transf[1][3] = 0.0;
	gl_ctx->transf[2][0] = 0.0;
	gl_ctx->transf[2][1] = 0.0;
	gl_ctx->transf[2][2] = 1.0;
	gl_ctx->transf[2][3] = 0.0;
	gl_ctx->transf[3][0] = 0.0;
	gl_ctx->transf[3][1] = 0.0;
	gl_ctx->transf[3][2] = 0.0;
	gl_ctx->transf[3][3] = 1.0;
	glDepthFunc(GL_ALWAYS);
	
	
	nk_foreach(cmd, gui->ctx){
		if (gl_ctx->elems == NULL){
			gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		}
		
		if (cmd->type == NK_COMMAND_LINE) {
			const struct nk_command_line *l = (const struct nk_command_line *)cmd;
			nk_to_gl_color(gl_ctx, l->color);
			draw_gl_line (gl_ctx, (int []){l->begin.x, l->begin.y, 0}, (int []){l->end.x,l->end.y, 0}, l->line_thickness);
		}
		else if (cmd->type == NK_COMMAND_RECT) {
			const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
			
			int x0, y0, x1, y1, i;
			double step = 2 * M_PI  / 16.0, cx, cy;
			
			nk_to_gl_color(gl_ctx, r->color);
			
			draw_gl_line (gl_ctx, (int []){r->x + r->rounding, r->y, 0}, (int []){r->x + r->w -r->rounding, r->y, 0}, r->line_thickness);
			x0 =  r->x + r->w - r->rounding;
			cx = x0;
			y0 = r->y;
			cy = r->y + r->rounding;
			for (i=13; i <= 16; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){x1, y1, 0}, r->line_thickness);
				x0 = x1;
				y0 = y1;
			}
			draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ r->x + r->w, r->y + r->h - r->rounding, 0}, r->line_thickness);
			cx = r->x + r->w - r->rounding;
			x0 = r->x + r->w;
			y0 = r->y + r->h - r->rounding;
			cy = y0;
			for (i=1; i <= 4; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){x1, y1, 0}, r->line_thickness);
				x0 = x1;
				y0 = y1;
			}
			draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ r->x + r->rounding, r->y + r->h, 0}, r->line_thickness);
			x0 = r->x + r->rounding;
			cx = x0;
			y0 = r->y + r->h;
			cy = r->y + r->h - r->rounding;
			for (i=5; i <= 8; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ x1, y1, 0}, r->line_thickness);
				x0 = x1;
				y0 = y1;
			}
			draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ r->x, r->y + r->rounding, 0}, r->line_thickness);
			cx = r->x + r->rounding;
			x0 = r->x;
			y0 = r->y + r->rounding;
			cy = y0;
			for (i=9; i < 12; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ x1, y1, 0}, r->line_thickness);
				x0 = x1;
				y0 = y1;
			}
			draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){r->x + r->rounding, r->y, 0}, r->line_thickness);
		}
		else if (cmd->type == NK_COMMAND_RECT_FILLED) {
			const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
			
			int x0, y0, x1, y1, i;
			double step = 2 * M_PI  / 16.0, cx, cy;
			struct edge edges[20];
			
			nk_to_gl_color(gl_ctx, r->color);
			
			edges[0] = (struct edge){r->x + r->rounding, 0, r->y, r->x + r->w -r->rounding, r->y, 0};
			x0 =  r->x + r->w - r->rounding;
			cx = x0;
			y0 = r->y;
			cy = r->y + r->rounding;
			for (i=13; i <= 16; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				edges[i] = (struct edge){x0, y0, 0, x1, y1, 0};
				x0 = x1;
				y0 = y1;
			}
			edges[17] = (struct edge){x0, y0, 0, r->x + r->w, r->y + r->h - r->rounding, 0};
			cx = r->x + r->w - r->rounding;
			x0 = r->x + r->w;
			y0 = r->y + r->h - r->rounding;
			cy = y0;
			for (i=1; i <= 4; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				edges[i] = (struct edge){x0, y0, 0, x1, y1, 0};
				x0 = x1;
				y0 = y1;
			}
			edges[18] = (struct edge){x0, y0, 0, r->x + r->rounding, r->y + r->h, 0};
			x0 = r->x + r->rounding;
			cx = x0;
			y0 = r->y + r->h;
			cy = r->y + r->h - r->rounding;
			for (i=5; i <= 8; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				edges[i] = (struct edge){x0, y0, 0, x1, y1, 0};
				x0 = x1;
				y0 = y1;
			}
			edges[19] = (struct edge){x0, y0, 0, r->x, r->y + r->rounding, 0};
			cx = r->x + r->rounding;
			x0 = r->x;
			y0 = r->y + r->rounding;
			cy = y0;
			for (i=9; i < 12; i++){
				x1 = cx +  r->rounding * cos(i * step);
				y1 = cy +  r->rounding * sin(i * step);
				edges[i] = (struct edge){x0, y0, 0, x1, y1, 0};
				x0 = x1;
				y0 = y1;
			}
			edges[12] = (struct edge){x0, y0, 0, r->x + r->rounding, r->y, 0};
			draw_gl_polygon (gl_ctx, 20, edges);
		}
		else if (cmd->type == NK_COMMAND_IMAGE) {
			const struct nk_command_image *i = (struct nk_command_image *)cmd;
			bmp_img *img = (bmp_img *)i->img.handle.ptr;
			
			if (i->h > 0 && i->w > 0){
				/* draw bitmap image */
				draw_gl (gl_ctx, 1); /* force draw previous commands and cleanup */
				/* choose blank base color */
				gl_ctx->fg[0] = 255;
				gl_ctx->fg[1] = 255;
				gl_ctx->fg[2] = 255;
				gl_ctx->fg[3] = 255;
				/* prepare for new opengl commands */
				gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
				glUniform1i(gl_ctx->tex_uni, 1); /* choose second texture */
				/* finally draw image */
				//draw_gl_image (gl_ctx, i->x, i->y, img->width, img->height, img);
				draw_gl_image_rec (gl_ctx, i->x, i->y, 0, i->w, i->h, img);
				draw_gl (gl_ctx, 1); /* force draw and cleanup */
			}
		}
		else if  (cmd->type == NK_COMMAND_TEXT) {
			const struct nk_command_text *t = (const struct nk_command_text*)cmd;
			nk_to_gl_color(gl_ctx, t->foreground);
			bmp_color color = nk_to_bmp_color(t->foreground);
			
			/* get font descriptor */
			struct tfont *font = (struct tfont *)t->font->userdata.ptr;
			
			#if (0)
			if (font->type != FONT_TT){ /*shape font */
				/* rendering text by default general drawing engine */
				list_node * graph = list_new(NULL, FRAME_LIFE);
				if (graph) {
					if (font_parse_str(font, graph, FRAME_LIFE, (char *)t->string, NULL, 0)){
						graph_list_color(graph, color);
						graph_list_modify(graph, t->x, t->y + t->font->height, t->font->height, -t->font->height, 0.0);
						
						struct draw_param param = {.ofs_x = 0, .ofs_y = 0, .scale = 1.0, .list = NULL, .subst = NULL, .len_subst = 0, .inc_thick = 0};
						graph_list_draw_gl(graph, gl_ctx, param);
					}
				}
			}
			#endif
			//else { /* True Type font */
				/* rendering text by glyph raster images - used only in GUI */
				
				
				/* get gui font structure */
				struct gui_font * gfont = gui_get_font (gui->ui_font_list, (struct nk_user_font *) t->font);
				struct gui_glyph *curr_glyph = NULL;
				int ofs = 0, str_start = 0, code_p;
				double ofs_x = 0.0;
				
				draw_gl (gl_ctx, 1); /* force draw previous commands and cleanup */
				/* prepare for new opengl commands */
				gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
				glUniform1i(gl_ctx->tex_uni, 1); /* choose second texture */
				/* finally draw image */
				int height = t->font->height + 5;
				if (font->type == FONT_TT){
					struct tt_font *tt_fnt = font->data;
					int desc = fabs(tt_fnt->descent) * (double)t->font->height + 0.5;
					//int lg = fabs(tt_fnt->line_gap) * (double)t->font->height + 0.5;
					height = t->font->height + desc + 1;// + lg;
				}
				
				
				draw_gl_rect (gl_ctx, t->x, t->y-3, 0, t->w, height);
				
				/* sweep the text string, decoding UTF8 in code points*/
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
				
				if (t->w > gl_ctx->tex_w || height > gl_ctx->tex_h){ /* verify if image is greater then texture */
					gl_ctx->tex_w = (gl_ctx->tex_w > t->w) ? gl_ctx->tex_w : t->w;
					gl_ctx->tex_h = (gl_ctx->tex_h > height) ? gl_ctx->tex_h : height;
					/* realoc texture */
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_ctx->tex_w, gl_ctx->tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				}
				
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t->w, height, GL_RGBA, GL_UNSIGNED_BYTE, gui->blank_tex);
				
				while (ofs = utf8_to_codepoint((char *)t->string + str_start, &code_p)){
					str_start += ofs;
					curr_glyph = gui_get_glyph (gfont, code_p);
					
					/* calcule current glyph position in texture */
					int x = curr_glyph->x + (int) ofs_x;
					int y = curr_glyph->y + (int) t->font->height + 1;
					
					x = (x < 0 ) ? 0 : x;
					y = (y < 0 ) ? 0 : y;
					
					int w = curr_glyph->w;
					int h = curr_glyph->h;
					
					glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, curr_glyph->rast);
					
					ofs_x += curr_glyph->adv; /*update position for next glyph */
				}
				
				draw_gl (gl_ctx, 1); /* force draw and cleanup */
			//}
			
		}
		else if  (cmd->type == NK_COMMAND_SCISSOR) {
			const struct nk_command_scissor *s =(const struct nk_command_scissor*)cmd;
			draw_gl (gl_ctx, 1); /* force draw previous commands and cleanup */
			glScissor((GLint)s->x, (GLint)(gl_ctx->win_h - s->y - s->h), (GLsizei)s->w, (GLsizei)s->h);
			glEnable(GL_SCISSOR_TEST);
		}
		
		else if  (cmd->type == NK_COMMAND_TRIANGLE) {
			
			const struct nk_command_triangle*t = (const struct nk_command_triangle*)cmd;
			
			/*change the color */
			nk_to_gl_color(gl_ctx, t->color);
			
			draw_gl_line (gl_ctx, (int []){t->a.x, t->a.y, 0}, (int []){t->b.x, t->b.y, 0}, t->line_thickness);
			draw_gl_line (gl_ctx, (int []){t->b.x, t->b.y, 0}, (int []){t->c.x, t->c.y, 0}, t->line_thickness);
			draw_gl_line (gl_ctx, (int []){t->c.x, t->c.y, 0}, (int []){t->a.x, t->a.y, 0}, t->line_thickness);
		} 
		
		else if  (cmd->type == NK_COMMAND_TRIANGLE_FILLED){
			const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
			
			/*change the color */
			nk_to_gl_color(gl_ctx, t->color);
			
			draw_gl_triang (gl_ctx, (int []){t->a.x, t->a.y, 0},
					(int []){t->b.x, t->b.y, 0},
					(int []){t->c.x, t->c.y, 0});
		}
		else if  (cmd->type == NK_COMMAND_CIRCLE) {
			const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, c->color);
			
			int x0, y0, x1, y1, i;
			double major_ax, minor_ax, cx, cy;
			//struct edge edges[20];
			
			major_ax = c->w / 2.0;
			minor_ax = c->h / 2.0;
			
			cx = c->x + major_ax;
			cy = c->y + minor_ax;
			
			x0 = 0.5 + cx + major_ax;
			y0 = 0.5 + cy;
			
			for (i = 1; i < 20; i++){
				x1 = 0.5 + cx + major_ax * cos( 0.1 * M_PI * i );
				y1 = 0.5 + cy + minor_ax * sin( 0.1 * M_PI * i );
				draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ x1, y1, 0}, c->line_thickness);
				x0 = x1;
				y0 = y1;
			}
			x1 = 0.5 + cx + major_ax;
			y1 = 0.5 + cy;
			draw_gl_line (gl_ctx, (int []){x0, y0, 0}, (int []){ x1, y1, 0}, c->line_thickness);
		} 		
		else if  (cmd->type == NK_COMMAND_CIRCLE_FILLED) {
			const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, c->color);
			
			int x0, y0, x1, y1, i;
			double major_ax, minor_ax, cx, cy;
			struct edge edges[20];
			
			major_ax = c->w / 2.0;
			minor_ax = c->h / 2.0;
			
			cx = c->x + major_ax;
			cy = c->y + minor_ax;
			
			x0 = 0.5 + cx + major_ax;
			y0 = 0.5 + cy;
			
			for (i = 1; i < 20; i++){
				x1 = 0.5 + cx + major_ax * cos( 0.1 * M_PI * i );
				y1 = 0.5 + cy + minor_ax * sin( 0.1 * M_PI * i );
				edges[i-1] = (struct edge){x0, y0, 0, x1, y1, 0};
				x0 = x1;
				y0 = y1;
			}
			x1 = 0.5 + cx + major_ax;
			y1 = 0.5 + cy;
			edges[19] = (struct edge){x0, y0, 0, x1, y1, 0};
			
			draw_gl_polygon (gl_ctx, 20, edges);
		} 
		else if  (cmd->type == NK_COMMAND_POLYGON) {
			const struct nk_command_polygon *p = (const struct nk_command_polygon*)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, p->color);
			
			//int i;
			//float vertices[p->point_count * 2];
			//for (i = 0; i < p->point_count; i++) {
			// vertices[i*2] = p->points[i].x;
			// vertices[(i*2) + 1] = p->points[i].y;
			//}
			//al_draw_polyline((const float*)&vertices, (2 * sizeof(float)),
			//    (int)p->point_count, ALLEGRO_LINE_JOIN_ROUND, ALLEGRO_LINE_CAP_CLOSED,
			//  color, (float)p->line_thickness, 0.0);
			//printf("polygon ");//------------------------------------teste
		}
		
		else if  (cmd->type == NK_COMMAND_POLYGON_FILLED) {
			const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, p->color);
			
			//int i;
			//float vertices[p->point_count * 2];
			// for (i = 0; i < p->point_count; i++) {
			//    vertices[i*2] = p->points[i].x;
			//     vertices[(i*2) + 1] = p->points[i].y;
			// }
			//  al_draw_filled_polygon((const float*)&vertices, (int)p->point_count, color);
			//printf("fill_polygon ");//------------------------------------teste
		}
		
		else if  (cmd->type == NK_COMMAND_POLYLINE) {
			const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, p->color);
			
			//int i;
			//float vertices[p->point_count * 2];
			//  for (i = 0; i < p->point_count; i++) {
			//      vertices[i*2] = p->points[i].x;
			//      vertices[(i*2) + 1] = p->points[i].y;
			//  }
			//  al_draw_polyline((const float*)&vertices, (2 * sizeof(float)),
			//      (int)p->point_count, ALLEGRO_LINE_JOIN_ROUND, ALLEGRO_LINE_CAP_ROUND,
			//      color, (float)p->line_thickness, 0.0);
			//printf("polyline ");//------------------------------------teste
		}
		else if  (cmd->type == NK_COMMAND_CURVE) {
			const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, q->color);
			
			// float points[8];
			// points[0] = (float)q->begin.x;
			// points[1] = (float)q->begin.y;
			// points[2] = (float)q->ctrl[0].x;
			// points[3] = (float)q->ctrl[0].y;
			//points[4] = (float)q->ctrl[1].x;
			// points[5] = (float)q->ctrl[1].y;
			// points[6] = (float)q->end.x;
			// points[7] = (float)q->end.y;
			// al_draw_spline(points, color, (float)q->line_thickness);
			//printf("curve ");//------------------------------------teste
		}
		else if  (cmd->type == NK_COMMAND_ARC) {
			const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
			/*change the color */
			nk_to_gl_color(gl_ctx, a->color);
			
			//    al_draw_arc((float)a->cx, (float)a->cy, (float)a->r, a->a[0],
			//       a->a[1], color, (float)a->line_thickness);
			//printf("arc ");//------------------------------------teste
		}
		else if  (cmd->type == NK_COMMAND_ARC_FILLED) {
			
		}
		else if  (cmd->type == NK_COMMAND_RECT_MULTI_COLOR) {
			
		}
		
		
		draw_gl (gl_ctx, 0);
	}
	
	draw_gl (gl_ctx, 1); /* force draw and cleanup */
	gl_ctx->flip_y = 0;
	glDisable(GL_SCISSOR_TEST);
	
	return 1;
}

static void nk_sdl_clipbard_paste(nk_handle usr, struct nk_text_edit *edit){
	const char *text = SDL_GetClipboardText();
	if (text) nk_textedit_paste(edit, text, nk_strlen(text));
	(void)usr;
}

static void nk_sdl_clipbard_copy(nk_handle usr, const char *text, int len){
	char *str = 0;
	(void)usr;
	if (!len) return;
	str = (char*)malloc((size_t)len+1);
	if (!str) return;
	memcpy(str, text, (size_t)len);
	str[len] = '\0';
	SDL_SetClipboardText(str);
	free(str);
}

NK_API int nk_sdl_init(gui_obj* gui){
	
	//gui_obj *gui = malloc(sizeof(gui_obj));
	
	if (gui){
		gui->ctx = malloc(sizeof(struct nk_context));
		gui->buf = calloc(1,FIXED_MEM);
		gui->last = calloc(1,FIXED_MEM);
		if((gui->ctx == NULL) || (gui->buf == NULL) || (gui->last == NULL)){
			return 0;
		}
	}
	else return 0;
	
	
	
	//nk_init_default(gui->ctx, font);
	//nk_init_fixed(gui->ctx, gui->buf, FIXED_MEM, font);
	nk_init_fixed(gui->ctx, gui->buf, FIXED_MEM, NULL);
	gui->ctx->clip.copy = nk_sdl_clipbard_copy;
	gui->ctx->clip.paste = nk_sdl_clipbard_paste;
	gui->ctx->clip.userdata = nk_handle_ptr(0);
	
	
	
	//struct tfont *ui_font = get_font_list(gui->font_list, "OpenSans-Regular.ttf");
	
	//struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
	
	//gui->ui_font.userdata = nk_handle_ptr(ui_font);
	//gui->ui_font.height = 11.0;

	gui->ui_font.width = gui_str_width;
	
	nk_style_set_font(gui->ctx, &(gui->ui_font));
	
	nk_textedit_init_default(&(gui->text_edit));
	nk_textedit_init_default(&(gui->debug_edit));
	
	return 1;
}

NK_API int
nk_sdl_handle_event(gui_obj *gui, SDL_Window *win, SDL_Event *evt)
{
	struct nk_context *ctx = gui->ctx;

	/* optional grabbing behavior 
	if (ctx->input.mouse.grab) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		ctx->input.mouse.grab = 0;
	}
	else if (ctx->input.mouse.ungrab) {
		int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(win, x, y);
		ctx->input.mouse.ungrab = 0;
	}*/
	if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
	/* key events */
		int down = evt->type == SDL_KEYDOWN;
		//const Uint8* state = SDL_GetKeyboardState(0);
		SDL_Keycode sym = evt->key.keysym.sym;
		SDL_Keymod mod = evt->key.keysym.mod;
		
		if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
			nk_input_key(ctx, NK_KEY_SHIFT, down);
		else if (sym == SDLK_DELETE)
			nk_input_key(ctx, NK_KEY_DEL, down);
		else if (sym == SDLK_RETURN)
			nk_input_key(ctx, NK_KEY_ENTER, down);
		else if (sym == SDLK_TAB)
			nk_input_key(ctx, NK_KEY_TAB, down);
		else if (sym == SDLK_BACKSPACE)
			nk_input_key(ctx, NK_KEY_BACKSPACE, down);
		else if (sym == SDLK_HOME) {
			nk_input_key(ctx, NK_KEY_TEXT_START, down);
			nk_input_key(ctx, NK_KEY_SCROLL_START, down);
		}
		else if (sym == SDLK_END) {
			nk_input_key(ctx, NK_KEY_TEXT_END, down);
			nk_input_key(ctx, NK_KEY_SCROLL_END, down);
		}
		else if (sym == SDLK_PAGEDOWN) {
			nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
		}
		else if (sym == SDLK_PAGEUP) {
			nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
		}
		else if (sym == SDLK_z)
			nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && (mod & KMOD_CTRL));
		else if (sym == SDLK_r)
			nk_input_key(ctx, NK_KEY_TEXT_REDO, down && (mod & KMOD_CTRL));
		else if (sym == SDLK_c)
			nk_input_key(ctx, NK_KEY_COPY, down && (mod & KMOD_CTRL));
		else if ((sym == SDLK_v) && ((mod & KMOD_CTRL))){
			nk_input_key(ctx, NK_KEY_PASTE, down && (mod & KMOD_CTRL));
			/*
			const char *text = SDL_GetClipboardText();
			if (text){
				nk_rune str_uni;
				int glyph_size;
				char *curr = (char *)text;
				
				while ((curr - text) < strlen(text)) {
				
					glyph_size = nk_utf_decode(curr, &str_uni, 2);
					if (glyph_size){
						nk_input_unicode(ctx, str_uni);
						curr += glyph_size;
					}
					else {
						break;
					}
				}
				SDL_free(text);
				SDL_Delay(100);
			}*/
		}
		else if (sym == SDLK_x)
			nk_input_key(ctx, NK_KEY_CUT, down && (mod & KMOD_CTRL));
		else if (sym == SDLK_b)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && (mod & KMOD_CTRL));
		else if (sym == SDLK_e)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && (mod & KMOD_CTRL));
		else if (sym == SDLK_UP)
			nk_input_key(ctx, NK_KEY_UP, down);
		else if (sym == SDLK_DOWN)
			nk_input_key(ctx, NK_KEY_DOWN, down);
		else if (sym == SDLK_LEFT) {
			if (mod & KMOD_CTRL)
				nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
			else nk_input_key(ctx, NK_KEY_LEFT, down);
		} 
		else if (sym == SDLK_RIGHT) {
			if (mod & KMOD_CTRL)
				nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
			else nk_input_key(ctx, NK_KEY_RIGHT, down);
		} else return 0;
		return 1;
	} 
	else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
		/* mouse button */
		int down = evt->type == SDL_MOUSEBUTTONDOWN;
		const int x = evt->button.x, y = evt->button.y;
		
		if (evt->button.button == SDL_BUTTON_LEFT) {
			if (evt->button.clicks > 1)
				nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
			nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
		}
		else if (evt->button.button == SDL_BUTTON_MIDDLE)
			nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
		else if (evt->button.button == SDL_BUTTON_RIGHT)
			nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
		else return 0;
		return 1;
	}
	else if (evt->type == SDL_MOUSEMOTION) {
		/* mouse motion */
		if (ctx->input.mouse.grabbed) {
			int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
			nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
		}
		else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
		return 1;
	}
	else if (evt->type == SDL_TEXTINPUT) {
		/* text input */
		nk_glyph glyph;
		memcpy(glyph, evt->text.text, NK_UTF_SIZE);
		nk_input_glyph(ctx, glyph);
		return 1;
	}
	else if (evt->type == SDL_MOUSEWHEEL) {
		/* mouse wheel */
		if (nk_window_is_any_hovered(ctx)){
			nk_input_scroll(ctx,nk_vec2((float)evt->wheel.x,(float)evt->wheel.y));
			return 1;
		}
		else return 0;
	}
	else if (evt->type == SDL_DROPFILE) {      // In case if dropped file
		char *dropped_filedir = evt->drop.file;
		
		SDL_SetClipboardText(dropped_filedir);
		
		SDL_free(dropped_filedir);    // Free dropped_filedir memory
		
		nk_input_key(ctx, NK_KEY_PASTE, 1);
		SDL_Delay(50);
		nk_input_key(ctx, NK_KEY_PASTE, 0);
		
		return 1;
	}
	return 0;
}

NK_API
void nk_sdl_shutdown(gui_obj *gui)
{
	if(gui){
		int i;
		for (i = 0; i < MAX_SCRIPTS; i++){
			if (gui->lua_script[i].L != NULL)
				lua_close(gui->lua_script[i].L);
		}
		
		nk_textedit_free(&(gui->text_edit));
		nk_textedit_free(&(gui->debug_edit));
		
		gui_list_font_free (gui->ui_font_list);
		strpool_term( &gui->file_pool );
		
		//nk_free(gui->ctx);
		free(gui->ctx);
		gui->ctx = NULL;
		//free(gui->font);
		gui->font = NULL;
		
		free(gui->buf);
		gui->buf = NULL;
		free(gui->last);
		gui->last = NULL;
		free(gui);
	}
		
    //memset(&sdl, 0, sizeof(sdl));
}

int gui_start(gui_obj *gui){
	if(!gui) return 0;
	
	gui->ui_font_list = gui_new_font (NULL);
	
	int i = 0, j = 0;
	bmp_color gray = {.r = 100, .g = 100, .b = 100, .a = 255};
	bmp_color hilite = {.r = 255, .g = 0, .b = 255, .a = 125};
	
	gui->ctx = NULL;
	gui->font = NULL;
	gui->buf = NULL;
	gui->last = NULL;
	
	gui->theme = THEME_GREEN;
	gui->cursor = CURSOR_CROSS;
	
	gui->seed = NULL;
	gui->dflt_pat = NULL;
	gui->dflt_lin = NULL;
	gui->extra_lin = NULL;
	
	gui->drawing = NULL;
	gui->element = NULL;
	gui->near_el = NULL;
	
	gui->near_list = NULL;
	gui->near_count = 0;
	gui->sel_idx = 0;
	gui->sel_mode = LIST_TOGGLE;
	gui->sel_type = SEL_INTL;
	gui->sel_count = 0;
	gui->free_sel = 1;
	gui->sel_ent_filter = ~DXF_NONE;
	
	gui->gl_ctx.vert_count = 0;
	gui->gl_ctx.elem_count = 0;
	gui->gl_ctx.verts = NULL;
	gui->gl_ctx.elems = NULL;
	gui->gl_ctx.win_w = 800;
	gui->gl_ctx.win_h = 600;
	gui->gl_ctx.flip_y = 0;
	gui->gl_ctx.fg[0] = 255; gui->gl_ctx.fg[1] = 255; gui->gl_ctx.fg[2] = 255; gui->gl_ctx.fg[3] = 255;
	gui->gl_ctx.bg[0] = 100; gui->gl_ctx.bg[1] = 100; gui->gl_ctx.bg[2] = 100; gui->gl_ctx.bg[3] = 255;
	
	gui->gl_ctx.tex = 0;
	gui->gl_ctx.tex_w = 600;
	gui->gl_ctx.tex_h = 600;
	
	gui->win_x = SDL_WINDOWPOS_CENTERED;
	gui->win_y = SDL_WINDOWPOS_CENTERED;
	gui->win_w = 1120;
	gui->win_h = 600;
	
	gui->next_win_x = 0;
	gui->next_win_y = 0;
	gui->next_win_w = 0;
	gui->next_win_h = 0;
	
	gui->mouse_x = 0;
	gui->mouse_y = 0;
	
	gui->zoom = 20.0;
	gui->ofs_x = -10.0;
	gui->ofs_y = -5.0;
	gui->ofs_z = 0.0;
	gui->prev_zoom = 20.0;
	
	gui->user_x = 0.0;
	gui->user_y = 0.0;
	for (i = 0; i <10; i++){
		gui->step_x[i] =0.0;
		gui->step_y[i] =0.0;
	}
	gui->near_x = 0.0;
	gui->near_y = 0.0;
	gui->bulge = 0.0;
	gui->scale_x = 1.0;
	gui->scale_y = 1.0;
	gui->angle = 0.0;
	gui->txt_h = 1.0;
	gui->rect_w = 0.0;
	
	gui->color_idx = 256;
	gui->lw_idx = 0;
	gui->t_al_v = 0;
	gui->t_al_h = 0;
	gui->layer_idx = 0;
	gui->ltypes_idx = 0;
	gui->t_sty_idx = 0;
	
	gui->step = 0;
	gui->user_flag_x = 0;
	gui->user_flag_y = 0;
	gui->lock_ax_x = 0;
	gui->lock_ax_y = 0;
	gui->user_number = 0;
	gui->keyEnter = 0;
	gui->draw = 0;
	gui->draw_tmp = 0;
	gui->draw_phanton = 0;
	gui->draw_vert = 0;
	gui->vert_idx = -1;
	gui->near_attr = 0;
	gui->text2tag = 0;
	gui->hide_tag = 0;
	gui->en_distance = 0;
	gui->entry_relative = 1;
	gui->rect_polar = 0;
	
	gui->action = NONE;
	gui->modal = SELECT;
	gui->prev_modal = SELECT;
	gui->ev = EV_NONE;
	gui->curr_attr_t = ATRC_END|ATRC_MID|ATRC_QUAD|ATRC_INS|ATRC_NODE;
	
	gui->background = gray;
	gui->hilite = hilite;
	
	gui->svg_curves = NULL;
	gui->svg_bmp = NULL;
	gui->preview_img = NULL;
	
	//gui->b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	//gui->b_icon_sel, gui->b_icon_unsel;
	
	gui->log_msg[0] = 0;
	gui->txt[0] = 0;
	gui->tag[0] = 0;
	gui->long_txt[0] = 0;
	gui->blk_name[0] = 0;
	gui->blk_descr[0] = 0;
	gui->tag_mark[0] = '#'; gui->tag_mark[1] = 0;
	gui->hide_mark[0] = '*'; gui->hide_mark[1] = 0;
	gui->value_mark[0] = '$'; gui->value_mark[1] = 0;
	gui->dflt_value[0] = '?'; gui->dflt_value[1] = 0;
	
	gui->sel_list = NULL;
	gui->phanton = NULL;
	//gui->list_do;
	gui->changed = 0;
	gui->font_list = NULL;
	
	
	gui->hatch_sel = 0;
	
	/* initialize the hatch pattern list */
	gui->hatch_fam_idx = 0;
	gui->hatch_idx = 0;
	gui->hatch_assoc = 0;
	gui->hatch_t_box = 0;
	gui->h_type = HATCH_USER;
	
	gui->user_patt.ang = 45.0;
	gui->user_patt.ox = 0.0; gui->user_patt.oy = 0.0;
	gui->user_patt.dx = 0.0; gui->user_patt.dy = 1;
	gui->user_patt.num_dash = 0;
	gui->user_patt.next = NULL;
	
	strncpy(gui->list_pattern.name, "USER_DEF", DXF_MAX_CHARS);
	strncpy(gui->list_pattern.descr, "User definied simple pattern", DXF_MAX_CHARS);
	gui->list_pattern.num_lines = 1;
	gui->list_pattern.lines = &(gui->user_patt);
	gui->list_pattern.next = NULL;
	
	strncpy(gui->hatch_fam.name, "USER_DEF", DXF_MAX_CHARS);
	strncpy(gui->hatch_fam.descr, "User definied simple pattern", DXF_MAX_CHARS);
	gui->hatch_fam.list = &(gui->list_pattern);
	gui->hatch_fam.next = NULL;
	gui->end_fam = &(gui->hatch_fam);
	
	//gui->hatch_fam.next = dxf_hatch_family("Standard", "Internal standard pattern library", (char*) h_pattern_lib_dflt () );
	//if(gui->hatch_fam.next) gui->end_fam = gui->hatch_fam.next;
	
	gui->patt_scale = 1.0;
	gui->patt_ang = 0.0;
	gui->patt_name[0] = 0;
	gui->patt_descr[0] = 0;
	gui->h_fam_name[0] = 0;
	gui->h_fam_descr[0] = 0;
	
	gui->dflt_fonts_path[0] = 0;
	
	gui->show_open = 0;
	gui->show_save = 0;
	gui->show_app_about = 0;
	gui->show_app_file = 0;
	gui->path_ok = 0;
	gui->show_info = 0;
	gui->show_script = 0;
	gui->show_print = 0;
	gui->show_export = 0;
	
	gui->progress = 0;
	gui->hist_new = 0;
	gui->show_lay_mng = 0;
	gui->show_color_pick = 0;
	gui->show_tstyles_mng = 0;
	gui->show_blk_mng = 0;
	gui->show_ltyp_mng = 0;
	gui->show_dim_mng = 0;
	gui->show_hatch_mng = 0;
	gui->show_config = 0;
	
	gui->curr_path[0] = 0;
	gui->base_dir[0] = 0;
	gui->dwg_file[0] = 0;
	gui->dwg_dir[0] = 0;
	gui->pref_path[0] = 0;
	
	gui->paper_fam = 0;
	gui->sel_paper = 6;
	
	gui->keep_orig = 0;
	
	gui->closed = 0;
	gui->proportional = 1;
	
	gui->sp_degree = 4;
	gui->spline_mode = SP_CTRL;
	
	gui->scale_mode = SCALE_FACTOR;
	gui->rot_mode = ROT_ANGLE;
	
	gui->expl_mode = EXPL_INS;
	
	gui->el_mode = EL_FULL;
	gui->o_view = O_TOP;
	
	gui->image_w = 0;
	gui->image_h = 0;
	
	for (i = 0; i < MAX_SCRIPTS; i++){
		memset (&gui->lua_script[i], 0, sizeof(struct script_obj));
		/*gui->lua_script[i].L = NULL;
		gui->lua_script[i].T = NULL;
		gui->lua_script[i].active = 0;
		gui->lua_script[i].dynamic = 0;
		gui->lua_script[i].status = LUA_OK;
		gui->lua_script[i].path[0] = 0; */
		
		gui->lua_script[i].timeout = 10.0;
		/*
		gui->lua_script[i].win[0] = 0;
		gui->lua_script[i].win_title[0] = 0;
		
		gui->lua_script[i].dyn_func[0] = 0;
		*/
	}
	
	//strncpy(gui->curr_script, "D:\\documentos\\cadzinho\\lua\\test.lua", MAX_PATH_LEN - 1);
	gui->curr_script[0] = 0;
	gui->image_path[0] = 0;
	
	/* ----------- init font list ------------------- */
	gui->font_list = list_new(NULL, PRG_LIFE);
	
	/* load internal classic CAD shape fonts */
	add_shp_font_list(gui->font_list, "complex.shx", (char *) shp_font_complex());
	add_shp_font_list(gui->font_list, "gothice.shx", (char *) shp_font_gothice());
	add_shp_font_list(gui->font_list, "gothicg.shx", (char *) shp_font_gothicg());
	add_shp_font_list(gui->font_list, "gothici.shx", (char *) shp_font_gothici());
	add_shp_font_list(gui->font_list, "isocp.shx", (char *) shp_font_isocp());
	add_shp_font_list(gui->font_list, "italic.shx", (char *) shp_font_italic());
	add_shp_font_list(gui->font_list, "romanc.shx", (char *) shp_font_romanc());
	add_shp_font_list(gui->font_list, "romand.shx", (char *) shp_font_romand());
	add_shp_font_list(gui->font_list, "romans.shx", (char *) shp_font_romans());
	add_shp_font_list(gui->font_list, "romant.shx", (char *) shp_font_romant());
	add_shp_font_list(gui->font_list, "scriptc.shx", (char *) shp_font_scriptc());
	add_shp_font_list(gui->font_list, "scripts.shx", (char *) shp_font_scripts());
	add_shp_font_list(gui->font_list, "simplex.shx", (char *) shp_font_simplex());
	add_shp_font_list(gui->font_list, "txt.shx", (char *) shp_font_txt());
	
	/* set "txt.shx" as default font (substitute other missing fonts) */
	gui->dflt_font = get_font_list(gui->font_list, "txt.shx");
		
	/* load internal shape library "ltypeshp", for use in complex linetypes*/
	add_shp_font_list(gui->font_list, "ltypeshp.shx", (char *) shp_font_ltypeshp());
	
	/* ui default font */
	struct tfont *ui_font = get_font_list(gui->font_list, "txt.shx");
	gui->ui_font.userdata = nk_handle_ptr(ui_font);
	gui->ui_font.height = 10.0;
	
	/* ----------- init history ------------------- */
	for (i = 0; i < DRWG_HIST_MAX; i++)
		gui->drwg_hist[i][0] = 0;
	gui->drwg_hist_size = 0;
	gui->drwg_hist_pos = 0;
	gui->drwg_hist_wr = 0;
	gui->drwg_hist_head = 0;
	
	/* ----------- init recent drawing files ------------------- */
	gui->recent_drwg = list_new(NULL, PRG_LIFE);
	/*init file pool */
	strpool_config_t str_pool_conf = strpool_default_config;
        //str_pool_conf.ignore_case = true;
	str_pool_conf.counter_bits = 16;
	str_pool_conf.index_bits = 16;
        strpool_init( &gui->file_pool, &str_pool_conf );
	
	
	
	gui->num_brk_pts = 0;
	
	memset(gui->blank_tex, 0, 4*20*600);
	
	
	gui->alpha = 0.0;
	gui->beta = 0.0;
	gui->gamma = 0.0;
	
	gui->drwg_view[0][0] = 1.0;
	gui->drwg_view[0][1] = 0.0;
	gui->drwg_view[0][2] = 0.0;
	gui->drwg_view[0][3] = 0.0;
	gui->drwg_view[1][0] = 0.0;
	gui->drwg_view[1][1] = 1.0;
	gui->drwg_view[1][2] = 0.0;
	gui->drwg_view[1][3] = 0.0;
	gui->drwg_view[2][0] = 0.0;
	gui->drwg_view[2][1] = 0.0;
	gui->drwg_view[2][2] = 1.0;
	gui->drwg_view[2][3] = 0.0;
	gui->drwg_view[3][0] = 0.0;
	gui->drwg_view[3][1] = 0.0;
	gui->drwg_view[3][2] = 0.0;
	gui->drwg_view[3][3] = 1.0;
	
	gui->drwg_view_i[0][0] = 1.0;
	gui->drwg_view_i[0][1] = 0.0;
	gui->drwg_view_i[0][2] = 0.0;
	gui->drwg_view_i[0][3] = 0.0;
	gui->drwg_view_i[1][0] = 0.0;
	gui->drwg_view_i[1][1] = 1.0;
	gui->drwg_view_i[1][2] = 0.0;
	gui->drwg_view_i[1][3] = 0.0;
	gui->drwg_view_i[2][0] = 0.0;
	gui->drwg_view_i[2][1] = 0.0;
	gui->drwg_view_i[2][2] = 1.0;
	gui->drwg_view_i[2][3] = 0.0;
	gui->drwg_view_i[3][0] = 0.0;
	gui->drwg_view_i[3][1] = 0.0;
	gui->drwg_view_i[3][2] = 0.0;
	gui->drwg_view_i[3][3] = 1.0;
	
	gui->discard_changes = 0;
	gui->desired_action = NONE;
	gui->hist_action = HIST_NONE;
	
	return 1;
}

int gui_tstyle(gui_obj *gui){
	if(!gui) return 0;
	
	int i = 0;
	
	int num_tstyles = gui->drawing->num_tstyles;
	dxf_tstyle *t_sty = gui->drawing->text_styles;
	struct tfont * font;
		
	for (i = 0; i < num_tstyles; i++){
		if(font = add_font_list(gui->font_list, t_sty[i].file, gui->dwg_dir)){
			t_sty[i].font = font;
			//if (font->type == FONT_SHP) shp_font_print((shp_typ *) font->data);
		}
		else if(font = add_font_list(gui->font_list, t_sty[i].file, gui->dflt_fonts_path)){
			t_sty[i].font = font;
			//if (font->type == FONT_SHP) shp_font_print((shp_typ *) font->data);
		}
		else{
			t_sty[i].font = gui->dflt_font;
			strncpy(t_sty[i].subst_file, gui->dflt_font->name, DXF_MAX_CHARS);
		}
		
	}
	
	return 1;
}

void gui_simple_select(gui_obj *gui){
	enum dxf_graph ent_type = DXF_NONE;
	if (gui->ev & EV_ENTER){ /* user hit an enter point */
		if (gui->element)
			/* select an object if cursor is over */
			list_modify(gui->sel_list, gui->element, LIST_TOGGLE, SEL_LIFE);
		
	}
	else if (gui->ev & EV_CANCEL){
		gui->element = NULL;
		list_node *list_el = NULL;
		if (gui->near_count > 0) {
			/* try to switch to another object that is overlaid */
			gui->sel_idx++;
			if (gui->sel_idx >= gui->near_count)
				gui->sel_idx = 0; /* back to list begin */
			
			/* examine the list of objects near cursor */
			int i = 0;
			list_el = gui->near_list->next;
			while (list_el){
				if (gui->sel_idx == i)
					/* show the candidate object */
					gui->element = (dxf_node *)list_el->data;
				list_el = list_el->next;
				i++;
			}
		}
		else {
			/* if not any hovered object, clear selection */
			gui_default_modal(gui);
		}
		gui->draw = 1;
	}
	if (gui->ev & EV_MOTION){ /* cursor motion */
		if (gui->sel_idx >= gui->near_count)
			gui->sel_idx = 0;
		
		/* examine the list of objects near cursor */
		gui->element = NULL;
		list_node *list_el = NULL;
		if (gui->near_count > 0) {
			int i = 0;
			list_el = gui->near_list->next;
			while (list_el){
				ent_type = dxf_ident_ent_type ((dxf_node *)list_el->data);
				if (ent_type & gui->sel_ent_filter){
					if (gui->sel_idx == i){
						/* show the candidate object */
						gui->element = (dxf_node *)list_el->data;
						break;
					}
					i++;
				}
				list_el = list_el->next;
				
			}
		}
		gui->draw = 1;
	}
}

int draw_cursor_gl(gui_obj *gui, int x, int y, enum Cursor_type type) {
	if (!gui) return 0;
	
	struct ogl *gl_ctx = &(gui->gl_ctx);
	
	/* init opengl context */
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	/* set the color to contrast with background*/
	gl_ctx->fg[0] = gui->background.r ^ 255;
	gl_ctx->fg[1] = gui->background.g ^ 255;
	gl_ctx->fg[2] = gui->background.b ^ 255;
	int alpha = 0;
	if (gui->background.r > 90 && gui->background.r <125) alpha += 30;
	if (gui->background.g > 90 && gui->background.g <125) alpha += 30;
	if (gui->background.b > 90 && gui->background.b <125) alpha += 30;
	gl_ctx->fg[3] = 100 + alpha;
	
	/* draw cursor */
	if (type == CURSOR_SQUARE){
		draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y+5, 0}, 3);
		draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 3);
		draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x+5, y+5, 0}, 3);
		draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-5, y+5, 0}, 3);
	}
	else if (type == CURSOR_X){
		draw_gl_line (gl_ctx, (int []){x-10, y-10, 0}, (int []){x+10, y+10, 0}, 3);
		draw_gl_line (gl_ctx, (int []){x-10, y+10, 0}, (int []){x+10, y-10, 0}, 3);
	}
	else if (type == CURSOR_CIRCLE){
		int x0, y0, x1, y1, i;
		double major_ax, minor_ax;
		struct edge edges[20];
		
		major_ax = 5.0;
		minor_ax = 5.0;
		
		x0 = 0.5 + x + major_ax;
		y0 = 0.5 + y;
		
		for (i = 1; i < 20; i++){
			x1 = 0.5 + x + major_ax * cos( 0.1 * M_PI * i );
			y1 = 0.5 + y + minor_ax * sin( 0.1 * M_PI * i );
			edges[i-1] = (struct edge){x0, y0, 0, x1, y1, 0};
			x0 = x1;
			y0 = y1;
		}
		x1 = 0.5 + x + major_ax;
		y1 = 0.5 + y;
		edges[19] = (struct edge){x0, y0, 0, x1, y1, 0};
		
		draw_gl_polygon (gl_ctx, 20, edges);
	}
	else{
		draw_gl_line (gl_ctx, (int []){-gl_ctx->win_w, y, 0}, (int []){gl_ctx->win_w*2,y, 0}, 3);
		draw_gl_line (gl_ctx, (int []){x, -gl_ctx->win_h, 0}, (int []){x, gl_ctx->win_h*2, 0}, 3);
		
		draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y+5, 0}, 1);
		draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 1);
		draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
		draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-5, y+5, 0}, 1);
	}
	
	draw_gl (gl_ctx, 0);
	return 1;
}

int draw_attractor_gl(gui_obj *gui, enum attract_type type, int x, int y, bmp_color color){
	if (!gui) return 0;
	
	struct ogl *gl_ctx = &(gui->gl_ctx);
	
	/* init opengl context */
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	/* set the color */
	gl_ctx->fg[0] = color.r;
	gl_ctx->fg[1] = color.g;
	gl_ctx->fg[2] = color.b;
	gl_ctx->fg[3] = color.a;
	
	/* draw attractor mark*/
	if (type == ATRC_NONE){
		return 0;
	}
	switch (type){
		case ATRC_END:
			/* draw square */
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-5, y+5, 0}, 1);
			break;
		case ATRC_MID:
			/* draw triangle */
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x, y+5, 0}, 1);
			break;
		case ATRC_CENTER:
			/* draw circle */
			draw_gl_line (gl_ctx, (int []){x-7, y, 0}, (int []){x-5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x, y+7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y+7, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y+5, 0}, (int []){x+7, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+7, y, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x, y-7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y-7, 0}, (int []){x-5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-7, y, 0}, 1);
			break;
		case ATRC_OCENTER:
			/* draw a *star */
			draw_gl_line (gl_ctx, (int []){x, y-5, 0}, (int []){x, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y, 0}, (int []){x+5, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y-5, 0}, 1);
			break;
		case ATRC_NODE:
			/* draw circle with x*/
			draw_gl_line (gl_ctx, (int []){x-7, y, 0}, (int []){x-5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x, y+7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y+7, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y+5, 0}, (int []){x+7, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+7, y, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x, y-7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y-7, 0}, (int []){x-5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-7, y, 0}, 1);
			
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y-5, 0}, 1);
			break;
		case ATRC_QUAD:
			/* draw diamond */
			draw_gl_line (gl_ctx, (int []){x-7, y, 0}, (int []){x, y+7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y+7, 0}, (int []){x+7, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+7, y, 0}, (int []){x, y-7, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y-7, 0}, (int []){x-7, y, 0}, 1);
			break;
		case ATRC_INTER:
			/* draw x */
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y+5, 0}, 3);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y-5, 0}, 3);
			break;
		case ATRC_EXT:
			draw_gl_line (gl_ctx, (int []){x-7, y, 0}, (int []){x-2, y, 0}, 3);
			draw_gl_line (gl_ctx, (int []){x, y, 0}, (int []){x+3, y, 0}, 3);
			draw_gl_line (gl_ctx, (int []){x+5, y, 0}, (int []){x+7, y, 0}, 3);
			break;
		case ATRC_PERP:
			/* draw square */
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y, 0}, (int []){x, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y-5, 0}, (int []){x, y, 0}, 1);
			break;
		case ATRC_INS:
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+1, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+1, y+5, 0}, (int []){x+1, y+1, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+1, y+1, 0}, (int []){x+5, y+1, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y+1, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x-1, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-1, y-5, 0}, (int []){x-1, y-1, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-1, y-1, 0}, (int []){x-5, y-1, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-1, 0}, (int []){x-5, y+5, 0}, 1);
			break;
		case ATRC_TAN:
			draw_gl_line (gl_ctx, (int []){x-6, y+6, 0}, (int []){x+6, y+6, 0}, 1);
			//bmp_line(img, x-5, y+5, x+5, y+5);
			//bmp_circle(img, x, y, 5);
			draw_gl_line (gl_ctx, (int []){x-5, y, 0}, (int []){x-3, y+3, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-3, y+3, 0}, (int []){x, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y+5, 0}, (int []){x+3, y+3, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+3, y+3, 0}, (int []){x+5, y, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y, 0}, (int []){x+3, y-3, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+3, y-3, 0}, (int []){x, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x, y-5, 0}, (int []){x-3, y-3, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-3, y-3, 0}, (int []){x-5, y, 0}, 1);
			break;
		case ATRC_PAR:
			/* draw two lines */
			draw_gl_line (gl_ctx, (int []){x-5, y-1, 0}, (int []){x+1, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-1, y-5, 0}, (int []){x+5, y+1, 0}, 1);
			break;
		case ATRC_CTRL:
			/* draw a cross */
			draw_gl_line (gl_ctx, (int []){x, y-5, 0}, (int []){x, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y, 0}, (int []){x+5, y, 0}, 1);
			break;
		case ATRC_AINT:
			/* draw square with x */
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x-5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y+5, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x+5, y-5, 0}, (int []){x-5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y-5, 0}, 1);
			break;
		case ATRC_ANY:
			/* draw a Hourglass */
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y+5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y-5, 0}, 1);
			draw_gl_line (gl_ctx, (int []){x-5, y+5, 0}, (int []){x+5, y+5, 0}, 2);
			draw_gl_line (gl_ctx, (int []){x-5, y-5, 0}, (int []){x+5, y-5, 0}, 2);
			break;
	}
	
	draw_gl (gl_ctx, 0);
	return 1;
}

int gui_draw_vert_rect_gl(gui_obj *gui, double x, double y, bmp_color color){
	
	if (!gui) return 0;
	
	struct ogl *gl_ctx = &(gui->gl_ctx);
	
	/* init opengl context */
	if (gl_ctx->elems == NULL){
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	/* set the color */
	gl_ctx->fg[0] = color.r;
	gl_ctx->fg[1] = color.g;
	gl_ctx->fg[2] = color.b;
	gl_ctx->fg[3] = color.a;
	
	/* convert entities coordinates to screen coordinates */
	int x1 = (int) round((x - gui->ofs_x) * gui->zoom);
	int y1 = (int) round((y - gui->ofs_y) * gui->zoom);
	
	draw_gl_quad (gl_ctx, (int []){x1-7, y1+7, 0}, (int []){x1-7, y1-7, 0}, (int []){x1+7, y1+7, 0}, (int []){x1+7, y1-7, 0});
	
	draw_gl (gl_ctx, 0);
	return 1;
}

void gui_draw_vert_gl(gui_obj *gui, dxf_node *obj){
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int pt = 0;
	enum dxf_graph ent_type = DXF_NONE;
	double x = 0.0, y = 0.0, z = 0.0;
	double point[3];
	
	int ellip = 0;
	
	if (!obj) return;
	if (obj->type != DXF_ENT) return;
	
	int vert_count = 0;
	
	stop = obj;
	ent_type =  dxf_ident_ent_type (obj);
	
	if ((ent_type != DXF_HATCH) && (obj->obj.content)){
		current = obj->obj.content->next;
		prev = current;
	}
	else if ((ent_type == DXF_HATCH) && (obj->obj.content)){
		current = dxf_find_attr_i(obj, 91, 0);
		if (current){
			current = current->next;
			prev = current;
		}
		dxf_node *end_bond = dxf_find_attr_i(obj, 75, 0);
		if (end_bond) stop = end_bond;
	}
	
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/*
			point[0] = of_x;
			point[1] = of_y;
			point[2] = of_z;
			
			dxf_get_extru(obj, point);
			
			ofs_x = point[0];
			ofs_y = point[1];
			ofs_z = point[2];
			*/
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				/* get the vertex coordinate set */
				if (current->value.group == 10){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 20))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 30))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			if (ent_type == DXF_HATCH){
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
				}
			}
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH || ent_type == DXF_ATTRIB){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 12){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 22))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 32))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_DIMENSION){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 14){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 24))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 34))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 15){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 25))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 35))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				/* get the vertex coordinate set */
				if (current->value.group == 16){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 26))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 36))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
		}
		if (pt){
			pt = 0;
			if(vert_count == gui->vert_idx) 
				gui_draw_vert_rect_gl(gui, x, y, dxf_colors[225]);
			else gui_draw_vert_rect_gl(gui, x, y, dxf_colors[224]);
			
			vert_count++;
		}
		
		if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
			current = NULL;
			break;
		}
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				if(prev == stop){
					current = NULL;
					break;
				}
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
}
