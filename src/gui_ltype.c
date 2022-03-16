#include "gui_ltype.h"
#include "graph.h"
static int ltype_cpy (dxf_ltype *dest, dxf_ltype *src, double scale){
	/* deep copy a line type structure and apply a scale factor in dash information */
	if (dest == NULL || src == NULL) return 0;
	
	/* copy strings */
	strncpy (dest->name, src->name, DXF_MAX_CHARS);
	strncpy (dest->descr, src->descr, DXF_MAX_CHARS);
	/* main dashes information */
	dest->size = src->size;
	dest->length = src->length * scale; /* apply scale */
	
	dest->num_el = 0;
	dest->obj = NULL;
	
	int i;
	for(i = 0; i < dest->size; i++){
		/* copy each dash definition, and apply scale factor in dimmension parameters */
		dest->dashes[i].dash = src->dashes[i].dash * scale; 
		dest->dashes[i].type  = src->dashes[i].type;
		strncpy (dest->dashes[i].sty, src->dashes[i].sty, 29);
		dest->dashes[i].sty_i  = src->dashes[i].sty_i;
		dest->dashes[i].abs_rot  = src->dashes[i].abs_rot;
		dest->dashes[i].rot  = src->dashes[i].rot;
		dest->dashes[i].scale = src->dashes[i].scale * scale;
		dest->dashes[i].ofs_x = src->dashes[i].ofs_x * scale;
		dest->dashes[i].ofs_y = src->dashes[i].ofs_y * scale;
		
		dest->dashes[i].num = 0;
		if (dest->dashes[i].type == LTYP_SHAPE){
			dest->dashes[i].num  = src->dashes[i].num;
		}
		else if (dest->dashes[i].type == LTYP_STRING){
			strncpy (dest->dashes[i].str, src->dashes[i].str, 29);
		}
	}
	
	return 1;
}

dxf_ltype * parse_lin_def(char *doc, int *n){
	/* parse Linetype Definition buffer */
	
	*n = 0; /*number of line types returned in vector - init to zero*/
	if (doc == NULL) return NULL; /* Error - invalid buffer */
	
	static dxf_ltype ret_vec[DXF_MAX_LTYPES];  /* returned Lin type data vector */
	char field[81]; /* field buffer */
	int idx = 0;
	enum State {
		NONE,
		NAME,
		DESCR,
		ALIGN,
		STROKE,
		CMPLX
	} state = NONE;
	
	enum Cmplx_state {
		SHAPE,
		FONT,
		STRING,
		STYLE,
		PARAM
	} cmplx_state = SHAPE;
	
	int new_line = 1, end_field = 0, end_cmplx = 0, end_str = 0;
	double stroke = 0.0;
	
	for(; *doc; doc++) { /* sweep document, analysing each character */
		if (*doc == '\n') { /* new line */
			new_line = 1;
			if (state != NONE) end_field = 1;
			idx = 0;
		}
		else if (*doc == '\r') { /* new line */
			if (*(doc + 1) == '\n') doc++;
			new_line = 1;
			if (state != NONE) end_field = 1;
			idx = 0;
		}
		else if (*doc == '*' && new_line){ /* start a new line type definition */
			/* proceed to get line type data, starting by name */
			state = NAME;
			new_line = 0;
		}
		else if (*doc == ';' && new_line){ /* comentary line - no action*/
			state = NONE;
			new_line = 0;
		}
		else if (*doc == ',' ){ /* field terminator */
			end_field = 1;
			idx = 0;
		}
		else if (*doc == '[' && state != DESCR) { /* starts a complex element */
			if (state != STROKE) state = NONE; /* ERROR */
			else {
				state = CMPLX;
				cmplx_state = SHAPE;
				end_cmplx = 0;
				end_str = 0;
			}
		}
		else if (*doc == ']' && state != DESCR) { /* ends complex element definition */
			if (state != CMPLX) state = NONE; /* ERROR */
			else end_cmplx = 1;
		}
		else if (*doc == '"' && state != DESCR){ /* starts or ends a string element */
			if (state != CMPLX) state = NONE; /* ERROR */
			else{
				if (cmplx_state != STRING ) cmplx_state = STRING;
				else end_str = 1;
			}
		}
		else if (state != NONE){
			/* store current character in field's buffer */
			if (new_line) new_line = 0;
			field[idx] = *doc;
			if (idx < 79) idx++;
			field[idx] = 0; /* terminate string */
		}
		
		if (end_field){ /* Field buffer is completed - proceed to get data, according state */
			end_field = 0;
			if (state == NAME){ /* init line type */
				/* get line type name */
				strncpy(ret_vec[*n].name, field, 80);
				/* init other parameters */
				ret_vec[*n].descr[0] = 0;
				ret_vec[*n]. size = 0;
				ret_vec[*n].length = 0.0;
				ret_vec[*n].dashes[0].dash = 0;
				ret_vec[*n].dashes[0].type = LTYP_SIMPLE;
				ret_vec[*n].dashes[0].str[0] = 0;
				ret_vec[*n].dashes[0].sty[0] = 0;
				ret_vec[*n].dashes[0].sty_i = -1;
				ret_vec[*n].dashes[0].abs_rot = 0;
				ret_vec[*n].dashes[0].rot = 0.0;
				ret_vec[*n].dashes[0].scale = 1.0;
				ret_vec[*n].dashes[0].ofs_x = 0.0;
				ret_vec[*n].dashes[0].ofs_y = 0.0;
				
				/* go to the next state */
				if (new_line) state = ALIGN;
				else state = DESCR; /* optional description */
			}
			else if (state == DESCR){
				strncpy(ret_vec[*n].descr, field, 80);
				if (new_line) state = ALIGN;
				else state = NONE; /* ERROR */
			}
			else if (state == ALIGN){
				if (strcmp(field, "A") != 0) state = NONE; /* ERROR */
				if (!new_line) state = STROKE;
				else state = NONE; /* ERROR */
			}
			else if (state == STROKE){
				/* dash length */
				stroke = atof(field);
				ret_vec[*n].dashes[ret_vec[*n]. size].dash = stroke;
				/* init current dash parameters*/
				ret_vec[*n].dashes[ret_vec[*n]. size].type = LTYP_SIMPLE;
				ret_vec[*n].dashes[ret_vec[*n]. size].str[0] = 0;
				ret_vec[*n].dashes[ret_vec[*n]. size].sty[0] = 0;
				ret_vec[*n].dashes[ret_vec[*n]. size].sty_i = -1;
				ret_vec[*n].dashes[ret_vec[*n]. size].abs_rot = 0;
				ret_vec[*n].dashes[ret_vec[*n]. size].rot = 0.0;
				ret_vec[*n].dashes[ret_vec[*n]. size].scale = 1.0;
				ret_vec[*n].dashes[ret_vec[*n]. size].ofs_x = 0.0;
				ret_vec[*n].dashes[ret_vec[*n]. size].ofs_y = 0.0;
				
				/* update line type global parameters */
				ret_vec[*n].length += fabs(stroke);
				ret_vec[*n]. size++;
				if (new_line) {
					/* complete line type */
					if (*n < DXF_MAX_LTYPES - 2) *n = *n + 1;
					state = NONE;
				}
			}
			else if (state == CMPLX){
				/* complex elements */
				/* store in previous dash index */
				int idx = 0;
				if (ret_vec[*n]. size > 0) idx = ret_vec[*n]. size - 1;
				
				if (new_line) state = NONE; /* ERROR */
				
				if (cmplx_state == SHAPE){
					/* get shape name */
					ret_vec[*n].dashes[idx].type = LTYP_SHAPE;
					strncpy(ret_vec[*n].dashes[idx].str, field, 29);
					cmplx_state = FONT;
				}
				else if (cmplx_state == STRING){
					/* get string in line type */
					ret_vec[*n].dashes[idx].type = LTYP_STRING;
					strncpy(ret_vec[*n].dashes[idx].str, field, 29);
					cmplx_state = STYLE;
				}
				else if (cmplx_state == FONT){
					/* get font name, for shape */
					strncpy(ret_vec[*n].dashes[idx].sty, field, 29);
					cmplx_state = PARAM;
				}
				else if (cmplx_state == STYLE){
					/* get text style name, for string */
					strncpy(ret_vec[*n].dashes[idx].sty, field, 29);
					cmplx_state = PARAM;
				}
				else if (cmplx_state == PARAM){
					/* indexed parameters (scale, rotation and offsets) */
					const char s[2] = "=";
					char *sufix = NULL, *value = NULL;
					
					/* get the parameter id */
					char *id = strtok(field, s);
					if (id) value = strtok(NULL, s);
					if(id && value){
						if(strpbrk(id, "Ss")){ /* scale */
							ret_vec[*n].dashes[idx].scale = atof(value);
						}
						if(strpbrk(id, "Xx")){ /* X offset */
							ret_vec[*n].dashes[idx].ofs_x = atof(value);
						}
						if(strpbrk(id, "Yy")){ /* Y offset */
							ret_vec[*n].dashes[idx].ofs_y = atof(value);
						}
						if(strpbrk(id, "Rr")){ /* Relative rotation angle */
							ret_vec[*n].dashes[idx].abs_rot = 0;
							ret_vec[*n].dashes[idx].rot = strtod(value, &sufix);
							if (sufix){ /* angle units */
								if(sufix[0] == 'D' || sufix[0] == 'd'){
									/* angle in degrees - no conversion needed */
								}
								else if(sufix[0] == 'R' || sufix[0] == 'r'){
									/* angle in radians */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/M_PI;
								}
								else if(sufix[0] == 'G' || sufix[0] == 'g'){
									/* angle in grads */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/200.0;
								}
							}
						}
						if(strpbrk(id, "Aa")){ /* Absolute rotation angle */
							ret_vec[*n].dashes[idx].abs_rot = 1;
							ret_vec[*n].dashes[idx].rot = strtod(value, &sufix);
							if (sufix){
								if(sufix[0] == 'D' || sufix[0] == 'd'){
									/* angle in degrees - no conversion needed */
								}
								else if(sufix[0] == 'R' || sufix[0] == 'r'){
									/* angle in radians */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/M_PI;
								}
								else if(sufix[0] == 'G' || sufix[0] == 'g'){
									/* angle in grads */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/200.0;
								}
							}
						}
						if(strpbrk(id, "Uu")){ /* "Easy to read" rotation - not fully implemented: equivalent to Relative rotation angle  */
							ret_vec[*n].dashes[idx].abs_rot = 0;
							ret_vec[*n].dashes[idx].rot = strtod(value, &sufix);
							if (sufix){
								if(sufix[0] == 'D' || sufix[0] == 'd'){
									/* angle in degrees - no conversion needed */
								}
								else if(sufix[0] == 'R' || sufix[0] == 'r'){
									/* angle in radians */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/M_PI;
								}
								else if(sufix[0] == 'G' || sufix[0] == 'g'){
									/* angle in grads */
									ret_vec[*n].dashes[idx].rot = ret_vec[*n].dashes[idx].rot * 180.0/200.0;
								}
							}
						}
					}
				}
				if (end_cmplx){
					end_cmplx = 0;
					state = STROKE;
					if (new_line) {
						/* complete line type */
						if (*n < DXF_MAX_LTYPES - 2) *n = *n + 1;
						state = NONE;
					}
				} 
			}
		}
	}
	
	return ret_vec;
}

int font_tstyle_idx (dxf_drawing *drawing, char *name){
	/* try to find a text style (return a index) looking for a font name */
	int i;
	char name1[DXF_MAX_CHARS], name2[DXF_MAX_CHARS];
	strncpy(name1, name, DXF_MAX_CHARS); /* preserve original string */
	str_upp(name1); /*upper case */
	if (drawing){
		for (i=0; i < drawing->num_tstyles; i++){
			/* look font name (file path) */
			strncpy(name2, drawing->text_styles[i].file, DXF_MAX_CHARS); /* preserve original string */
			str_upp(name2); /*upper case */
			if (strcmp(name1, name2) == 0){
				return i;
			}
		}
	}
	
	return -1; /*search fails */
}

dxf_ltype * load_lin_buf(dxf_drawing *drawing, char *buf, int *n){
	/* parse Linetype Definition string and validate in drawing parameters */
	*n = 0;
	dxf_ltype * lib = parse_lin_def(buf, n);
	int i, j;
	
	for (i = 0; i < *n; i++){ /* sweep linetype vector */
		for (j = 0; j < lib[i].size; j++){ /* validate each complex dash */
			if (lib[i].dashes[j].type == LTYP_SHAPE){ /* dash have a shape element */
				/* get drawing's text style, by looking its shape file */
				drawing, lib[i].dashes[j].sty_i = font_tstyle_idx(drawing, lib[i].dashes[j].sty);
				
				if (drawing, lib[i].dashes[j].sty_i >= 0){ /* text style found */
					struct tfont *font = drawing->text_styles[lib[i].dashes[j].sty_i].font;
					if (font && font->type == FONT_SHP){ /* verify if is a shape file */
						/* search shape, by looking its name */
						shp_typ *shape = shp_name((shp_typ *)font->data, lib[i].dashes[j].str);
						if (shape){
							lib[i].dashes[j].num = shape->num; /* update line type */
						}
					}
				}
				else if(dxf_new_tstyle_shp (drawing, lib[i].dashes[j].sty)){ /* try to add a new font and reload */
					i = -1;
					j = 0;
					break;
				}
			}
			else if (lib[i].dashes[j].type == LTYP_STRING){ /* dash have a string element */
				/* get drawing's text style, by looking its name */
				drawing, lib[i].dashes[j].sty_i = dxf_tstyle_idx(drawing, lib[i].dashes[j].sty);
			}
		}
	}
	
	return lib;
}

/* Custom nuklear widget to show line patern preview.  Derived frow styled button.*/
 int preview_ltype(struct nk_context *ctx, struct nk_style_button *style, dxf_ltype line_type, double length) {
	/* get canvas to draw widget */
	struct nk_command_buffer *canvas;
	canvas = nk_window_get_canvas(ctx);
	
	/* get state of widget */
	struct nk_rect space;
	enum nk_widget_layout_states state;
	state = nk_widget(&space, ctx);
	if (!state) return 0; /* no need to do anything - widget out of drawing window */
	 
	 struct nk_input *input = &ctx->input;
	int ret  = 0;
	struct nk_color color;
	const struct nk_style_item *background;
	
	/* get parameters from style - normal is the default */
	background = &style->normal;
	color = style->text_normal;
	if (state != NK_WIDGET_ROM){
		/* widget need attention */
		if (nk_input_is_mouse_hovering_rect(input, space)) {
			/* hovering */
			background = &style->hover;
			color = style->text_hover;
			if (nk_input_is_mouse_down(input, NK_BUTTON_LEFT)){
				/* click */
				background = &style->active;
				color = style->text_active;
				ret = 1;
			}
		}
	}
	
	/* draw background, according style */
	if (background->type == NK_STYLE_ITEM_IMAGE) {
		nk_draw_image(canvas, space, &background->data.image, nk_rgb(255,255,255));
	} else {
		nk_fill_rect(canvas, space, style->rounding, background->data.color);
		nk_stroke_rect(canvas, space, style->rounding, style->border, style->border_color);
	}
	
	/* calcule content area */
	struct nk_rect content;
	content.x = space.x + style->padding.x + style->border + style->rounding;
	content.y = space.y + style->padding.y + style->border + style->rounding;
	content.w = space.w - (2 * style->padding.x + style->border + style->rounding*2);
	content.h = space.h - (2 * style->padding.y + style->border + style->rounding*2);
	
	double scale = 1.0;
	if (length > 0.0) scale = content.w / length; /* scale pattern by maximum length, or */
	else 
		scale = content.w / (line_type.length * 7.0); /* auto scale by current  line type */
	/* initial coordinates to draw pattern */
	double x = content.x;
	double y = content.y + content.h / 2.0;
	int i = 0, idx = 0;
	int steps = 0;
	
	/* calcule iterations needed to draw pattern */
	if (line_type.length > 0.0) steps = (double)line_type.size * content.w / (line_type.length * scale);
	
	if (steps == 0 || steps > content.w) {
		/* continuous line case */
		nk_stroke_line(canvas, x, y, x+content.w, y, 2.1, color);
	}
	else{
		while (i < steps){
			/* draw pattern */
			double x1 = x + fabs(line_type.dashes[idx].dash) * scale;
			if (x1 > content.x + content.w) x1 = content.x + content.w;
			if (line_type.dashes[idx].dash >= 0.0) nk_stroke_line(canvas, x, y, x1, y, 2.1, color);
			x = x1;
			
			idx++; /* index to current stroke in pattern*/
			if (idx >= line_type.size) idx = 0; /* restart pattern */
			i++;
		}
	}
	
	return ret;
}

/* ---------------------  auxiliary functions for sorting files (qsort) ------------ */
/* compare by ltype name */
int cmp_ltype_name(const void * a, const void * b) {
	char *name1, *name2;
	char copy1[DXF_MAX_CHARS+1], copy2[DXF_MAX_CHARS+1];
	
	dxf_ltype *ltyp1 = ((struct sort_by_idx *)a)->data;
	dxf_ltype *ltyp2 = ((struct sort_by_idx *)b)->data;
	/* copy strings for secure manipulation */
	strncpy(copy1, ltyp1->name, DXF_MAX_CHARS);
	strncpy(copy2, ltyp2->name, DXF_MAX_CHARS);
	/* remove trailing spaces */
	name1 = trimwhitespace(copy1);
	name2 = trimwhitespace(copy2);
	/* change to upper case */
	str_upp(name1);
	str_upp(name2);
	return (strncmp(name1, name2, DXF_MAX_CHARS));
}

int cmp_ltype_name_rev(const void * a, const void * b) {
	return -cmp_ltype_name(a, b);
}

/* compare by ltype in use flag */
int cmp_ltype_use(const void * a, const void * b) {
	char *ltype1, *ltype2;
	int ret;
	
	dxf_ltype *ltyp1 = ((struct sort_by_idx *)a)->data;
	dxf_ltype *ltyp2 = ((struct sort_by_idx *)b)->data;
	
	ret = (ltyp1->num_el > 0) - (ltyp2->num_el > 0);
	if (ret == 0) { /* in case both ltypes in use, sort by ltype name */
		return cmp_ltype_name(a, b);
	}
	return ret;
}
int cmp_ltype_use_rev(const void * a, const void * b) {
	return -cmp_ltype_use(a, b);
}

/*-------------------------------------------------------------------------------------*/


/* ltype manager window */
int ltyp_mng (gui_obj *gui){
	int i, show_ltyp_mng = 1;
	static int show_add = 0, show_ltyp_name = 0, init = 0;
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 900;
	gui->next_win_h = 470;
	
	enum ltyp_op {
		LTYP_OP_NONE,
		LTYP_OP_ADD,
		LTYP_OP_RENAME,
		LTYP_OP_UPDATE
	};
	static int ltyp_change = LTYP_OP_NONE;
	
	dxf_ltype *ltypes = gui->drawing->ltypes;
	int num_ltypes = 0;
	
	static bmp_img ltyp_prev[DXF_MAX_LTYPES + 1];
	#define PREV_W 294
	#define PREV_H 34
	static unsigned char prev_buf[DXF_MAX_LTYPES + 1][4 * PREV_W * PREV_H];
	if (!init){
		bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
		bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
		for (i = 0; i <= DXF_MAX_LTYPES; i++){
			/* initialize preview images */
			/* dimensions */
			ltyp_prev[i].width = PREV_W;
			ltyp_prev[i].height = PREV_H;
			/* colors */
			ltyp_prev[i].bkg = transp;
			ltyp_prev[i].frg = black;
			/* line pattern generation vars  */
			ltyp_prev[i].tick = 0;
			ltyp_prev[i].patt_i = 0;
			ltyp_prev[i].pix_count = 0;
			/* line pattern */
			/* initialize the image with a solid line pattern */
			ltyp_prev[i].patt_size = 1;
			ltyp_prev[i].pattern[0] = 1;
			ltyp_prev[i].pat_scale = 15;
			
			ltyp_prev[i].zero_tl = 0; /* zero in botton left corner */
			
			/*clipping rectangle */
			ltyp_prev[i].clip_x = 0;
			ltyp_prev[i].clip_y = 0;
			ltyp_prev[i].clip_w = PREV_W;
			ltyp_prev[i].clip_h = PREV_H;
			
			ltyp_prev[i].prev_x = 0;
			ltyp_prev[i].prev_y = 0;
			
			ltyp_prev[i].end_x[0] = 0; ltyp_prev[i].end_x[1] = 0; ltyp_prev[i].end_x[2] = 0; ltyp_prev[i].end_x[3] = 0;
			ltyp_prev[i].end_y[0] = 0; ltyp_prev[i].end_y[1] = 0; ltyp_prev[i].end_y[2] = 0; ltyp_prev[i].end_y[3] = 0;
			
			/*order of color components in buffer. Init with ARGB */
			ltyp_prev[i].r_i = 2;
			ltyp_prev[i].g_i = 1;
			ltyp_prev[i].b_i = 0;
			ltyp_prev[i].a_i = 3;
			
			/* "alloc" the pix map buffer
			the image will have 4 chanels: R, G, B and alfa */
			ltyp_prev[i].buf = prev_buf[i];
			bmp_fill (&(ltyp_prev[i]), transp); /* fill the image with the background color */
		}
		
		init = 1;
	}
	
	
	double max_len = 0.0;
	
	static int sorted = 0;
	enum sort {
		UNSORTED,
		BY_NAME,
		BY_LTYPE,
		BY_COLOR,
		BY_LW,
		BY_USE,
		BY_OFF,
		BY_FREEZE,
		BY_LOCK
	};
	static int sort_reverse = 0;
	
	static struct sort_by_idx sort_ltyp[DXF_MAX_LTYPES];
	char str_copy[DXF_MAX_CHARS+1]; /* for case insensitive string comparission */
	
	
	ltype_use(gui->drawing); /* update ltypes in use*/
	
	/* construct list for sorting */
	num_ltypes = 0;
	max_len = 0.0;
	for (i = 0; i < gui->drawing->num_ltypes; i++){
		strncpy(str_copy, ltypes[i].name, DXF_MAX_CHARS);
		str_upp(str_copy);
		if (!(strcmp(str_copy, "BYLAYER") == 0 || strcmp(str_copy, "BYBLOCK") == 0)){ /* skip bylayer and byblock line descriptions */
			if (ltypes[i].size > 1 && ltypes[i].length > max_len) max_len = ltypes[i].length;
			
			sort_ltyp[num_ltypes].idx = i;
			sort_ltyp[num_ltypes].data = &(ltypes[i]);
			num_ltypes++;
		}
	}
	
	/* generate image previews for linetypes */
	if (max_len <= 0.0) max_len = 10.0;
	for (i = 0; i < gui->drawing->num_ltypes; i++){
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){ /* create a graph object to preview */
			/* apply the line type */
			change_ltype (gui->drawing, graph, i, 1.0 / gui->drawing->ltscale);
			/*get color of button widget text */
			struct nk_color t_color = gui->ctx->style.button.text_normal;
			bmp_color color = {.r = t_color.r, .g = t_color.g, .b = t_color.b, .a = t_color.a};
			graph->color = color;
			/* add a single line - try to scale its size to view and compare pattern*/
			if (ltypes[i].length > 0.0 && max_len > 40.0 * ltypes[i].length) 
				line_add(graph, 0, 2*ltypes[i].length*PREV_H/PREV_W, 0, 4 * ltypes[i].length, 2*ltypes[i].length*PREV_H/PREV_W, 0);
			else line_add(graph, 0, 2*max_len*PREV_H/PREV_W, 0, 4*max_len, 2*max_len*PREV_H/PREV_W, 0);
			
		}
		/* draw parameters */
		struct draw_param d_param;
		d_param.ofs_x = 0;
		d_param.ofs_y = 0;
		/* try to scale its size to view and compare pattern*/
		if (ltypes[i].length > 0.0 && max_len > 40.0 * ltypes[i].length) 
			d_param.scale = (double) 0.95*PREV_W /(4 * ltypes[i].length);
		else 
			d_param.scale = (double) 0.95*PREV_W / (4*max_len);
		d_param.list = NULL;
		d_param.subst = NULL;
		d_param.len_subst = 0;
		d_param.inc_thick = 1;
		bmp_fill (&(ltyp_prev[i]), ltyp_prev[i].bkg); /* clear the image with the background color */
		graph_draw3(graph, &(ltyp_prev[i]), d_param); /* finally draw preview */
		
	}
	
	/* execute sort, according sorting criteria */
	if (sorted == BY_NAME){
		if(!sort_reverse)
			qsort(sort_ltyp, num_ltypes, sizeof(struct sort_by_idx), cmp_ltype_name);
		else
			qsort(sort_ltyp, num_ltypes, sizeof(struct sort_by_idx), cmp_ltype_name_rev);
	}
	else if (sorted == BY_USE){
		if(!sort_reverse)
			qsort(sort_ltyp, num_ltypes, sizeof(struct sort_by_idx), cmp_ltype_use);
		else
			qsort(sort_ltyp, num_ltypes, sizeof(struct sort_by_idx), cmp_ltype_use_rev);
	}
	
	if (nk_begin(gui->ctx, "Line Types Manager", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		static char ltyp_name[DXF_MAX_CHARS] = "";
		int ltyp_exist = 0;
		nk_flags res;
		static char ltscale_str[64] = "1.0";
		static char celtscale_str[64] = "1.0";
		
		static int sel_ltyp = -1;
		int lw_i, sel_ltype, ltyp_idx;
		
		char str_tmp[DXF_MAX_CHARS];
		
		dxf_node *ltyp_flags = NULL;
		
		/* list header */
		nk_layout_row_dynamic(gui->ctx, 32, 1);
		if (nk_group_begin(gui->ctx, "Ltyp_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* buttons to change sorting criteria */
			
			nk_layout_row(gui->ctx, NK_STATIC, 22, 4, (float[]){175, 300, 300, 50});
			/* sort by ltype name */
			if (sorted == BY_NAME){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Name", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Name", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Name")){
				sorted = BY_NAME;
				sort_reverse = 0;
			}
			
			nk_button_label(gui->ctx, "Description");
			nk_button_label(gui->ctx, "Preview");
			
			/* sort by use */
			if (sorted == BY_USE){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Used", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Used", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Used")){
				sorted = BY_USE;
				sort_reverse = 0;
			}
			
			nk_group_end(gui->ctx);
		}
		
		/* body of list */
		nk_layout_row_dynamic(gui->ctx, 300, 1);
		if (nk_group_begin(gui->ctx, "Ltyp_view", NK_WINDOW_BORDER)) {
			nk_layout_row(gui->ctx, NK_STATIC, 40, 4, (float[]){175, 300, 300, 50});
			for (i = 0; i < num_ltypes; i++){ /* sweep list of ltypes */
				/* show and change current ltype parameters */
				ltyp_idx = sort_ltyp[i].idx; /* current ltype */
				/* select/deselect ltype */
				if (sel_ltyp == ltyp_idx){
					if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, ltypes[ltyp_idx].name)){
						sel_ltyp = -1;
					}
				}
				else {
					if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, ltypes[ltyp_idx].name)){
						sel_ltyp = ltyp_idx;
					}
				}
				
				if (sel_ltyp == ltyp_idx){
					if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, ltypes[ltyp_idx].descr)){
						sel_ltyp = -1;
					}
				}
				else {
					if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, ltypes[ltyp_idx].descr)){
						sel_ltyp = ltyp_idx;
					}
				}
				
				if (sel_ltyp == ltyp_idx){
					//if(preview_ltype(gui->ctx ,&gui->b_icon_sel, ltypes[ltyp_idx], scale)){
					if (nk_button_image_styled(gui->ctx, &gui->b_icon_sel, nk_image_ptr(&(ltyp_prev[ltyp_idx])))){
						sel_ltyp = -1;
					}
				}
				else {
					//if(preview_ltype(gui->ctx ,&gui->b_icon_unsel, ltypes[ltyp_idx], scale)){
					if (nk_button_image_styled(gui->ctx, &gui->b_icon_unsel, nk_image_ptr(&(ltyp_prev[ltyp_idx])))){
						sel_ltyp = ltyp_idx;
					}
				}
				
				/* show if current ltype is in use */
				if (ltypes[ltyp_idx].num_el)
					nk_label(gui->ctx, "x",  NK_TEXT_CENTERED);
				else nk_label(gui->ctx, " ",  NK_TEXT_CENTERED);
			}
			nk_group_end(gui->ctx);
		} /* list end */

		
		nk_layout_row_dynamic(gui->ctx, 20, 3);
		/* add a new ltype */
		if (nk_button_label(gui->ctx, "Add")){
			/* open a window to create a new linetype */
			show_add = 1;
		}
		/* rename selected ltype */
		if ((nk_button_label(gui->ctx, "Rename")) && (sel_ltyp >= 0)){
			/* open a popup for entering the ltype name */
			show_ltyp_name = 1;
			strncpy(ltyp_name, ltypes[sel_ltyp].name, DXF_MAX_CHARS);
			ltyp_change = LTYP_OP_RENAME;
			
		}
		/* delete selected ltype */
		if ((nk_button_label(gui->ctx, "Remove")) && (sel_ltyp >= 0)){
			/* update all ltypes for sure */
			ltype_use(gui->drawing);
			/* don't remove ltype in use */
			if (ltypes[sel_ltyp].num_el){ 
				snprintf(gui->log_msg, 63, "Error: Don't remove Line Type in use");
			}
			else{
				/* remove ltype from main structure */
				do_add_entry(&gui->list_do, "Remove Line Type");
				do_add_item(gui->list_do.current, ltypes[sel_ltyp].obj, NULL);
				dxf_obj_subst(ltypes[sel_ltyp].obj, NULL);
				sel_ltyp = -1;
				/* update the drawing */
				dxf_ltype_assemb (gui->drawing);
			}
		}
		
		/* edit glogal drawing line type scale */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 4, (float[]){140, 100, 160,100});
		nk_label(gui->ctx, "Global Scale Factor:", NK_TEXT_RIGHT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ltscale_str, 63, nk_filter_float);
		if (!(res & NK_EDIT_ACTIVE)){
			snprintf(ltscale_str, 63, "%.9g", gui->drawing->ltscale);
		}
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			nk_edit_unfocus(gui->ctx);
			if (strlen(ltscale_str)) /* update parameter value */
				gui->drawing->ltscale = atof(ltscale_str);
			snprintf(ltscale_str, 63, "%.9g", gui->drawing->ltscale);
			/* change in DXF main struct */
			dxf_node *start = NULL, *end = NULL, *part = NULL;
			if(dxf_find_head_var(gui->drawing->head, "$LTSCALE", &start, &end)){
				/* variable exists */
				part = dxf_find_attr_i2(start, end, 40, 0);
				if (part != NULL){
					part->value.d_data = gui->drawing->ltscale;
				}
			}
			else{
				dxf_attr_append(gui->drawing->head, 9, "$LTSCALE", DWG_LIFE);
				dxf_attr_append(gui->drawing->head, 40, &gui->drawing->ltscale, DWG_LIFE);
			}
			dxf_ents_parse(gui->drawing); /* redraw all drawing*/
			gui->draw = 1;
		}
		
		/* edit line type scale for the next new entities*/
		nk_label(gui->ctx, "Current Object Scale:", NK_TEXT_RIGHT);
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, celtscale_str, 63, nk_filter_float);
		if (!(res & NK_EDIT_ACTIVE)){
			snprintf(celtscale_str, 63, "%.9g", gui->drawing->celtscale);
		}
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			nk_edit_unfocus(gui->ctx);
			if (strlen(celtscale_str)) /* update parameter value */
				gui->drawing->celtscale = atof(celtscale_str);
			snprintf(celtscale_str, 63, "%.9g", gui->drawing->celtscale);
		}
		
		/* popup to entering ltype name in rename action */
		if ((show_ltyp_name)){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Line Type Name", NK_WINDOW_CLOSABLE, nk_rect(10, 20, 220, 100))){
				
				/* get name */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ltyp_name, DXF_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "OK")){
					/* try to create a new ltype */
					if ((ltyp_change == LTYP_OP_RENAME) && (sel_ltyp >= 0)){
						/* verify if name already exists*/
						ltyp_exist = 0;
						for (i = 0; i < num_ltypes; i++){
							if (i != sel_ltyp){ /*except current ltype*/
								if(strcmp(ltypes[i].name, ltyp_name) == 0){
									ltyp_exist = 1;
									break;
								}
							}
						}
						if (!ltyp_exist){ /* change ltype name */
							ltype_rename(gui->drawing, sel_ltyp, ltyp_name); /* update all related elements in drawing */
							nk_popup_close(gui->ctx);
							show_ltyp_name = 0;
						}
						else snprintf(gui->log_msg, 63, "Error: exists Line Type with same name");
					}
				/* ------------ cancel or close window ---------------- */
					else {
						nk_popup_close(gui->ctx);
						show_ltyp_name = 0;
						ltyp_change = LTYP_OP_NONE;
					}
				}
				if (nk_button_label(gui->ctx, "Cancel")){
					nk_popup_close(gui->ctx);
					show_ltyp_name = 0;
					ltyp_change = LTYP_OP_NONE;
				}
				nk_popup_end(gui->ctx);
			} else {
				show_ltyp_name = 0;
				ltyp_change = LTYP_OP_NONE;
			}
		}
	} else {
		show_ltyp_mng = 0;
		ltyp_change = LTYP_OP_UPDATE;
	}
	nk_end(gui->ctx);
	
	if ((show_add)){
		static int add_init = 0;
		/* Window to add a line type definition to drawing */
		if (nk_begin(gui->ctx, "Add Line Type", nk_rect(gui->next_win_x + 150, gui->next_win_y + 20, 560, 520), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)){
			
			static char name[DXF_MAX_CHARS+1] = "", descr[DXF_MAX_CHARS+1] = "";
			static char cpy_from[DXF_MAX_CHARS+1] = "";
			static enum Mode {LT_ADD_CPY, LT_ADD_LIB} mode = LT_ADD_CPY;
			static enum libMode {LT_LIB_NONE, LT_LIB_DFLT, LT_LIB_EXTRA, LT_LIB_FILE} lib_mode = LT_LIB_NONE;
			static int idx = -1, sel_ltyp = -1, n_lib = 0;
			static double scale = 1.0;
			static dxf_ltype * lib = NULL;
			
			if (!add_init){
				add_init = 1;
				name[0] = 0;
				descr[0] = 0;
				cpy_from[0] = 0;
				idx = -1;
			}
			
			dxf_ltype line_type;
			
			/* Name and description of new line type */
			nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.4, 0.6});
			nk_label(gui->ctx, "Name:", NK_TEXT_LEFT);
			nk_label(gui->ctx, "Description:", NK_TEXT_LEFT);
			nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, name, DXF_MAX_CHARS, nk_filter_default);
			nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, descr, DXF_MAX_CHARS, nk_filter_default);
			
			/* apply scale to new line type (from original definition source) */
			nk_layout_row(gui->ctx, NK_STATIC, 32, 2, (float[]){100, 200});
			nk_label(gui->ctx, "Apply Scale:", NK_TEXT_RIGHT);
			scale = nk_propertyd(gui->ctx, "Factor", 1e-9, scale, 1.0e9, SMART_STEP(scale), SMART_STEP(scale));
			
			/* Selection mode option - source of line type definition - copy or library */
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, "From:", NK_TEXT_LEFT);
			nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
			nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
			if (gui_tab (gui, "Copy", mode == LT_ADD_CPY)) mode = LT_ADD_CPY;
			if (gui_tab (gui, "Library", mode == LT_ADD_LIB)) mode = LT_ADD_LIB;
			nk_style_pop_vec2(gui->ctx);
			nk_layout_row_end(gui->ctx);
			
			nk_layout_row_dynamic(gui->ctx, 315, 1);
			if (nk_group_begin(gui->ctx, "lt_add_controls", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				if (mode == LT_ADD_CPY){
					/* copy from existing line type in drawing */
					nk_layout_row(gui->ctx, NK_STATIC, 22, 2, (float[]){60, 200});
					nk_label(gui->ctx, "Source:", NK_TEXT_LEFT);
					
					int h = num_ltypes * 25 + 5;
					h = (h < 200)? h : 200;

					if (nk_combo_begin_label(gui->ctx, cpy_from, nk_vec2(300, h))){
						nk_layout_row_dynamic(gui->ctx, 20, 2);
						
						for (i = 0; i < num_ltypes; i++){
							int ltyp_idx = sort_ltyp[i].idx; /* current ltype */
							if (nk_button_label(gui->ctx, ltypes[ltyp_idx].name)){
								/* pre set name and description of new line with selected one */
								idx = ltyp_idx;
								strncpy (cpy_from, ltypes[ltyp_idx].name, DXF_MAX_CHARS);
								strncpy (name, ltypes[ltyp_idx].name, DXF_MAX_CHARS);
								strncpy (descr, ltypes[ltyp_idx].descr, DXF_MAX_CHARS);
								nk_combo_close(gui->ctx);
								break;
							}
							nk_label(gui->ctx, ltypes[ltyp_idx].descr, NK_TEXT_LEFT);
						}
						
						nk_combo_end(gui->ctx);
					}
					
				}
				if (mode == LT_ADD_LIB) {
					/* Get new line type definition from library */
					nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
					nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
					/* There are 3 library options to choose */
					if (gui_tab (gui, "Default", lib_mode == LT_LIB_DFLT)) {
						/* default library - configurable internal library */
						lib_mode = LT_LIB_DFLT;
						lib = load_lin_buf(gui->drawing, gui->dflt_lin, &n_lib); /* parse library */
					}
					if (gui_tab (gui, "Extra", lib_mode == LT_LIB_EXTRA)) {
						/* extra library - another configurable internal library */
						lib_mode = LT_LIB_EXTRA;
						lib = load_lin_buf(gui->drawing, gui->extra_lin, &n_lib); /* parse library */
					}
					if (gui_tab (gui, "File", lib_mode == LT_LIB_FILE)) {
						/* external library, from .LIN files */
						lib_mode = LT_LIB_FILE;
						n_lib = 0;
					}
					nk_style_pop_vec2(gui->ctx);
					nk_layout_row_end(gui->ctx);
					
					
					if(lib_mode == LT_LIB_FILE){ /* file library option */
						/* prepare to load designated file path*/
						
						nk_layout_row_dynamic(gui->ctx, 10, 1);
						
						static char path[DXF_MAX_CHARS] = "";
						static int show_app_file = 0;
						int i;

						/* supported file format */
						static const char *ext_type[] = {
							"LIN",
							"*"
						};
						static const char *ext_descr[] = {
							"Line Type Library (.lin)",
							"All files (*)"
						};
						#define FILTER_COUNT 2
						nk_layout_row(gui->ctx, NK_DYNAMIC, 22, 3, (float[]){0.15, 0.75, 0.1});
						
						if (nk_button_label(gui->ctx, "Browse")){/* call file browser */
							show_app_file = 1;
							/* set filter for suported output formats */
							for (i = 0; i < FILTER_COUNT; i++){
								gui->file_filter_types[i] = ext_type[i];
								gui->file_filter_descr[i] = ext_descr[i];
							}
							gui->file_filter_count = FILTER_COUNT;
							gui->filter_idx = 0;
							
							gui->show_file_br = 1;
							gui->curr_path[0] = 0;
						}
						if (show_app_file){ /* running file browser */
							if (gui->show_file_br == 2){ /* return file OK */
								/* close browser window*/
								gui->show_file_br = 0;
								show_app_file = 0;
								/* update output path */
								strncpy(path, gui->curr_path, DXF_MAX_CHARS - 1);
							}
						}
						
						/* manual entry to file path */
						nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, path, DXF_MAX_CHARS - 1, nk_filter_default);
						
						if (nk_button_label(gui->ctx, "Load")){
							
							long fsize;
							struct Mem_buffer *buf  = load_file_reuse(path, &fsize); /* load file in temp buffer */
							/* parse line type library */
							if (buf != NULL) {
								lib = load_lin_buf(gui->drawing, (char *)buf->buffer, &n_lib);
								gui_tstyle(gui); /* try to update fonts */
								lib = load_lin_buf(gui->drawing, (char *)buf->buffer, &n_lib);
							}
							
							manage_buffer(0, BUF_RELEASE); /* release buffer */
						}
					}
					
					if( n_lib > 0 && lib != NULL ){ /* verify if exists line type definitions in library */
						/* list header */
						nk_layout_row_dynamic(gui->ctx, 32, 1);
						if (nk_group_begin(gui->ctx, "LibLtyp_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
							nk_layout_row(gui->ctx, NK_STATIC, 22, 2, (float[]){175, 300});
							nk_button_label(gui->ctx, "Name");
							nk_button_label(gui->ctx, "Description");
							//nk_button_label(gui->ctx, "Preview");
							nk_group_end(gui->ctx);
						}
						
						/* body of list */
						nk_layout_row_dynamic(gui->ctx, 150, 1);
						if (nk_group_begin(gui->ctx, "LibLtyp_view", NK_WINDOW_BORDER)) {
							nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){175, 300});
							for (i = 0; i < n_lib; i++){ /* show each line type information */
								/* select/deselect ltype */
								if (sel_ltyp == i){
									if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, lib[i].name)){
										sel_ltyp = -1;
									}
								}
								else {
									if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, lib[i].name)){
										sel_ltyp = i;
										/* pre set name and description of new line with selected one */
										strncpy (name, lib[i].name, DXF_MAX_CHARS);
										strncpy (descr, lib[i].descr, DXF_MAX_CHARS);
									}
								}
								
								if (sel_ltyp == i){
									if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, lib[i].descr)){
										sel_ltyp = -1;
									}
								}
								else {
									if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, lib[i].descr)){
										sel_ltyp = i;
										/* pre set name and description of new line with selected one */
										strncpy (name, lib[i].name, DXF_MAX_CHARS);
										strncpy (descr, lib[i].descr, DXF_MAX_CHARS);
									}
								}
							}
							nk_group_end(gui->ctx);
						}
					}
					/* preview of selected line type in library */
					if (n_lib > 0 && sel_ltyp>= 0){
						static double prev_s = 1.0; /* preview scale */
						graph_obj *graph = graph_new(FRAME_LIFE);
						if (graph){ /* create a graph object to preview */
							/* apply the line type */
							change_ltype2 (gui->drawing, graph, lib[sel_ltyp], 1.0 / gui->drawing->ltscale);
							/*get color of button widget text */
							struct nk_color t_color = gui->ctx->style.button.text_normal;
							bmp_color color = {.r = t_color.r, .g = t_color.g, .b = t_color.b, .a = t_color.a};
							graph->color = color;
							/* add a single line - try to scale its size to view and compare pattern*/
							if (lib[sel_ltyp].length > 0.0) 
								line_add(graph, 0, 2*prev_s * lib[sel_ltyp].length*PREV_H/PREV_W, 0, 4 * prev_s * lib[sel_ltyp].length, 2*prev_s * lib[sel_ltyp].length*PREV_H/PREV_W, 0);
							else line_add(graph, 0, 0.5*PREV_H, 0, PREV_W, 0.5*PREV_H, 0); /* continuous line */
							
						}
						/* draw parameters */
						struct draw_param d_param;
						d_param.ofs_x = 0;
						d_param.ofs_y = 0;
						/* try to scale its size to view and compare pattern*/
						d_param.scale = 1.0;
						if (lib[sel_ltyp].length > 0.0) 
							d_param.scale = (double) 0.95*PREV_W /(4 * prev_s * lib[sel_ltyp].length);
						d_param.list = NULL;
						d_param.subst = NULL;
						d_param.len_subst = 0;
						d_param.inc_thick = 1;
						bmp_fill (&(ltyp_prev[DXF_MAX_LTYPES]), ltyp_prev[DXF_MAX_LTYPES].bkg); /* clear the image with the background color */
						graph_draw3(graph, &(ltyp_prev[DXF_MAX_LTYPES]), d_param); /* finally draw preview */
						
						nk_layout_row(gui->ctx, NK_STATIC, 40, 4, (float[]){70, 300, 120});
						nk_label(gui->ctx, "Preview:", NK_TEXT_RIGHT);
						/* show preview image as a button */
						nk_button_image_styled(gui->ctx, &gui->b_icon_unsel, nk_image_ptr(&(ltyp_prev[DXF_MAX_LTYPES])));
						/* edit preview scale */
						prev_s = nk_propertyd(gui->ctx, "Scale", 1e-9, prev_s, 1.0e9, SMART_STEP(prev_s), SMART_STEP(prev_s));
					}
				}
			}
			nk_group_end(gui->ctx);
			
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){100, 100});
			if (nk_button_label(gui->ctx, "OK")){ /* proceeding to create a line type in drawing*/
				if (mode == LT_ADD_CPY){
					if (idx > -1){
						ltype_cpy (&line_type, &ltypes[idx], scale); /* deep copy the line type structure */
						strncpy (line_type.name, name, DXF_MAX_CHARS);
						strncpy (line_type.descr, descr, DXF_MAX_CHARS);
						
						if (!dxf_new_ltype (gui->drawing, &line_type)){
							/* fail to  create, commonly name already exists */
							snprintf(gui->log_msg, 63, "Error: Line Type already exists");
						}
						else {
							/* ltype created and attached in drawing main structure */
							show_add = 0;
							add_init = 1;
						}
					}
					else{
						snprintf(gui->log_msg, 63, "Error: Select Source Line Type");
					}
				}
				else{
					if (n_lib > 0 && sel_ltyp>= 0){
						ltype_cpy (&line_type, &lib[sel_ltyp], scale); /* deep copy the line type structure */
						strncpy (line_type.name, name, DXF_MAX_CHARS);
						strncpy (line_type.descr, descr, DXF_MAX_CHARS);
						
						if (!dxf_new_ltype (gui->drawing, &line_type)){
							/* fail to  create, commonly name already exists */
							snprintf(gui->log_msg, 63, "Error: Line Type already exists");
						}
						else {
							/* ltype created and attached in drawing main structure */
							show_add = 0;
							add_init = 1;
						}
					}
				}
			}
			if (nk_button_label(gui->ctx, "Cancel")){
				show_add = 0;
				add_init = 1;
			}
			
		} else {
			show_add = 0;
			add_init = 1;
		}
		nk_end(gui->ctx);
	}
	
	return show_ltyp_mng;
}

int ltype_rename(dxf_drawing *drawing, int idx, char *name){
	/* rename existing ltype -  update all related elements in drawing */
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[3], *ltyp_obj;
	char *new_name = trimwhitespace(name);
	
	list[0] = NULL; list[1] = NULL; list[2] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
		list[2] = drawing->t_layer;
	}
	else return 0;
	
	/* first, update all related elements in drawing */
	for (i = 0; i< 3; i++){ /* look in BLOCKS and ENTITIES sections, and in LAYERS table too */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section*/
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				ltyp_obj = dxf_find_attr2(current, 6); /* get element's ltype */
				if (ltyp_obj){
					char ltype[DXF_MAX_CHARS], old_name[DXF_MAX_CHARS];
					strncpy(ltype, ltyp_obj->value.s_data, DXF_MAX_CHARS);
					str_upp(ltype);
					strncpy(old_name, drawing->ltypes[idx].name, DXF_MAX_CHARS);
					str_upp(old_name);
					/* verify if is related to modified ltype */
					if(strcmp(ltype, old_name) == 0){
						/* change the ltype name */
						dxf_attr_change(current, 6, new_name);
					}
				}
				/* search also in sub elements */
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content;
					continue;
				}
			}
				
			current = current->next; /* go to the next in the list*/
			
			if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
				current = NULL;
				break;
			}

			/* ============================================================= */
			while (current == NULL){
				/* end of list sweeping */
				if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
					current = NULL;
					break;
				}
				/* try to back in structure hierarchy */
				prev = prev->master;
				if (prev){ /* up in structure */
					/* try to continue on previous point in structure */
					current = prev->next;
					
				}
				else{ /* stop the search if structure ends */
					current = NULL;
					break;
				}
			}
		}
	}
	
	/* finally, change ltype's struct */
	dxf_attr_change(drawing->ltypes[idx].obj, 2, new_name);
	strncpy (drawing->ltypes[idx].name, new_name, DXF_MAX_CHARS);
	return ok;
}

int ltype_use(dxf_drawing *drawing){
	/* count drawing elements related to ltype */
	int ok = 0, i, idx;
	dxf_node *current, *prev, *obj = NULL, *list[3], *ltyp_obj;
	
	list[0] = NULL; list[1] = NULL; list[2] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
		list[2] = drawing->t_layer;
	}
	else return 0;
	
	for (i = 0; i < drawing->num_ltypes; i++){ /* clear all ltypes */
		drawing->ltypes[i].num_el = 0;
	}
	
	for (i = 0; i< 3; i++){ /* look in BLOCKS and ENTITIES sections, and in LAYERS table too */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section */
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				ltyp_obj = dxf_find_attr2(current, 6); /* get element's ltype */
				if (ltyp_obj){
					/* get ltype index */
					idx = dxf_ltype_idx(drawing, ltyp_obj->value.s_data);
					/* and update its counting */
					drawing->ltypes[idx].num_el++;
					
				}
				/* search also in sub elements */
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content;
					continue;
				}
			}
				
			current = current->next; /* go to the next in the list*/
			
			if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
				current = NULL;
				break;
			}

			/* ============================================================= */
			while (current == NULL){
				/* end of list sweeping */
				if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
					//printf("para\n");
					current = NULL;
					break;
				}
				/* try to back in structure hierarchy */
				prev = prev->master;
				if (prev){ /* up in structure */
					/* try to continue on previous point in structure */
					current = prev->next;
					
				}
				else{ /* stop the search if structure ends */
					current = NULL;
					break;
				}
			}
		}
	}
	
	return ok;
}

int ltype_prop(gui_obj *gui){
	/* show ltypes and its parameters in combo box */
	int num_ltypes = gui->drawing->num_ltypes;
	int i;
	int h = num_ltypes * 25 + 5;
	h = (h < 200)? h : 200;
	
	if (nk_combo_begin_label(gui->ctx, gui->drawing->ltypes[gui->ltypes_idx].name, nk_vec2(300, h))){
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		
		for (i = 0; i < num_ltypes; i++){
			
			if (nk_button_label(gui->ctx, gui->drawing->ltypes[i].name)){
				gui->ltypes_idx = i;
				gui->action = LTYPE_CHANGE;
				nk_combo_close(gui->ctx);
				break;
			}
			nk_label(gui->ctx, gui->drawing->ltypes[i].descr, NK_TEXT_LEFT);
		}
		
		nk_combo_end(gui->ctx);
	}
	
	return 1;
}