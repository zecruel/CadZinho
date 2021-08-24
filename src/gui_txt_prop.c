#include "gui_use.h"

int gui_txt_prop_interactive(gui_obj *gui){
	if (gui->modal != TXT_PROP) return 0;
	
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
		gui->sel_ent_filter = DXF_TEXT | DXF_MTEXT;
		gui_simple_select(gui);
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
			gui->step = 0;
		}
	}
	else if (gui->step >= 1){
		gui->element = NULL;
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			sel_list_clear (gui);
			gui_first_step(gui);
			gui->step = 0;
		}
	}
	return 1;
}

int gui_txt_prop_info (gui_obj *gui){
	/* view and modify properties of TEXT entities in selection */
	if (gui->modal != TXT_PROP) return 0;
	static dxf_node *ent = NULL;
	static char style[DXF_MAX_CHARS+1];
	static int t_al_v = 0, t_al_h = 0, sty_i = 0, at_pt_i = 6;
	static int en_sty = 0, en_al_v = 0, en_al_h = 0, en_h = 0, en_ang = 0, en_rec = 0;
	static double txt_h, ang, rec_w;
	
	/* alingment options */
	int at_pt_h[] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
	int at_pt_v[] = {3, 3, 3, 2, 2, 2, 1, 1, 1};
	
	dxf_node *tmp;
	
	char tmp_str[DXF_MAX_CHARS+1];
	int i, j, h;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Edit Text Properties", NK_TEXT_LEFT);
	if (gui->step == 0){ /* get elements to edit */
		nk_label(gui->ctx, "Select a element", NK_TEXT_LEFT);
	}
	else if (gui->step == 1){ /* init the interface */
		/* get information of first element in selection list */
		ent = gui->sel_list->next->data;
		
		/* clear variables */
		style[0] = 0;
		t_al_v = 0;
		t_al_h = 0;
		txt_h = 1.0;
		ang = 0.0;
		at_pt_i = 6;
		rec_w = 0.0;
		
		/* try to look parameters indexes from drawing lists */
		sty_i = dxf_tstyle_get(gui->drawing, ent);
		if (sty_i < 0) sty_i = 0; /* error on look for style */
		
		if(strcmp(ent->obj.name, "TEXT") == 0 ||
			strcmp(ent->obj.name, "MTEXT") == 0){
		
			/* get raw parameters directly from entity */
			if(tmp = dxf_find_attr2(ent, 7))
				strncpy (style, tmp->value.s_data, DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(ent, 40))
				txt_h = tmp->value.d_data;
			if(tmp = dxf_find_attr2(ent, 50))
				ang = tmp->value.d_data;
			if(strcmp(ent->obj.name, "TEXT") == 0){
				/* TEXT parameters */
				if(tmp = dxf_find_attr2(ent, 72))
					t_al_h = tmp->value.i_data;
				if(tmp = dxf_find_attr2(ent, 73))
					t_al_v = tmp->value.i_data;
			}
			else{
				/* MTEXT parameters */
				
				/* get atachment point and convert to alingment options */
				if(tmp = dxf_find_attr2(ent, 71))
					at_pt_i = tmp->value.i_data - 1;
				if (at_pt_i > 8 || at_pt_i < 0) at_pt_i = 6;
				t_al_h = at_pt_h[at_pt_i];
				t_al_v = at_pt_v[at_pt_i];
				
				/* get angle from vector */
				double x = 1.0, y = 0.0;
				if(tmp = dxf_find_attr2(ent, 11)){
					x = tmp->value.d_data;
					if (tmp = dxf_find_attr2(ent, 21))
						y = tmp->value.d_data;
					
					ang = atan2(y, x) * 180.0 / M_PI;
				}
				
				/* get rectangle width */
				if(tmp = dxf_find_attr2(ent, 41))
					rec_w = tmp->value.d_data;
			}
			
		}
		gui->step = 2;
	}
	else { /* all inited */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		
		if(strcmp(ent->obj.name, "TEXT") == 0 ||
			strcmp(ent->obj.name, "MTEXT") == 0)
		{ /* show entity type */
			nk_label(gui->ctx, "Type:", NK_TEXT_RIGHT);
			nk_label_colored(gui->ctx, ent->obj.name, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------  properties -------------*/
		
		/* ----------  style -------------*/
		nk_checkbox_label(gui->ctx, "Style:", &en_sty);
		int num_tstyles = gui->drawing->num_tstyles;
		dxf_tstyle *t_sty = gui->drawing->text_styles;
		/* text style combo selection */
		h = num_tstyles * 25 + 5;
		h = (h < 200)? h : 200;
		if (en_sty){ /* enable editing */
			if (nk_combo_begin_label(gui->ctx,  t_sty[sty_i].name, nk_vec2(220, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				/* show available text styles in drawing */
				for (j = 0; j < num_tstyles; j++){
					/* skip unauthorized styles */
					if (strlen(t_sty[j].name) == 0) continue;
					
					if (nk_button_label(gui->ctx, t_sty[j].name)){
						sty_i = j; /* select current style */
						nk_combo_close(gui->ctx);
						break;
					}
				}
				nk_combo_end(gui->ctx);
			}
		}
		else { /* only show information */
			nk_label_colored(gui->ctx, style, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- alignment ------------ */
		/* vertical alignment */
		nk_checkbox_label(gui->ctx, "Vert:", &en_al_v);
		if (en_al_v){
			t_al_v = nk_combo(gui->ctx, text_al_v, T_AL_V_LEN, t_al_v, 20, nk_vec2(100,105));
		}
		else { /* only show information */
			nk_label_colored(gui->ctx, text_al_v[t_al_v], NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		/* horizontal alignment */
		nk_checkbox_label(gui->ctx, "Horiz:", &en_al_h);
		if (en_al_h){
			t_al_h = nk_combo(gui->ctx, text_al_h, T_AL_H_LEN, t_al_h, 20, nk_vec2(100,105));
		}
		else { /* only show information */
			nk_label_colored(gui->ctx, text_al_h[t_al_h], NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- text height ------------ */
		nk_checkbox_label(gui->ctx, "Height:", &en_h);
		if (en_h){
			txt_h = nk_propertyd(gui->ctx, "#", 1.0e-9, txt_h, 1.0e9, SMART_STEP(txt_h), SMART_STEP(txt_h));
		}
		else {/* only show information */
			snprintf(tmp_str, DXF_MAX_CHARS, "%.5g", txt_h);
			nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- angle ------------ */
		nk_checkbox_label(gui->ctx, "Angle:", &en_ang);
		if (en_ang){
			ang = nk_propertyd(gui->ctx, "#", 0.0, ang, 360.0, 0.1, 0.1f);
		}
		else {/* only show information */
			snprintf(tmp_str, DXF_MAX_CHARS, "%.5g", ang);
			nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- rectangle width (MTEXT only) ------------ */
		nk_checkbox_label(gui->ctx, "Rec W:", &en_rec);
		if (en_rec){
			rec_w = nk_propertyd(gui->ctx, "#", 0.0, rec_w, 1.0e9, SMART_STEP(rec_w), SMART_STEP(rec_w));
		}
		else {/* only show information */
			snprintf(tmp_str, DXF_MAX_CHARS, "%.5g", rec_w);
			nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ---------------------*/
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Modify") && ( en_sty || en_al_v || en_al_h || en_h || en_ang || en_rec)){
			/* change selection properties according selected parameters */
			if (gui->sel_list != NULL){
				int ini_do = 0;
				
				/* sweep the selection list */
				list_node *current = gui->sel_list->next;
				dxf_node *new_ent = NULL;
				while (current != NULL){
					if (current->data){
						/* check type of current entity */
						if(strcmp(((dxf_node *)current->data)->obj.name, "TEXT") == 0 ||
							strcmp(((dxf_node *)current->data)->obj.name, "MTEXT") == 0){
							/* copy current entity to preserve information to undo */
							new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
							/* change properties according options */
							if (en_sty) dxf_attr_change(new_ent, 7, gui->drawing->text_styles[sty_i].name);
							if (en_ang) dxf_attr_change(new_ent, 50, &ang);
							if (en_h) dxf_attr_change(new_ent, 40, &txt_h);
							if(strcmp(new_ent->obj.name, "TEXT") == 0){
								if (en_al_h) dxf_attr_change(new_ent, 72, &t_al_h);
								if (en_al_v) dxf_attr_change(new_ent, 73, &t_al_v);
							}
							else {
								if (en_al_h || en_al_v) {
									/* alingment options to attach point */
									int al_h = (t_al_h < 3 && t_al_h >= 0)? t_al_h : 0;
									int al_v = (t_al_v < 4 && t_al_v > 0)? t_al_v : 1;
									int attch_pt = (3 - al_v) * 3 + al_h + 1;
									dxf_attr_change(new_ent, 71, &attch_pt);
								}
								if (en_ang){
									/* angle to vector */
									double x = cos(ang * M_PI / 180.0);
									double y = sin(ang * M_PI / 180.0);
									dxf_attr_change(new_ent, 11, &x);
									dxf_attr_change(new_ent, 21, &y);
								}
								/* rectangle width */
								if (en_rec) dxf_attr_change(new_ent, 40, &rec_w);
							}
							/* update in drawing */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
							dxf_obj_subst((dxf_node *)current->data, new_ent);
							/* update undo/redo list */
							if (!ini_do){
								do_add_entry(&gui->list_do, "CHANGE TEXT PROPERTIES"); /* init do/undo list */
								gui->step = 1;
								ini_do = 1;
							}
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
			/* get selected entity properties to match main tools (place text tools) */
			if (en_sty) gui->t_sty_idx = sty_i;
			if (en_h) gui->txt_h = txt_h;
			if (en_al_v) gui->t_al_v = t_al_v;
			if (en_al_h) gui->t_al_h = t_al_h;
			if (en_ang) gui->angle = ang;
			if (en_rec) gui->rect_w = rec_w;
		}
		
		
	}

	return 1;
}