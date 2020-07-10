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

int gui_txt_prop_info (gui_obj *gui){
	/* view and modify properties of TEXT entities in selection */
	if (gui->modal != TXT_PROP) return 0;
	static dxf_node *ent = NULL;
	static char style[DXF_MAX_CHARS+1];
	static int t_al_v = 0, t_al_h = 0, sty_i = 0;
	static int en_sty = 0, en_al_v = 0, en_al_h = 0, en_h = 0, en_ang = 0;
	static double txt_h, ang;
	
	dxf_node *tmp;
	
	char tmp_str[DXF_MAX_CHARS+1];
	int i, j, h;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Edit Text Properties", NK_TEXT_LEFT);
	if (gui->step == 0){ /* get insert element to edit */
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
		
		if(strcmp(ent->obj.name, "TEXT") == 0 ||
			strcmp(ent->obj.name, "MTEXT") == 0){
		
			/* get raw parameters directly from entity */
			if(tmp = dxf_find_attr2(ent, 7))
				strncpy (style, tmp->value.s_data, DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(ent, 40))
				txt_h = tmp->value.d_data;
			if(tmp = dxf_find_attr2(ent, 50))
				ang = tmp->value.d_data;
			if(tmp = dxf_find_attr2(ent, 72))
				t_al_h = tmp->value.i_data;
			if(tmp = dxf_find_attr2(ent, 73))
				t_al_v = tmp->value.i_data;
			
		}
		gui->step = 2;
	}
	else { /* all inited */
		/* show entity type */
		snprintf(tmp_str, DXF_MAX_CHARS, "Entity: %s", ent->obj.name);
		nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
		
		/* ----------  properties -------------*/
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		
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
		nk_checkbox_label(gui->ctx, "Vert:", &en_al_v);
		if (en_al_v){
			t_al_v = nk_combo(gui->ctx, text_al_v, T_AL_V_LEN, t_al_v, 20, nk_vec2(100,105));
		}
		else {
			nk_label_colored(gui->ctx, text_al_v[t_al_v], NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		nk_checkbox_label(gui->ctx, "Horiz:", &en_al_h);
		if (en_al_h){
			t_al_h = nk_combo(gui->ctx, text_al_h, T_AL_H_LEN, t_al_h, 20, nk_vec2(100,105));
		}
		else {
			nk_label_colored(gui->ctx, text_al_h[t_al_h], NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- text height ------------ */
		nk_checkbox_label(gui->ctx, "Height:", &en_h);
		if (en_h){
			txt_h = nk_propertyd(gui->ctx, "#", 0.0d, txt_h, 100.0d, 0.1d, 0.1d);
		}
		else {
			snprintf(tmp_str, DXF_MAX_CHARS, "%.5g", txt_h);
			nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ----------- angle ------------ */
		nk_checkbox_label(gui->ctx, "Angle:", &en_ang);
		if (en_ang){
			ang = nk_propertyd(gui->ctx, "#", 0.0d, ang, 100.0d, 0.1d, 0.1d);
		}
		else {
			snprintf(tmp_str, DXF_MAX_CHARS, "%.5g", ang);
			nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
		}
		
		/* ---------------------*/
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Modify") && ( en_sty || en_al_v || en_al_h)){
			/* change selection properties according selected parameters */
			if (gui->sel_list != NULL){
				/* sweep the selection list */
				list_node *current = gui->sel_list->next;
				dxf_node *new_ent = NULL;
				if (current != NULL){
					do_add_entry(&gui->list_do, "CHANGE TEXT PROPERTIES"); /* init do/undo list */
					gui->step = 1;
				}
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* copy current entity to preserve information to undo */
							new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
							/* change properties according options */
							if (en_sty) dxf_attr_change(new_ent, 7, gui->drawing->text_styles[sty_i].name);
							//if (en_lw) dxf_attr_change(new_ent, 370, &dxf_lw[lw_i]);
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
			//if (en_sty) gui->layer_idx = lay_i;
		}
		
		
	}

	return 1;
}