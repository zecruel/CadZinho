#include "gui_use.h"

int gui_prop_interactive(gui_obj *gui){
	if (gui->modal != PROP) return 0;
	
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step >= 1 && (!gui->sel_list->next || (gui->ev & EV_ADD))){
		/* if selection list is empty, back to first step */
		gui->step = 0;
		gui->free_sel = 1;
	}
	
	if (gui->step == 0){
		/* in first step, select the elements to proccess*/
		gui->en_distance = 0;
		gui->sel_ent_filter = ~DXF_NONE;
		gui_simple_select(gui);
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
		}
	}
	else if (gui->step >= 1){
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			sel_list_clear (gui);
			gui_first_step(gui);
			gui->step = 0;
		}
	}
	return 1;
}

int gui_prop_info (gui_obj *gui){
	/* view and modify general properties (layer, line type, color, line weight) 
	of entities in selection */
	if (gui->modal != PROP) return 0;
	static int show_color_pick = 0, ins = 0;
	static dxf_node *ent = NULL;
	static char layer[DXF_MAX_CHARS+1];
	static char ltype[DXF_MAX_CHARS+1];
	static char blk_name[DXF_MAX_CHARS+1];
	static int color, lay_i, ltyp_i, lw, lw_i;
	static int en_lay = 0, en_ltyp = 0, en_color = 0, en_lw = 0;
	dxf_node *tmp;
	
	char tmp_str[DXF_MAX_CHARS+1];
	int i, j, h;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Edit Properties", NK_TEXT_LEFT);
	if (gui->step == 0){ /* get insert element to edit */
		nk_label(gui->ctx, "Select a element", NK_TEXT_LEFT);
		show_color_pick = 0;
	}
	else if (gui->step == 1){ /* init the interface */
		/* get information of first element in selection list */
		ent = gui->sel_list->next->data;
		
		/* clear variables */
		layer[0] = 0;
		ltype[0] = 0;
		blk_name[0] = 0;
		color = 256; /* color by layer */
		lw = -1; /* line weight by layer */
		
		/* get raw parameters directly from entity */
		if(tmp = dxf_find_attr2(ent, 8))
			strncpy (layer, tmp->value.s_data, DXF_MAX_CHARS);
		if(tmp = dxf_find_attr2(ent, 6))
			strncpy (ltype, tmp->value.s_data, DXF_MAX_CHARS);
		if(tmp = dxf_find_attr2(ent, 62))
			color = abs(tmp->value.i_data);
		if(tmp = dxf_find_attr2(ent, 370))
			lw = tmp->value.i_data;
		
		/* try to look parameters indexes from drawing lists */
		/* layer index */
		lay_i = dxf_layer_get(gui->drawing, ent);
		if (lay_i < 0) lay_i = 0; /* error on look for layer */
		/* line type index */
		ltyp_i = dxf_ltype_get(gui->drawing, ent);
		if (ltyp_i < 0) ltyp_i = 0; /* error on look for line type */
		/* line weight index */
		lw_i = 0;
		for (j = 0; j < DXF_LW_LEN + 2; j++){
			if (dxf_lw[j] == lw){
				lw_i = j;
				break;
			}
		}
		
		/* verify if element is a insert */
		ins = 0;
		if(strcmp(ent->obj.name, "INSERT") == 0){
			ins = 1;
			/* get block name */
			if(tmp = dxf_find_attr2(ent, 2))
				strncpy (blk_name, tmp->value.s_data, DXF_MAX_CHARS);
		}
		
		gui->step = 2;
		show_color_pick = 0;
	}
	else { /* all inited */
		/* show entity type */
		snprintf(tmp_str, DXF_MAX_CHARS, "Entity: %s", ent->obj.name);
		nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
		
		/* ----------  properties -------------*/
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		
		/* ----------  layer -------------*/
		nk_checkbox_label(gui->ctx, "Layer:", &en_lay);
		dxf_layer *layers = gui->drawing->layers;
		int num_layers = gui->drawing->num_layers;
		h = num_layers * 25 + 5;
		h = (h < 200)? h : 200;
		if (en_lay){ /* enable editing */
			if (nk_combo_begin_label(gui->ctx, layers[lay_i].name, nk_vec2(200, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				/* show available layers in drawing */
				for (j = 0; j < num_layers; j++){
					/* skip unauthorized layers */
					if (strlen(layers[j].name) == 0) continue;
					/* layer name*/
					if (nk_button_label(gui->ctx, layers[j].name)){
						/* choose layer */
						lay_i = j;
						nk_combo_close(gui->ctx);
						break;
					}
				}
				nk_combo_end(gui->ctx);
			}
		}
		else { /* only show information */
			nk_label(gui->ctx, layer, NK_TEXT_LEFT);
		}
		
		/* ----------- line type ------------ */
		nk_checkbox_label(gui->ctx, "Ltype:", &en_ltyp); 
		dxf_ltype *ltypes = gui->drawing->ltypes;
		int num_ltypes = gui->drawing->num_ltypes;
		h = num_ltypes * 25 + 5;
		h = (h < 200)? h : 200;
		if (en_ltyp){ /* enable editing */
			if (nk_combo_begin_label(gui->ctx, ltypes[ltyp_i].name, nk_vec2(300, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				/* show available line patterns in drawing */
				for (j = 0; j < num_ltypes; j++){
					/* skip unauthorized line types*/
					if (strlen(ltypes[j].name) == 0) continue;
					/* line type name*/
					if (nk_button_label(gui->ctx, ltypes[j].name)){
						/* choose line type*/
						ltyp_i = j;
						nk_combo_close(gui->ctx);
						break;
					}
					/* show line type short description */
					nk_label(gui->ctx, ltypes[j].descr, NK_TEXT_LEFT);
				}
				nk_combo_end(gui->ctx);
			}
		}
		else { /* only show information */
			nk_label(gui->ctx, ltype, NK_TEXT_LEFT);
		}
		nk_layout_row(gui->ctx, NK_STATIC, 20, 3, (float[]){60, 20, 80});
		
		/* ----------- color ------------ */
		nk_checkbox_label(gui->ctx, "Color:", &en_color);
		int curr_color = abs(color);
		/* if is by layer, get layer color */
		if (curr_color > 255) curr_color = layers[lay_i].color;
		if (curr_color > 255) curr_color = 7;
		/* RGBA color to show */
		struct nk_color b_color = {
			.r = dxf_colors[curr_color].r,
			.g = dxf_colors[curr_color].g,
			.b = dxf_colors[curr_color].b,
			.a = dxf_colors[curr_color].a
		};
		if(nk_button_color(gui->ctx, b_color) && en_color){ /* enable editing */
			/* to change color, open a popup */
			show_color_pick = 1;
			
		}
		/* show color index (DFX color table) */
		if ( abs(color) > 255 ) snprintf(tmp_str, DXF_MAX_CHARS, "By Layer");
		else if ( abs(color) == 0 ) snprintf(tmp_str, DXF_MAX_CHARS, "By Block");
		else snprintf(tmp_str, DXF_MAX_CHARS, "%d", abs(color));
		nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
		
		
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		
		/* ----------- line weigth ------------ */
		nk_checkbox_label(gui->ctx, "LW:", &en_lw);
		if (en_lw){ /* enable editing */
			lw_i = nk_combo(gui->ctx, dxf_lw_descr, DXF_LW_LEN + 2, lw_i, 15, nk_vec2(100,205));
		}
		else{ /* only show line width description */
			nk_label(gui->ctx, dxf_lw_descr[lw_i], NK_TEXT_LEFT);
		}
		
		/* ---------------------*/
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Modify") && ( en_lay || en_ltyp || en_color || en_lw)){
			/* change selection properties according selected parameters */
			if (gui->sel_list != NULL){
				/* sweep the selection list */
				list_node *current = gui->sel_list->next;
				dxf_node *new_ent = NULL;
				if (current != NULL){
					do_add_entry(&gui->list_do, "CHANGE PROPERTIES"); /* init do/undo list */
					gui->step = 1;
				}
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* copy current entity to preserve information to undo */
							new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
							/* change properties according options */
							if (en_lay) dxf_attr_change(new_ent, 8, gui->drawing->layers[lay_i].name);
							if (en_ltyp) dxf_attr_change(new_ent, 6, gui->drawing->ltypes[ltyp_i].name);
							if (en_color) dxf_attr_change(new_ent, 62, &color);
							if (en_lw) dxf_attr_change(new_ent, 370, &dxf_lw[lw_i]);
							/* update in drawing */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
							dxf_obj_subst((dxf_node *)current->data, new_ent);
							do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
							current->data = new_ent;
						}
					}
					current = current->next;
				}
			}
			gui->draw = 1;
		}
		if (nk_button_label(gui->ctx, "Pick")){
			/* get selected entity properties to match main tools*/
			if (en_lay) gui->layer_idx = lay_i;
			if (en_ltyp) gui->ltypes_idx = ltyp_i;
			if (en_color) gui->color_idx = color;
			if (en_lw) gui->lw_idx = lw_i;
		}
		
		if (ins){ /* show insert information */
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
			nk_label(gui->ctx, "Block:", NK_TEXT_RIGHT);
			nk_label_colored(gui->ctx, blk_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
			
		}
		
		/* popup to pick color */
		if (show_color_pick){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Choose Color", NK_WINDOW_CLOSABLE, nk_rect(220, 10, 220, 300))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "By Layer")){
					color = 256;
					nk_popup_close(gui->ctx);
					show_color_pick = 0;
				}
				if (nk_button_label(gui->ctx, "By Block")){
					color = 0;
					nk_popup_close(gui->ctx);
					show_color_pick = 0;
				}
				nk_layout_row_static(gui->ctx, 15, 15, 10);
				struct nk_color b_color;
				nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
				/* show available colors */
				for (j = 1; j < 256; j++){
					b_color.r = dxf_colors[j].r;
					b_color.g = dxf_colors[j].g;
					b_color.b = dxf_colors[j].b;
					b_color.a = dxf_colors[j].a;
					
					if(nk_button_color(gui->ctx, b_color)){
						/* pick color */
						color = j;
						nk_popup_close(gui->ctx);
						show_color_pick = 0;
						break;
					}
				}
				nk_popup_end(gui->ctx);
			} else show_color_pick = 0;
		}
		
		
	}

	return 1;
}