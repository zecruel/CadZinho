#include "gui_ltype.h"

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
	else scale = content.w / (line_type.length * 7.0); /* auto scale by current  line type */
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
			double x1 = x + fabs(line_type.pat[idx]) * scale;
			if (x1 > content.x + content.w) x1 = content.x + content.w;
			if (line_type.pat[idx] >= 0.0) nk_stroke_line(canvas, x, y, x1, y, 2.1, color);
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
	static int show_add = 0, show_ltyp_name = 0;
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 900;
	gui->next_win_h = 370;
	
	enum ltyp_op {
		LTYP_OP_NONE,
		LTYP_OP_ADD,
		LTYP_OP_RENAME,
		LTYP_OP_UPDATE
	};
	static int ltyp_change = LTYP_OP_NONE;
	
	dxf_ltype *ltypes = gui->drawing->ltypes;
	int num_ltypes = 0;
	
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
	for (i = 0; i < gui->drawing->num_ltypes; i++){
		strncpy(str_copy, ltypes[i].name, DXF_MAX_CHARS);
		str_upp(str_copy);
		if (!(strcmp(str_copy, "BYLAYER") == 0 || strcmp(str_copy, "BYBLOCK") == 0)){ /* skip bylayer and byblock line descriptions */
			if (ltypes[i].length > max_len) max_len = ltypes[i].length;
			
			sort_ltyp[num_ltypes].idx = i;
			sort_ltyp[num_ltypes].data = &(ltypes[i]);
			num_ltypes++;
		}
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
		int lw_i, j, sel_ltype, ltyp_idx;
		
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
		nk_layout_row_dynamic(gui->ctx, 200, 1);
		if (nk_group_begin(gui->ctx, "Ltyp_view", NK_WINDOW_BORDER)) {
			nk_layout_row(gui->ctx, NK_STATIC, 20, 4, (float[]){175, 300, 300, 50});
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
				
				double scale = 20.0;
				if (max_len > 0.0) scale = 2*max_len;
				
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
					if(preview_ltype(gui->ctx ,&gui->b_icon_sel, ltypes[ltyp_idx], scale)){
						sel_ltyp = -1;
					}
				}
				else {
					if(preview_ltype(gui->ctx ,&gui->b_icon_unsel, ltypes[ltyp_idx], scale)){
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
			/* open a popup for entering the ltype name */
			//show_ltyp_name = 1;
			//ltyp_name[0] = 0;
			//ltyp_change = LTYP_OP_ADD;
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
			
			
			dxf_ents_parse(gui->drawing); /* redraw */
			
			
			gui->draw = 1;
		}
		
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
		
		/* popup to entering ltype name (new and rename) */
		if ((show_ltyp_name)){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Line Type Name", NK_WINDOW_CLOSABLE, nk_rect(10, 20, 220, 100))){
				
				/* get name */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ltyp_name, DXF_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "OK")){
					/* try to create a new ltype */
					if (ltyp_change == LTYP_OP_ADD){
						dxf_ltype line_type;
						strncpy (line_type.name, ltyp_name, DXF_MAX_CHARS);
						
						/*TODO*/
						line_type.descr[0] = 0;
						line_type.size = 0;
						line_type.pat[0] = 0;
						line_type.length = 0.0;
						line_type.num_el = 0;
						line_type.obj = NULL;
						
						if (!dxf_new_ltype (gui->drawing, &line_type)){
							/* fail to  create, commonly name already exists */
							snprintf(gui->log_msg, 63, "Error: Line Type already exists");
						}
						else {
							/* ltype created and attached in drawing main structure */
							nk_popup_close(gui->ctx);
							show_ltyp_name = 0;
							ltyp_change = LTYP_OP_UPDATE;
						}
					}
					else if ((ltyp_change == LTYP_OP_RENAME) && (sel_ltyp >= 0)){
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
		/* edit window - allow modifications on parameters of selected text style */
		if (nk_begin(gui->ctx, "Add Line Type", nk_rect(gui->next_win_x + 150, gui->next_win_y + 100, 330, 220), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)){
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			int h = num_ltypes * 25 + 5;
			h = (h < 200)? h : 200;
			
			static int idx = 0;

			if (nk_combo_begin_label(gui->ctx, ltypes[idx].name, nk_vec2(300, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				
				for (i = 0; i < num_ltypes; i++){
					int ltyp_idx = sort_ltyp[i].idx; /* current ltype */
					if (nk_button_label(gui->ctx, ltypes[ltyp_idx].name)){
						idx = ltyp_idx;
						
						nk_combo_close(gui->ctx);
						break;
					}
					nk_label(gui->ctx, ltypes[ltyp_idx].descr, NK_TEXT_LEFT);
				}
				
				nk_combo_end(gui->ctx);
			}
			
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){100, 100});
			if (nk_button_label(gui->ctx, "OK")){
				
			}
			if (nk_button_label(gui->ctx, "Cancel")){
				show_add = 0;
			}
			
		} else {
			show_add = 0;
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