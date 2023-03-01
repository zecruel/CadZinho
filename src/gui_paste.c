#include "gui_use.h"

int gui_paste_interactive(gui_obj *gui){
	if (gui->modal == PASTE){
		//static dxf_node *new_el;
		static double min_x, min_y, min_z, max_x, max_y, max_z, center_x, center_y;
		if (gui->step == 0){
			if (!gui->clip_drwg) gui_default_modal(gui);
			gui->draw_tmp = 1;
			/* phantom object */
			//dxf_ents_parse(gui->clip_drwg);
			//dxf_ent_print2 (gui->clip_drwg->ents);
			//dxf_ents_parse2(dxf_drawing *drawing, int p_space, int pool_idx)
			gui->clip_drwg->font_list = gui->font_list;
			gui->clip_drwg->dflt_font = gui->dflt_font;
			//list_node * ents_list = dxf_ents_list(gui->clip_drwg, ONE_TIME);
			//gui->phanton = dxf_list_parse(gui->clip_drwg, ents_list, 0, 0);
			
			gui->phanton = dxf_ents_parse2(gui->clip_drwg, 0, FRAME_LIFE);
			if (!gui->phanton) gui_default_modal(gui);
			//dxf_ents_ext(dxf_drawing *drawing, double * min_x, double * min_y, double * max_x, double * max_y)
			
			dxf_ents_ext(gui->clip_drwg, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
			center_x = min_x + (max_x - min_x)/2.0;
			center_y = min_y + (max_y - min_y)/2.0;
			
			graph_list_modify(gui->phanton, -center_x, -center_y, 1.0, 1.0, 0.0);
			graph_list_modify(gui->phanton, 0.0, 0.0, 1.0, 1.0, gui->angle);
			graph_list_modify(gui->phanton, gui->step_x[gui->step] , gui->step_y[gui->step], 1.0, 1.0, 0.0);
			graph_list_modify(gui->phanton, (gui->step_x[gui->step])*(1 - gui->scale_x), (gui->step_y[gui->step]) *(1 - gui->scale_x), gui->scale_x, gui->scale_x, 0.0);
			
			gui->element = NULL;
			gui->draw_phanton = 1;
			gui->en_distance = 1;
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			gui->step = 1;
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			gui_next_step(gui);
		}
		else{
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				//list_node * dxf_drwg_ent_cpy_all(dxf_drawing *source, dxf_drawing *dest, int pool_idx){
				list_node * list = dxf_drwg_ent_cpy_all(gui->clip_drwg, gui->drawing, DWG_LIFE);
				if (!list) gui_default_modal(gui);
				dxf_cpy_lay_drwg(gui->clip_drwg, gui->drawing);
				dxf_cpy_sty_drwg(gui->clip_drwg, gui->drawing);
				dxf_cpy_ltyp_drwg(gui->clip_drwg, gui->drawing);
				dxf_cpy_appid_drwg(gui->clip_drwg, gui->drawing);
				gui_tstyle(gui);
				
				if (gui->sel_list != NULL){
					/* sweep the selection list */
					list_node *current = list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, _l("PASTE"));
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = (dxf_node *)current->data;
								
								dxf_edit_move(new_ent, -center_x, -center_y, 0.0);
								dxf_edit_rot(new_ent, gui->angle);
								dxf_edit_move(new_ent, gui->step_x[gui->step], gui->step_y[gui->step], 0.0);
								dxf_edit_scale(new_ent, gui->scale_x, gui->scale_x, gui->scale_x);
								dxf_edit_move(new_ent, gui->step_x[gui->step] * (1 - gui->scale_x), gui->step_y[gui->step] * (1 - gui->scale_x), 0.0);
								
								
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
								
								do_add_item(gui->list_do.current, NULL, new_ent);
								
							}
						}
						current = current->next;
					}
					//list_clear(gui->sel_list);
				}
				gui_default_modal(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
			if (gui->ev & EV_MOTION){
				gui->phanton = dxf_ents_parse2(gui->clip_drwg, 0, FRAME_LIFE);
				graph_list_modify(gui->phanton, -center_x, -center_y, 1.0, 1.0, 0.0);
				graph_list_modify(gui->phanton, 0.0, 0.0, 1.0, 1.0, gui->angle);
				graph_list_modify(gui->phanton, gui->step_x[gui->step] , gui->step_y[gui->step], 1.0, 1.0, 0.0);
				graph_list_modify(gui->phanton, (gui->step_x[gui->step])*(1 - gui->scale_x), (gui->step_y[gui->step]) *(1 - gui->scale_x), gui->scale_x, gui->scale_x, 0.0);
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			}
		}
	}
	return 1;
}

int gui_paste_info (gui_obj *gui){
	if (gui->modal == PASTE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Paste a selection"), NK_TEXT_LEFT);
		nk_label(gui->ctx, _l("Enter destination point"), NK_TEXT_LEFT);
		gui->scale_x = nk_propertyd(gui->ctx, _l("Scale"), 0.0, gui->scale_x, 100000.0, 0.1, 0.1f);
		gui->angle = nk_propertyd(gui->ctx, _l("Angle"), -180.0, gui->angle, 180.0, 0.1, 0.1f);
	}
	return 1;
}