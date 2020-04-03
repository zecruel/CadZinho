#include "gui_use.h"

int gui_attrib_interactive(gui_obj *gui){
	if (gui->modal != ADD_ATTRIB) return 0;
	static dxf_node *new_attrib;
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step == 1 && (!gui->sel_list->next)){
		/* if selection list is empty, back to first step */
		gui->step = 0;
		gui->free_sel = 1;
	}
	
	if (gui->step == 0){
		/* in first step, select the elements to proccess*/
		gui->en_distance = 0;
		gui->sel_ent_filter = DXF_INSERT;
		gui_simple_select(gui);
	}
	else if (gui->step == 1){
		
		gui->draw_tmp = 1;
		/* create a new DXF attrib candidate */
		new_attrib = dxf_new_attrib (
			gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->txt_h, /* pt1, height */
			gui->txt, "tag", /* text, */
			gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
			gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
			0, ONE_TIME); /* paper space */
		gui->element = new_attrib;
		dxf_attr_change_i(new_attrib, 72, &gui->t_al_h, -1);
		dxf_attr_change_i(new_attrib, 74, &gui->t_al_v, -1);
		dxf_attr_change(new_attrib, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
		gui->step = 2;
		gui_next_step(gui);
	}
	else{
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){ /* place and confirm  text creation */
			
			/* copy original insert entity */
			dxf_node *ins_ent = gui->sel_list->next->data;
			dxf_node *new_ins = dxf_ent_copy(ins_ent, DWG_LIFE);
			dxf_attr_change(new_ins, 66, (int[]){1});
			
			/* copy attrib entity to permanent memory pool */
			new_attrib = dxf_ent_copy(new_attrib, DWG_LIFE);
			dxf_mem_pool(ZERO_DXF, ONE_TIME);
			/* apply attrib parameters */
			dxf_attr_change_i(new_attrib, 10, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_attrib, 20, &gui->step_y[gui->step], -1);
			dxf_attr_change_i(new_attrib, 11, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_attrib, 21, &gui->step_y[gui->step], -1);
			dxf_attr_change(new_attrib, 40, &gui->txt_h);
			dxf_attr_change(new_attrib, 1, gui->txt);
			dxf_attr_change(new_attrib, 2, gui->tag);
			dxf_attr_change_i(new_attrib, 72, &gui->t_al_h, -1);
			dxf_attr_change_i(new_attrib, 74, &gui->t_al_v, -1);
			dxf_attr_change(new_attrib, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			
			ent_handle(gui->drawing, new_attrib);
			dxf_insert_append(gui->drawing, new_ins, new_attrib, DWG_LIFE);
			
			/* attach to drawing */
			new_ins->obj.graphics = dxf_graph_parse(gui->drawing, new_ins, 0 , DWG_LIFE);
			dxf_obj_subst(ins_ent, new_ins);
			
			/* undo/redo list */
			do_add_entry(&gui->list_do, "ADD TAG");
			do_add_item(gui->list_do.current, ins_ent, new_ins);
			
			gui->step_x[gui->step - 1] = gui->step_x[gui->step];
			gui->step_y[gui->step - 1] = gui->step_y[gui->step];
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){ /* cancel attrib creation */
			gui_default_modal(gui);
		}
		if (gui->ev & EV_MOTION){
			/* modify attrib parameters */
			dxf_attr_change_i(new_attrib, 10, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_attrib, 20, &gui->step_y[gui->step], -1);
			dxf_attr_change_i(new_attrib, 11, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_attrib, 21, &gui->step_y[gui->step], -1);
			dxf_attr_change(new_attrib, 40, &gui->txt_h);
			dxf_attr_change(new_attrib, 1, gui->txt);
			dxf_attr_change(new_attrib, 2, gui->tag);
			dxf_attr_change(new_attrib, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
			dxf_attr_change(new_attrib, 8, gui->drawing->layers[gui->layer_idx].name);
			dxf_attr_change(new_attrib, 370, &dxf_lw[gui->lw_idx]);
			dxf_attr_change(new_attrib, 62, &gui->color_idx);
			dxf_attr_change_i(new_attrib, 72, &gui->t_al_h, -1);
			dxf_attr_change_i(new_attrib, 74, &gui->t_al_v, -1);
			dxf_attr_change(new_attrib, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			double angle = gui->angle;
			if (angle <= 0.0) angle = 360.0 - angle;
			angle = fmod(angle, 360.0);
			dxf_attr_change(new_attrib, 50, &angle);
			/* generate current graph to show */
			new_attrib->obj.graphics = dxf_graph_parse(gui->drawing, new_attrib, 0 , FRAME_LIFE);
		}
	}
	
	return 1;
}

int gui_attrib_info (gui_obj *gui){
	if (gui->modal != ADD_ATTRIB) return 0;
	nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Add a Tag", NK_TEXT_LEFT);
	if (gui->step == 0){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Select a Insert element", NK_TEXT_LEFT);
	} else if (gui->step == 2){
		/* get available text styles */
		int num_tstyles = gui->drawing->num_tstyles;
		dxf_tstyle *t_sty = gui->drawing->text_styles;
		if (gui->t_sty_idx >= num_tstyles) gui->t_sty_idx = 0;
		
		nk_layout_row_template_begin(gui->ctx, 20);
		nk_layout_row_template_push_static(gui->ctx, 45);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_end(gui->ctx);
		nk_label(gui->ctx, "Style:", NK_TEXT_LEFT);
		/* text style combo selection */
		int h = num_tstyles * 25 + 5;
		h = (h < 200)? h : 200;
		if (nk_combo_begin_label(gui->ctx,  t_sty[gui->t_sty_idx].name, nk_vec2(220, h))){
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			int j = 0;
			for (j = 0; j < num_tstyles; j++){
				if (nk_button_label(gui->ctx, t_sty[j].name)){
					gui->t_sty_idx = j; /* select current style */
					nk_combo_close(gui->ctx);
					break;
				}
			}
			nk_combo_end(gui->ctx);
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Tag Id:", NK_TEXT_LEFT);
		/* atribute tag string */
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->tag, DXF_MAX_CHARS, nk_filter_default);
		nk_label(gui->ctx, "Value:", NK_TEXT_LEFT);
		/* atribute value string */
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->txt, DXF_MAX_CHARS, nk_filter_default);
		/* size and rotation parameters */
		gui->txt_h = nk_propertyd(gui->ctx, "Height", 0.0d, gui->txt_h, 100.0d, 0.1d, 0.1d);
		gui->angle = nk_propertyd(gui->ctx, "Angle", -180.0d, gui->angle, 180.0d, 0.1d, 0.1d);
		/* text aligment parameters */
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		gui->t_al_v = nk_combo(gui->ctx, text_al_v, T_AL_V_LEN, gui->t_al_v, 20, nk_vec2(100,105));
		gui->t_al_h = nk_combo(gui->ctx, text_al_h, T_AL_H_LEN, gui->t_al_h, 20, nk_vec2(100,105));
	}
	return 1;
}