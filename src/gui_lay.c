#include "gui_lay.h"

/* ---------------------  auxiliary functions for sorting files (qsort) ------------ */
/* compare by layer name */
int cmp_layer_name(const void * a, const void * b) {
	char *name1, *name2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	/* copy strings for secure manipulation */
	strncpy(copy1, lay1->name, DXF_MAX_CHARS);
	strncpy(copy2, lay2->name, DXF_MAX_CHARS);
	/* remove trailing spaces */
	name1 = trimwhitespace(copy1);
	name2 = trimwhitespace(copy2);
	/* change to upper case */
	str_upp(name1);
	str_upp(name2);
	return (strncmp(name1, name2, DXF_MAX_CHARS));
}

int cmp_layer_name_rev(const void * a, const void * b) {
	return -cmp_layer_name(a, b);
}

/* compare by line type name */
int cmp_layer_ltype(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	/* copy strings for secure manipulation */
	strncpy(copy1, lay1->ltype, DXF_MAX_CHARS);
	strncpy(copy2, lay2->ltype, DXF_MAX_CHARS);
	/* remove trailing spaces */
	ltype1 = trimwhitespace(copy1);
	ltype2 = trimwhitespace(copy2);
	/* change to upper case */
	str_upp(ltype1);
	str_upp(ltype2);
	ret = strncmp(ltype1, ltype2, DXF_MAX_CHARS);
	if (ret == 0) { /* in case the line type is the same, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_ltype_rev(const void * a, const void * b) {
	return -cmp_layer_ltype(a, b);
}

/* compare by color */
int cmp_layer_color(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = lay1->color - lay2->color;
	if (ret == 0) { /* in case the color is the same, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_color_rev(const void * a, const void * b) {
	return -cmp_layer_color(a, b);
}

/* compare by line weight */
int cmp_layer_lw(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = lay1->line_w - lay2->line_w;
	if (ret == 0) { /* in case the Line weight is the same, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_lw_rev(const void * a, const void * b) {
	return -cmp_layer_lw(a, b);
}

/* compare by layer in use flag */
int cmp_layer_use(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = (lay1->num_el > 0) - (lay2->num_el > 0);
	if (ret == 0) { /* in case both layers in use, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_use_rev(const void * a, const void * b) {
	return -cmp_layer_use(a, b);
}

/* compare by layer on/off flag */
int cmp_layer_off(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = (lay2->off > 0) - (lay1->off > 0);
	if (ret == 0) { /* in case both layers are off, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_off_rev(const void * a, const void * b) {
	return -cmp_layer_off(a, b);
}

/* compare by layer freeze flag */
int cmp_layer_freeze(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = (lay2->frozen > 0) - (lay1->frozen > 0);
	if (ret == 0) { /* in case both layers are frozen, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_freeze_rev(const void * a, const void * b) {
	return -cmp_layer_freeze(a, b);
}

/* compare by layer lock flag */
int cmp_layer_lock(const void * a, const void * b) {
	char *ltype1, *ltype2;
	char copy1[DXF_MAX_CHARS], copy2[DXF_MAX_CHARS];
	int ret;
	
	dxf_layer *lay1 = ((struct sort_by_idx *)a)->data;
	dxf_layer *lay2 = ((struct sort_by_idx *)b)->data;
	
	ret = (lay2->lock > 0) - (lay1->lock > 0);
	if (ret == 0) { /* in case both layers are locked, sort by layer name */
		return cmp_layer_name(a, b);
	}
	return ret;
}
int cmp_layer_lock_rev(const void * a, const void * b) {
	return -cmp_layer_lock(a, b);
}

/*-------------------------------------------------------------------------------------*/


/* layer manager window */
int lay_mng (gui_obj *gui){
	int i, show_lay_mng = 1;
	static int show_color_pick = 0, show_lay_name = 0;
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 670;
	gui->next_win_h = 310;
	
	enum lay_op {
		LAY_OP_NONE,
		LAY_OP_CREATE,
		LAY_OP_RENAME,
		LAY_OP_UPDATE
	};
	static int lay_change = LAY_OP_NONE;
	
	if (nk_begin(gui->ctx, "Layer Manager", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		static char lay_name[DXF_MAX_CHARS] = "";
		int lay_exist = 0;
		
		dxf_layer *layers = gui->drawing->layers;
		dxf_ltype *ltypes = gui->drawing->ltypes;
		int num_layers = gui->drawing->num_layers;
		
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
		
		static struct sort_by_idx sort_lay[DXF_MAX_LAYERS];
		
		layer_use(gui->drawing); /* update layers in use*/
		
		/* construct list for sorting */
		for (i = 0; i < num_layers; i++){
			sort_lay[i].idx = i;
			sort_lay[i].data = &(layers[i]);
		}
		
		/* execute sort, according sorting criteria */
		if (sorted == BY_NAME){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_name);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_name_rev);
		}
		else if (sorted == BY_LTYPE){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_ltype);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_ltype_rev);
		}
		else if (sorted == BY_COLOR){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_color);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_color_rev);
		}
		else if (sorted == BY_LW){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_lw);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_lw_rev);
		}
		else if (sorted == BY_USE){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_use);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_use_rev);
		}
		else if (sorted == BY_OFF){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_off);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_off_rev);
		}
		else if (sorted == BY_FREEZE){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_freeze);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_freeze_rev);
		}
		else if (sorted == BY_LOCK){
			if(!sort_reverse)
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_lock);
			else
				qsort(sort_lay, num_layers, sizeof(struct sort_by_idx), cmp_layer_lock_rev);
		}
		
		static int sel_lay = -1;
		int lw_i, j, sel_ltype, lay_idx;
		
		char str_tmp[DXF_MAX_CHARS];
		
		dxf_node *lay_flags = NULL;
		
		/* list header */
		nk_layout_row_dynamic(gui->ctx, 32, 1);
		if (nk_group_begin(gui->ctx, "Lay_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* buttons to change sorting criteria */
			
			nk_layout_row(gui->ctx, NK_STATIC, 22, 8, (float[]){175, 20, 20, 20, 20, 175, 100, 50});
			/* sort by Layer name */
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
			/* sort by color */
			if (sorted == BY_COLOR){
				if (sort_reverse){
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_UP)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "C")){
				sorted = BY_COLOR;
				sort_reverse = 0;
			}
			/* sort by layer on/off */
			if (sorted == BY_OFF){
				if (sort_reverse){
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_UP)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EYE]))){
				sorted = BY_OFF;
				sort_reverse = 0;
			}
			/* sort by layer freeze*/
			if (sorted == BY_FREEZE){
				if (sort_reverse){
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_UP)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SUN]))){
				sorted = BY_FREEZE;
				sort_reverse = 0;
			}
			/* sort by layer lock*/
			if (sorted == BY_LOCK){
				if (sort_reverse){
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_UP)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LOCK]))){
				sorted = BY_LOCK;
				sort_reverse = 0;
			}
			/* sort by Line pattern */
			if (sorted == BY_LTYPE){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Line type", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Line type", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Line type")){
				sorted = BY_LTYPE;
				sort_reverse = 0;
			}
			/* sort by Line weight */
			if (sorted == BY_LW){
				if (sort_reverse){
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_DOWN, "Line weight", NK_TEXT_CENTERED)){
						sorted = UNSORTED;
						sort_reverse = 0;
					}
				} else {
					if (nk_button_symbol_label(gui->ctx, NK_SYMBOL_TRIANGLE_UP, "Line weight", NK_TEXT_CENTERED)){
						sort_reverse = 1;
					}
				}
			}else if (nk_button_label(gui->ctx, "Line weight")){
				sorted = BY_LW;
				sort_reverse = 0;
			}
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
		if (nk_group_begin(gui->ctx, "Lay_view", NK_WINDOW_BORDER)) {
			nk_layout_row(gui->ctx, NK_STATIC, 20, 8, (float[]){175, 20, 20, 20, 20, 175, 100, 50});
			for (i = 0; i < num_layers; i++){ /* sweep list of layers */
				/* show and change current layer parameters */
				lay_idx = sort_lay[i].idx; /* current layer */
				/* select/deselect layer */
				if (sel_lay == lay_idx){
					if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, layers[lay_idx].name)){
						sel_lay = -1;
					}
				}
				else {
					if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, layers[lay_idx].name)){
						sel_lay = lay_idx;
					}
				}
				/* layer's default color */
				struct nk_color b_color = {
					.r = dxf_colors[layers[lay_idx].color].r,
					.g = dxf_colors[layers[lay_idx].color].g,
					.b = dxf_colors[layers[lay_idx].color].b,
					.a = dxf_colors[layers[lay_idx].color].a
				};
				if(nk_button_color(gui->ctx, b_color)){
					/* to change color, open a popup */
					show_color_pick = 1;
					sel_lay = lay_idx;
				}
				/* layer on/off */
				if (layers[lay_idx].off){
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NO_EYE]))){
						layers[lay_idx].off = 0;
						/* layer is on when its color is positive */
						dxf_attr_change(layers[lay_idx].obj, 62, (int []){ abs(layers[lay_idx].color) });
					}
				}
				else{
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EYE]))){
						layers[lay_idx].off = 1;
						/* layer is off when its color is negative */
						dxf_attr_change(layers[lay_idx].obj, 62, (int []){ -1 * abs(layers[lay_idx].color) });
					}
				}
				/* freeze layer */
				if (layers[lay_idx].frozen){
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FREEZE]))){
						layers[lay_idx].frozen = 0;
						/* look for layer flags */
						lay_flags =  dxf_find_attr2(layers[lay_idx].obj, 70);
						if (lay_flags){ /* change corresponding bit in flags word */
							lay_flags->value.i_data &= ~(1 << 0);
						}
						
					}
				}
				else{
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SUN]))){
						layers[lay_idx].frozen= 1;
						/* look for layer flags */
						lay_flags =  dxf_find_attr2(layers[lay_idx].obj, 70);
						if (lay_flags){ /* change corresponding bit in flags word */
							lay_flags->value.i_data |= (1 << 0);
						}
					}
				}
				/* lock layer */
				if (layers[lay_idx].lock){
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LOCK]))){
						layers[lay_idx].lock = 0;
						/* look for layer flags */
						lay_flags =  dxf_find_attr2(layers[lay_idx].obj, 70);
						if (lay_flags){ /* change corresponding bit in flags word */
							lay_flags->value.i_data &= ~(1 << 2);
						}
					}
				}
				else{
					if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UNLOCK]))){
						layers[lay_idx].lock = 1;
						/* look for layer flags */
						lay_flags =  dxf_find_attr2(layers[lay_idx].obj, 70);
						if (lay_flags){ /* change corresponding bit in flags word */
							lay_flags->value.i_data |= (1 << 2);
						}
					}
				}
				
				
				sel_ltype = -1;
				
				/* layer's default line pattern */
				int num_ltypes = gui->drawing->num_ltypes;
				int h = num_ltypes * 25 + 5;
				h = (h < 200)? h : 200;
				
				if (nk_combo_begin_label(gui->ctx, layers[lay_idx].ltype, nk_vec2(300, h))){
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					
					/* show available line patterns */
					for (j = 0; j < num_ltypes; j++){
						strncpy(str_tmp, ltypes[j].name, DXF_MAX_CHARS);
						str_upp(str_tmp);
						
						/* skip unauthorized line types for layers */
						if (strlen(str_tmp) == 0) continue;
						if (strcmp(str_tmp, "BYBLOCK") == 0) continue;
						if (strcmp(str_tmp, "BYLAYER") == 0) continue;
						
						/* line type name*/
						if (nk_button_label(gui->ctx, ltypes[j].name)){ /* change line type*/
							sel_ltype = j;
							nk_combo_close(gui->ctx);
						}
						/* show line type short description */
						nk_label(gui->ctx, ltypes[j].descr, NK_TEXT_LEFT);
					}
					nk_combo_end(gui->ctx);
				}
				
				if (sel_ltype >= 0){
					strncpy(layers[lay_idx].ltype, ltypes[sel_ltype].name, DXF_MAX_CHARS);
					
					dxf_attr_change(layers[lay_idx].obj, 6, layers[lay_idx].ltype);
				
				}
				
				/* layer's default line width */
				/* look for lw index */
				lw_i = 0;
				for (j = 0; j < DXF_LW_LEN; j++){
					if (dxf_lw[j] == layers[lay_idx].line_w){
						lw_i = j;
						break;
					}
				}
				/* show line width description */
				lw_i = nk_combo(gui->ctx, dxf_lw_descr, DXF_LW_LEN, lw_i, 15, nk_vec2(100,205));
				if (layers[lay_idx].line_w != dxf_lw[lw_i]){ /* change lw */
					dxf_attr_change(layers[lay_idx].obj, 370, &(dxf_lw[lw_i]));
					layers[lay_idx].line_w = dxf_lw[lw_i];
				}
				
				/* show if current layer is in use */
				if (layers[lay_idx].num_el)
					nk_label(gui->ctx, "x",  NK_TEXT_CENTERED);
				else nk_label(gui->ctx, " ",  NK_TEXT_CENTERED);
			}
			nk_group_end(gui->ctx);
		} /* list end */

		
		nk_layout_row_dynamic(gui->ctx, 20, 3);
		/* create a new layer */
		if (nk_button_label(gui->ctx, "Create")){
			/* open a popup for entering the layer name */
			show_lay_name = 1;
			lay_name[0] = 0;
			lay_change = LAY_OP_CREATE;
		}
		/* rename selected layer */
		if ((nk_button_label(gui->ctx, "Rename")) && (sel_lay >= 0)){
			/* open a popup for entering the layer name */
			show_lay_name = 1;
			strncpy(lay_name, layers[sel_lay].name, DXF_MAX_CHARS);
			lay_change = LAY_OP_RENAME;
			
		}
		/* delete selected layer */
		if ((nk_button_label(gui->ctx, "Remove")) && (sel_lay >= 0)){
			/* update all layers for sure */
			layer_use(gui->drawing);
			/* don't remove layer in use */
			if (layers[sel_lay].num_el){ 
				snprintf(gui->log_msg, 63, "Error: Don't remove Layer in use");
			}
			else{
				/* remove layer from main structure */
				do_add_entry(&gui->list_do, "Remove Layer");
				do_add_item(gui->list_do.current, layers[sel_lay].obj, NULL);
				dxf_obj_subst(layers[sel_lay].obj, NULL);
				sel_lay = -1;
				/* update the drawing */
				dxf_layer_assemb (gui->drawing);
			}
		}
		
		/* popup to change layer's color */
		if ((show_color_pick) && (sel_lay >= 0)){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Layer Color", NK_WINDOW_CLOSABLE, nk_rect(220, 10, 220, 300))){
				
				int j;
				nk_layout_row_static(gui->ctx, 15, 15, 10);
				struct nk_color b_color;
				nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
				/* show available colors */
				for (j = 1; j < 256; j++){
					
					b_color.r = dxf_colors[j].r;
					b_color.g = dxf_colors[j].g;
					b_color.b = dxf_colors[j].b;
					b_color.a = dxf_colors[j].a;
					
					if(nk_button_color(gui->ctx, b_color)){ /* pick color */
						/* change layer color */
						layers[sel_lay].color = j;
						dxf_attr_change(layers[sel_lay].obj, 62, &j);
						nk_popup_close(gui->ctx);
						show_color_pick = 0;
					}
				}
				nk_popup_end(gui->ctx);
			} else show_color_pick = 0;
		}
		
		/* popup to entering layer name (new and rename) */
		if ((show_lay_name)){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Layer Name", NK_WINDOW_CLOSABLE, nk_rect(10, 20, 220, 100))){
				
				/* get name */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, lay_name, DXF_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "OK")){
					/* try to create a new layer */
					if (lay_change == LAY_OP_CREATE){
						if (!dxf_new_layer (gui->drawing, lay_name, 7, ltypes[dxf_ltype_idx (gui->drawing, "Continuous")].name)){
							/* fail to  create, commonly name already exists */
							snprintf(gui->log_msg, 63, "Error: Layer already exists");
						}
						else {
							/* layer created and attached in drawing main structure */
							nk_popup_close(gui->ctx);
							show_lay_name = 0;
							lay_change = LAY_OP_UPDATE;
						}
					}
					else if ((lay_change == LAY_OP_RENAME) && (sel_lay >= 0)){
						/* verify if name already exists*/
						lay_exist = 0;
						for (i = 0; i < num_layers; i++){
							if (i != sel_lay){ /*except current layer*/
								if(strcmp(layers[i].name, lay_name) == 0){
									lay_exist = 1;
									break;
								}
							}
						}
						if (!lay_exist){ /* change layer name */
							layer_rename(gui->drawing, sel_lay, lay_name); /* update all related elements in drawing */
							nk_popup_close(gui->ctx);
							show_lay_name = 0;
						}
						else snprintf(gui->log_msg, 63, "Error: exists Layer with same name");
					}
	/* ------------ cancel or close window ---------------- */
					else {
						nk_popup_close(gui->ctx);
						show_lay_name = 0;
						lay_change = LAY_OP_NONE;
					}
				}
				if (nk_button_label(gui->ctx, "Cancel")){
					nk_popup_close(gui->ctx);
					show_lay_name = 0;
					lay_change = LAY_OP_NONE;
				}
				nk_popup_end(gui->ctx);
			} else {
				show_lay_name = 0;
				lay_change = LAY_OP_NONE;
			}
		}
	} else {
		show_lay_mng = 0;
		lay_change = LAY_OP_UPDATE;
	}
	nk_end(gui->ctx);
	
	return show_lay_mng;
}

int layer_rename(dxf_drawing *drawing, int idx, char *name){
	/* rename existing layer -  update all related elements in drawing */
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[2], *lay_obj;
	char *new_name = trimwhitespace(name);
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	/* first, update all related elements in drawing */
	for (i = 0; i< 2; i++){ /* look in BLOCKS and ENTITIES sections */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section*/
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				lay_obj = dxf_find_attr2(current, 8); /* get element's layer */
				if (lay_obj){
					char layer[DXF_MAX_CHARS], old_name[DXF_MAX_CHARS];
					strncpy(layer, lay_obj->value.s_data, DXF_MAX_CHARS);
					str_upp(layer);
					strncpy(old_name, drawing->layers[idx].name, DXF_MAX_CHARS);
					str_upp(old_name);
					/* verify if is related to modified layer */
					if(strcmp(layer, old_name) == 0){
						/* change the layer name */
						dxf_attr_change(current, 8, new_name);
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
	
	/* finally, change layer's struct */
	dxf_attr_change(drawing->layers[idx].obj, 2, new_name);
	strncpy (drawing->layers[idx].name, new_name, DXF_MAX_CHARS);
	return ok;
}

int layer_use(dxf_drawing *drawing){
	/* count drawing elements related to layer */
	int ok = 0, i, idx;
	dxf_node *current, *prev, *obj = NULL, *list[2], *lay_obj;
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	for (i = 0; i < drawing->num_layers; i++){ /* clear all layers */
		drawing->layers[i].num_el = 0;
	}
	
	for (i = 0; i< 2; i++){ /* look in BLOCKS and ENTITIES sections */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section */
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				lay_obj = dxf_find_attr2(current, 8); /* get element's layer */
				if (lay_obj){
					/* get layer index */
					idx = dxf_lay_idx(drawing, lay_obj->value.s_data);
					/* and update its counting */
					drawing->layers[idx].num_el++;
					
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


int layer_prop(gui_obj *gui){
	/* show layers and its parameters in combo box */
	int num_layers = gui->drawing->num_layers;
	
	int h = num_layers * 25 + 5;
	h = (h < 300)? h : 300;
	
	if (nk_combo_begin_label(gui->ctx, gui->drawing->layers[gui->layer_idx].name, nk_vec2(300,h))){
		int i;
		float wid[] = {175, 20, 20, 20, 20};
		nk_layout_row(gui->ctx, NK_STATIC, 20, 5, wid);
		
		dxf_layer *layers = gui->drawing->layers;
		dxf_ltype *ltypes = gui->drawing->ltypes;
		
		
		dxf_node *lay_flags = NULL;
		
		/* available layers */
		for (i = 0; i < num_layers; i++){
			/* layer name */
			if (nk_button_label(gui->ctx, gui->drawing->layers[i].name)){
				/* change current layer - also change selected objects */
				gui->layer_idx = i;
				gui->action = LAYER_CHANGE;
				nk_combo_close(gui->ctx);
			}
			
			/* layer color */
			struct nk_color b_color = {
				.r = dxf_colors[gui->drawing->layers[i].color].r,
				.g = dxf_colors[gui->drawing->layers[i].color].g,
				.b = dxf_colors[gui->drawing->layers[i].color].b,
				.a = dxf_colors[gui->drawing->layers[i].color].a
			};
			if(nk_button_color(gui->ctx, b_color)){
				
			}
			
			/* toggle on/off */
			if (gui->drawing->layers[i].off){
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NO_EYE]))){
					gui->drawing->layers[i].off = 0;
					dxf_attr_change(gui->drawing->layers[i].obj, 62, (int []){ abs(gui->drawing->layers[i].color) });
				}
			}
			else{
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EYE]))){
					gui->drawing->layers[i].off = 1;
					dxf_attr_change(gui->drawing->layers[i].obj, 62, (int []){ -1 * abs(gui->drawing->layers[i].color) });
				}
			}
			/* toggle freeze/solarize*/
			if (gui->drawing->layers[i].frozen){
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FREEZE]))){
					gui->drawing->layers[i].frozen = 0;
					lay_flags =  dxf_find_attr2(gui->drawing->layers[i].obj, 70);
					if (lay_flags){
						lay_flags->value.i_data &= ~(1 << 0);
					}
				}
			}
			else{
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SUN]))){
					gui->drawing->layers[i].frozen= 1;
					lay_flags =  dxf_find_attr2(gui->drawing->layers[i].obj, 70);
					if (lay_flags){
						lay_flags->value.i_data |= (1 << 0);
					}
				}
			}
			/* toggle lock/unlock*/
			if (gui->drawing->layers[i].lock){
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LOCK]))){
					gui->drawing->layers[i].lock = 0;
					lay_flags =  dxf_find_attr2(gui->drawing->layers[i].obj, 70);
					if (lay_flags){
						lay_flags->value.i_data &= ~(1 << 2);
					}
				}
			}
			else{
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UNLOCK]))){
					gui->drawing->layers[i].lock = 1;
					lay_flags =  dxf_find_attr2(gui->drawing->layers[i].obj, 70);
					if (lay_flags){
						lay_flags->value.i_data |= (1 << 2);
					}
				}
			}
			
		}
		nk_combo_end(gui->ctx);
	}
	
	return 1;
}