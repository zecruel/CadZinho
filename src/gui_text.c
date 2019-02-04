#include "gui_use.h"

int gui_text_interactive(gui_obj *gui){
	if (gui->modal == TEXT){
		static dxf_node *new_el;
		if (gui->step == 0){
			gui->draw_tmp = 1;
			/* create a new DXF text */
			new_el = (dxf_node *) dxf_new_text (
				gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->txt_h, /* pt1, height */
				gui->txt, /* text, */
				gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0); /* paper space */
			gui->element = new_el;
			dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
			dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
			dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			gui->step = 1;
			goto next_step;
		}
		else{
			if (gui->ev & EV_ENTER){
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				dxf_attr_change_i(new_el, 11, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 21, &gui->step_y[gui->step], -1);
				dxf_attr_change(new_el, 40, &gui->txt_h);
				dxf_attr_change(new_el, 1, gui->txt);
				dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
				dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
				dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "TEXT");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				goto first_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
			if (gui->ev & EV_MOTION){
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
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	goto end_step;
	default_modal:
		gui->modal = SELECT;
	first_step:
		gui->en_distance = 0;
		gui->draw_tmp = 0;
		gui->element = NULL;
		gui->draw = 1;
		gui->step = 0;
		gui->draw_phanton = 0;
		//if (gui->phanton){
		//	gui->phanton = NULL;
		//}
	next_step:
		
		gui->lock_ax_x = 0;
		gui->lock_ax_y = 0;
		gui->user_flag_x = 0;
		gui->user_flag_y = 0;

		gui->draw = 1;
	end_step:
		return 1;
}

int gui_text_info (gui_obj *gui){
	if (gui->modal == TEXT) {
		//static int t_sty_idx = 0;
		int num_tstyles = gui->drawing->num_tstyles;
		dxf_tstyle *t_sty = gui->drawing->text_styles;
		
		if (gui->t_sty_idx >= num_tstyles) gui->t_sty_idx = 0;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place an text", NK_TEXT_LEFT);
		
		nk_label(gui->ctx, "Style:", NK_TEXT_LEFT);
		if (nk_combo_begin_label(gui->ctx,  t_sty[gui->t_sty_idx].name, nk_vec2(220,200))){
			
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
		
		nk_label(gui->ctx, "Text:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->txt, DXF_MAX_CHARS, nk_filter_default);
		gui->txt_h = nk_propertyd(gui->ctx, "Text Height", 0.0d, gui->txt_h, 100.0d, 0.1d, 0.1d);
		gui->t_al_v = nk_combo(gui->ctx, text_al_v, T_AL_V_LEN, gui->t_al_v, 20, nk_vec2(100,105));
		gui->t_al_h = nk_combo(gui->ctx, text_al_h, T_AL_H_LEN, gui->t_al_h, 20, nk_vec2(100,105));
	}
	return 1;
}