#include "gui_use.h"

int gui_text_interactive(gui_obj *gui){
	if (gui->modal != TEXT) return 0;
	static dxf_node *new_el;
	if (gui->step == 0){
		gui->draw_tmp = 1;
		/* create a new DXF text candidate */
		new_el = (dxf_node *) dxf_new_text (
			gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->txt_h, /* pt1, height */
			gui->txt, /* text, */
			gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
			gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
			0, ONE_TIME); /* paper space */
		gui->element = new_el;
		dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
		dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
		dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
		gui->step = 1;
		gui_next_step(gui);
	}
	else{
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){ /* place and confirm  text creation */
			/* copy text entity to permanent memory pool */
			new_el = dxf_ent_copy(new_el, DWG_LIFE);
			dxf_mem_pool(ZERO_DXF, ONE_TIME);
			/* apply text parameters */
			dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
			dxf_attr_change_i(new_el, 11, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_el, 21, &gui->step_y[gui->step], -1);
			dxf_attr_change(new_el, 40, &gui->txt_h);
			dxf_attr_change(new_el, 1, gui->txt);
			dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
			dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
			dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			
			/* attach to drawing */
			new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , DWG_LIFE);
			drawing_ent_append(gui->drawing, new_el);
			
			/* undo/redo list */
			do_add_entry(&gui->list_do, _l("TEXT"));
			do_add_item(gui->list_do.current, NULL, new_el);
			
			gui->step_x[gui->step - 1] = gui->step_x[gui->step];
			gui->step_y[gui->step - 1] = gui->step_y[gui->step];
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){ /* cancel text creation */
			gui_default_modal(gui);
		}
		if (gui->ev & EV_MOTION){
			/* modify text parameters */
			dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
			dxf_attr_change_i(new_el, 11, &gui->step_x[gui->step], -1);
			dxf_attr_change_i(new_el, 21, &gui->step_y[gui->step], -1);
			dxf_attr_change(new_el, 40, &gui->txt_h);
			dxf_attr_change(new_el, 1, gui->txt);
			dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
			dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
			dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
			dxf_attr_change(new_el, 62, &gui->color_idx);
			dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
			dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
			dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			double angle = gui->angle;
			if (angle <= 0.0) angle = 360.0 - angle;
			angle = fmod(angle, 360.0);
			dxf_attr_change(new_el, 50, &angle);
			/* generate current graph to show */
			new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , FRAME_LIFE);
		}
	}
	
	return 1;
}

int gui_text_info (gui_obj *gui){
	if (gui->modal != TEXT) return 0;
	
	/* get available text styles */
	int num_tstyles = gui->drawing->num_tstyles;
	dxf_tstyle *t_sty = gui->drawing->text_styles;
	if (gui->t_sty_idx >= num_tstyles) gui->t_sty_idx = 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place an text"), NK_TEXT_LEFT);
	
	nk_layout_row_template_begin(gui->ctx, 20);
	nk_layout_row_template_push_static(gui->ctx, 45);
	nk_layout_row_template_push_dynamic(gui->ctx);
	nk_layout_row_template_end(gui->ctx);
	nk_label(gui->ctx, _l("Style:"), NK_TEXT_LEFT);
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
	nk_label(gui->ctx, _l("Text:"), NK_TEXT_LEFT);
	/* string to text entiy */
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->txt, DXF_MAX_CHARS, nk_filter_default);
	/* size and rotation parameters */
	gui->txt_h = nk_propertyd(gui->ctx, _l("Height"), 1e-9, gui->txt_h, 1e9, SMART_STEP(gui->txt_h), SMART_STEP(gui->txt_h));
	gui->angle = nk_propertyd(gui->ctx, _l("Angle"), -180.0, gui->angle, 180.0, 0.1, 0.1f);
	/* text aligment parameters */
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	gui->t_al_v = nk_combo(gui->ctx, text_al_v, T_AL_V_LEN, gui->t_al_v, 20, nk_vec2(100,105));
	gui->t_al_h = nk_combo(gui->ctx, text_al_h, T_AL_H_LEN, gui->t_al_h, 20, nk_vec2(100,105));
	
	return 1;
}