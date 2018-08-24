#include "gui.h"


void set_style(struct nk_context *ctx, enum theme theme){
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
        nk_style_from_table(ctx, table);
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
        nk_style_from_table(ctx, table);
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
        nk_style_from_table(ctx, table);
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
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_ZE) {
        table[NK_COLOR_TEXT] = nk_rgba(250, 250, 250, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 71, 58, 215);
        table[NK_COLOR_HEADER] = nk_rgba(52, 57, 52, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 112, 54, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(71, 161, 80, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(89, 201, 100, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(50, 61, 50, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(46, 57, 46, 255);
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
        nk_style_from_table(ctx, table);
    } else {
        nk_style_default(ctx);
    }
}

float nk_user_font_get_text_width(nk_handle handle, float height, const char *text, int len){
	struct font_obj *font = (struct font_obj *)handle.ptr;
	if ((text!= NULL) && (font!=NULL)) {
		
	
		/* We must copy into a new buffer with exact length null-terminated
		as nuklear uses variable size buffers and shx_fonts routines doesn't
		accept a length, it infers length from null-termination */
		char txt_cpy[len+2];
		strncpy((char*)&txt_cpy, text, len);
		//txt_cpy[len - 1] = ' ';
		txt_cpy[len] = '\0';
		
		
		nk_rune str_uni[255];
		char str[255];
		int glyph_size, char_size, pos = 0;
		
		char *curr = 0, *curr_pos =0;
		
		curr = (char *)txt_cpy;
		curr_pos = str;
		pos = 0;
		
		while ((*curr != 0) && (pos < 254)){
		
			glyph_size = nk_utf_decode(curr, str_uni, 2);
			if (glyph_size){
				char_size = wctomb(curr_pos, (wchar_t)str_uni[0]);
				curr += glyph_size;
				pos += char_size;
				curr_pos += char_size;
			}
			else {
				curr = 0;
			}
		}
		
		if(pos<255){
			str[pos] = 0;
		}
		else{
			str[254] = 0;
		}
		
		double txt_w;
		graph_obj *curr_graph = shx_font_parse(font->shx_font, 1, str, &txt_w);
		if (curr_graph){
			return (float) font->scale * txt_w;
		}
	}
	return 0;
}

bmp_color nk_to_bmp_color(struct nk_color color){
	bmp_color ret_color = {.r = color.r, .g = color.g, .b = color.b, .a = color.a };
	return ret_color;
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

NK_API void nk_sdl_render(gui_obj *gui, bmp_img *img){
	const struct nk_command *cmd = NULL;
	bmp_color color = {.r = 255, .g = 255, .b =255, .a = 255};
	int iter = 0;
	
	static int one_time = 0;
	if (!one_time){
		//one_time =1;
	}
	
	if ((img != NULL) && (gui != NULL)){
		
		/* initialize the image with a solid line pattern */
		img->patt_i = 0;
		img->pix_count = 0;
		img->patt_size = 1;
		img->pattern[0] = 1;
		img->zero_tl =1;

		nk_foreach(cmd, gui->ctx){
			
			/* break the loop, if more then 10000 iterations */
			iter += 1;
			if (iter > 10000){
				printf("error render\n");
				break;
			}
			
			switch (cmd->type) {
				case NK_COMMAND_NOP: break;
				
				case NK_COMMAND_SCISSOR: {
					const struct nk_command_scissor *s =(const struct nk_command_scissor*)cmd;
					img->clip_x = (unsigned int)s->x;
					img->clip_y = (unsigned int)s->y;
					img->clip_w = (unsigned int)s->w;
					img->clip_h = (unsigned int)s->h;
				} break;
				
				case NK_COMMAND_LINE: {
					const struct nk_command_line *l = (const struct nk_command_line *)cmd;
					color = nk_to_bmp_color(l->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = (unsigned int) l->line_thickness;
					
					bmp_line(img, l->begin.x, l->begin.y, l->end.x,l->end.y);
				} break;
				
				case NK_COMMAND_RECT: {
					const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
					color = nk_to_bmp_color(r->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = (unsigned int) r->line_thickness;
					
					int x0, y0, x1, y1, i, cx, cy;
					
					bmp_line(img, r->x + r->rounding, r->y, r->x + r->w -r->rounding, r->y);
					x0 =  r->x + r->w - r->rounding;
					cx = x0;
					y0 = r->y;
					cy = r->y + r->rounding;
					for (i=13; i <= 16; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x + r->w, r->y + r->rounding, r->x + r->w, r->y + r->h - r->rounding);
					cx = r->x + r->w - r->rounding;
					x0 = r->x + r->w;
					y0 = r->y + r->h - r->rounding;
					cy = y0;
					for (i=1; i <= 4; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x + r->w - r->rounding, r->y + r->h, r->x + r->rounding, r->y + r->h);
					x0 = r->x + r->rounding;
					cx = x0;
					y0 = r->y + r->h;
					cy = r->y + r->h - r->rounding;
					for (i=5; i <= 8; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
					bmp_line(img, r->x, r->y + r->h - r->rounding, r->x, r->y + r->rounding);
					cx = r->x + r->rounding;
					x0 = r->x;
					y0 = r->y + r->rounding;
					cy = y0;
					for (i=9; i <= 12; i++){
						x1 = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						y1 = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
						bmp_line(img, x0, y0, x1, y1);
						x0 = x1;
						y0 = y1;
					}
				} break;
				
				case NK_COMMAND_RECT_FILLED: {
					const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
					
					int vert_x[20], vert_y[20];
					int i, cx, cy;
					
					vert_x[0] = r->x + r->rounding;
					vert_x[1] = r->x + r->w - r->rounding;
					
					cx =  r->x + r->w - r->rounding;
					cy = r->y + r->rounding;
					for (i=13; i < 16; i++){
						vert_x[i-11] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i-11] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[5] = r->x + r->w;
					vert_x[6] = r->x + r->w;
					
					cx = r->x + r->w - r->rounding;
					cy = r->y + r->h - r->rounding;
					for (i=1; i < 4; i++){
						vert_x[i+6] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+6] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[10] = r->x + r->w - r->rounding;
					vert_x[11] = r->x + r->rounding;
					
					cx = r->x + r->rounding;
					cy = r->y + r->h - r->rounding;
					for (i=5; i < 8; i++){
						vert_x[i+7] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+7] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_x[15] = r->x;
					vert_x[16] = r->x;
					
					cx = r->x + r->rounding;
					cy = r->y + r->rounding;
					for (i=9; i < 12; i++){
						vert_x[i+8] = cx +  round((double)r->rounding * cos(2 * M_PI * i  / 16.0));
						vert_y[i+8] = cy +  round((double)r->rounding * sin(2 * M_PI * i  / 16.0));
					}
					
					vert_y[0] = r->y;
					vert_y[1] = r->y;
					
					vert_y[5] = r->y + r->rounding;
					vert_y[6] = r->y + r->h - r->rounding;
					
					vert_y[10] = r->y + r->h;
					vert_y[11] = r->y + r->h;
					
					vert_y[15] = r->y + r->h - r->rounding;
					vert_y[16] = r->y + r->rounding;
					
					color = nk_to_bmp_color(r->color);
					/*change the color */
					img->frg = color;
					
					bmp_poly_fill(img, 20, vert_x, vert_y, NULL);
				} break;
				
				case NK_COMMAND_CIRCLE: {
					const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
					color = nk_to_bmp_color(c->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = c->line_thickness;
					int xr = c->w/2;
					
					bmp_circle(img, c->x + xr, c->y + xr, xr);
				} break;
				
				case NK_COMMAND_CIRCLE_FILLED: {
					const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
					color = nk_to_bmp_color(c->color);
					/*change the color */
					img->frg = color;
					int xr = c->w/2;
					
					bmp_circle_fill(img, c->x + xr, c->y + xr, xr);
				} break;
				
				case NK_COMMAND_TRIANGLE: {
					const struct nk_command_triangle*t = (const struct nk_command_triangle*)cmd;
					color = nk_to_bmp_color(t->color);
					/*change the color */
					img->frg = color;
					/*change tickness */
					img->tick = t->line_thickness;
					bmp_line(img, t->a.x, t->a.y, t->b.x, t->b.y);
					bmp_line(img, t->b.x, t->b.y, t->c.x, t->c.y);
					bmp_line(img, t->c.x, t->c.y, t->a.x, t->a.y);
				} break;
				
				case NK_COMMAND_TRIANGLE_FILLED: {
					const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
					int vert_x[3] = {t->a.x, t->b.x, t->c.x};
					int vert_y[3] = {t->a.y, t->b.y, t->c.y};
					
					color = nk_to_bmp_color(t->color);
					/*change the color */
					img->frg = color;
					
					bmp_poly_fill(img, 3, vert_x, vert_y, NULL);
				} break;
				
				case NK_COMMAND_POLYGON: {
					const struct nk_command_polygon *p = (const struct nk_command_polygon*)cmd;
					color = nk_to_bmp_color(p->color);
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
				} break;
				
				case NK_COMMAND_POLYGON_FILLED: {
					const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
					color = nk_to_bmp_color(p->color);
					//int i;
					//float vertices[p->point_count * 2];
					// for (i = 0; i < p->point_count; i++) {
					//    vertices[i*2] = p->points[i].x;
					//     vertices[(i*2) + 1] = p->points[i].y;
					// }
					//  al_draw_filled_polygon((const float*)&vertices, (int)p->point_count, color);
					//printf("fill_polygon ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_POLYLINE: {
					const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
					color = nk_to_bmp_color(p->color);
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
				} break;
				
				case NK_COMMAND_TEXT: {
					const struct nk_command_text *t = (const struct nk_command_text*)cmd;
					color = nk_to_bmp_color(t->foreground);
					nk_rune str_uni[255];
					char str[255];
					int glyph_size, char_size, pos = 0;
					
					char *curr = 0, *curr_pos =0;
					
					curr = (char *)t->string;
					curr_pos = str;
					pos = 0;
					
					while ((*curr != 0) && (pos < 254)){
					
						glyph_size = nk_utf_decode(curr, str_uni, 10);
						if (glyph_size){
							char_size = wctomb(curr_pos, (wchar_t)str_uni[0]);
							curr += glyph_size;
							pos += char_size;
							curr_pos += char_size;
						}
						else {
							curr = 0;
						}
					}
					
					if(pos<255){
						str[pos] = 0;
					}
					else{
						str[254] = 0;
					}
					
					struct font_obj *font = (struct font_obj *)t->font->userdata.ptr;
					graph_obj *curr_graph = shx_font_parse(font->shx_font, 1, (const char*)str, NULL);
					/*change the color */
					if(curr_graph){
						curr_graph->color = color;
					}

					/* apply the scales, offsets and rotation to graphs */
					graph_modify(curr_graph, t->x, t->y + t->font->height, font->scale, -font->scale, 0);
					graph_draw_aa(curr_graph, img, 0.0, 0.0, 1.0);
				} break;
				
				case NK_COMMAND_CURVE: {
					const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
					color = nk_to_bmp_color(q->color);
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
				} break;
				
				case NK_COMMAND_ARC: {
					const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
					color = nk_to_bmp_color(a->color);
					//    al_draw_arc((float)a->cx, (float)a->cy, (float)a->r, a->a[0],
					//       a->a[1], color, (float)a->line_thickness);
					//printf("arc ");//------------------------------------teste
				} break;
				
				case NK_COMMAND_RECT_MULTI_COLOR: {
					if (!one_time){
						one_time =1;
						//printf("multi_c ");//------------------------------------teste
					}
				} break;
				
				case NK_COMMAND_IMAGE: {
					const struct nk_command_image *i = (struct nk_command_image *)cmd;
					if (i->h > 0 && i->w > 0){
						bmp_img *w_img = (bmp_img *)i->img.handle.ptr;
						if (w_img){
							w_img->zero_tl = 1;
							bmp_copy(w_img, img, i->x, i->y);
							w_img->zero_tl = 0;
						}
					}
				} break;
				
				case NK_COMMAND_ARC_FILLED: {
					
				} break;
				
				default: break;
			}
		}
		/* reset image parameters */
		img->zero_tl = 0;
		img->clip_x = 0;
		img->clip_y = 0;
		img->clip_w = img->width;
		img->clip_h = img->height;
	}
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

NK_API int nk_sdl_init(gui_obj* gui, struct nk_user_font *font){
	
	//gui_obj *gui = malloc(sizeof(gui_obj));
	
	if (gui){
		gui->ctx = malloc(sizeof(struct nk_context));
		gui->font = font;
		gui->buf = calloc(1,FIXED_MEM);
		gui->last = calloc(1,FIXED_MEM);
		if((gui->ctx == NULL) || (gui->font == NULL) || (gui->buf == NULL) || (gui->last == NULL)){
			return 0;
		}
	}
	else return 0;
	
	
	
	//nk_init_default(gui->ctx, font);
	nk_init_fixed(gui->ctx, gui->buf, FIXED_MEM, font);
	gui->ctx->clip.copy = nk_sdl_clipbard_copy;
	gui->ctx->clip.paste = nk_sdl_clipbard_paste;
	gui->ctx->clip.userdata = nk_handle_ptr(0);
	
	nk_style_set_font(gui->ctx, font);
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
		const Uint8* state = SDL_GetKeyboardState(0);
		SDL_Keycode sym = evt->key.keysym.sym;
		
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
			nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_r)
			nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_c)
			nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
		else if ((sym == SDLK_v) && (state[SDL_SCANCODE_LCTRL])){
			nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
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
			nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_b)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_e)
			nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
		else if (sym == SDLK_UP)
			nk_input_key(ctx, NK_KEY_UP, down);
		else if (sym == SDLK_DOWN)
			nk_input_key(ctx, NK_KEY_DOWN, down);
		else if (sym == SDLK_LEFT) {
			if (state[SDL_SCANCODE_LCTRL])
				nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
			else nk_input_key(ctx, NK_KEY_LEFT, down);
		} 
		else if (sym == SDLK_RIGHT) {
			if (state[SDL_SCANCODE_LCTRL])
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
		//SDL_Delay(100);
		nk_input_key(ctx, NK_KEY_PASTE, 0);
		
		return 1;
	}
	return 0;
}

NK_API
void nk_sdl_shutdown(gui_obj *gui)
{
	if(gui){
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
	
	int i = 0;
	bmp_color gray = {.r = 100, .g = 100, .b = 100, .a = 255};
	
	gui->ctx = NULL;
	gui->font = NULL;
	gui->buf = NULL;
	gui->last = NULL;
	
	gui->drawing = NULL;
	gui->element = NULL;
	gui->near_el = NULL;
	
	gui->main_w = 2048;
	gui->main_h = 2048;
	
	gui->win_w = 1200;
	gui->win_h = 710;
	
	gui->next_win_x = 0;
	gui->next_win_y = 0;
	gui->next_win_w = 0;
	gui->next_win_h = 0;
	
	gui->mouse_x = 0;
	gui->mouse_y = 0;
	
	gui->zoom = 20.0;
	gui->ofs_x = -11.0;
	gui->ofs_y = -71.0;
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
	gui->scale = 1.0;
	gui->txt_h = 1.0;
	
	gui->color_idx = 256;
	gui->lw_idx = 0;
	gui->t_al_v = 0;
	gui->t_al_h = 0;
	gui->layer_idx = 0;
	gui->ltypes_idx = 0;
	
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
	gui->near_attr = 0;
	gui->text2tag = 0;
	gui->en_distance = 0;
	gui->entry_relative = 1;
	
	gui->action = NONE;
	gui->modal = SELECT;
	gui->prev_modal = SELECT;
	gui->ev = EV_NONE;
	gui->curr_attr_t = ATRC_END|ATRC_MID|ATRC_QUAD;
	
	gui->background = gray;
	
	gui->svg_curves = NULL;
	gui->svg_bmp = NULL;
	gui->preview_img = NULL;
	
	//gui->b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	//gui->b_icon_sel, gui->b_icon_unsel;
	
	gui->log_msg[0] = 0;
	gui->txt[0] = 0;
	gui->blk_name[0] = 0;
	gui->tag_mark[0] = 0;
	
	gui->sel_list = NULL;
	gui->phanton = NULL;
	//gui->list_do;
	
	
	/* initialize the hatch pattern list */
	gui->hatch_fam_idx = 0;
	gui->hatch_idx = 0;
	gui->hatch_solid = 0;
	gui->hatch_assoc = 0;
	gui->hatch_user = 1;
	gui->hatch_predef = 0;
	
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
	
	gui->hatch_fam.next = dxf_hatch_family("Standard", "Internal standard pattern library", (char*)std_h_pat);
	if(gui->hatch_fam.next) gui->end_fam = gui->hatch_fam.next;
	//dxf_parse_patt((char*)acadiso_pat, &(gui->list_pattern));
	
	gui->patt_scale = 1.0;
	gui->patt_ang = 0.0;
	gui->patt_name[0] = 0;
	gui->patt_descr[0] = 0;
	gui->h_fam_name[0] = 0;
	gui->h_fam_descr[0] = 0;
	
	return 1;
}

//#endif
