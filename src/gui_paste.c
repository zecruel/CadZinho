#include "gui_use.h"

int gui_paste_interactive(gui_obj *gui){
	if (gui->modal == PASTE){
		//static dxf_node *new_el;
		static double min_x, min_y, max_x, max_y, center_x, center_y;
		if (gui->step == 0){
			if (!gui->clip_drwg) goto default_modal;
			gui->draw_tmp = 1;
			/* phantom object */
			//dxf_ents_parse(gui->clip_drwg);
			//dxf_ent_print2 (gui->clip_drwg->ents);
			//dxf_ents_parse2(dxf_drawing *drawing, int p_space, int pool_idx)
			gui->clip_drwg->font_list = gui->font_list;
			gui->clip_drwg->dflt_font = gui->dflt_font;
			gui->phanton = dxf_ents_parse2(gui->clip_drwg, 0, ONE_TIME);
			if (!gui->phanton) goto default_modal;
			//dxf_ents_ext(dxf_drawing *drawing, double * min_x, double * min_y, double * max_x, double * max_y)
			
			dxf_ents_ext(gui->clip_drwg, &min_x, &min_y, &max_x, &max_y);
			center_x = min_x + (max_x - min_x)/2.0;
			center_y = min_y + (max_y - min_y)/2.0;
			
			graph_list_modify(gui->phanton, gui->step_x[gui->step] - center_x, gui->step_y[gui->step] - center_y, 1.0, 1.0, 0.0);
			
			gui->element = NULL;
			gui->draw_phanton = 1;
			gui->en_distance = 1;
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			gui->step = 1;
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			goto next_step;
		}
		else{
			if (gui->ev & EV_ENTER){
				//list_node * dxf_drwg_ent_cpy_all(dxf_drawing *source, dxf_drawing *dest, int pool_idx){
				list_node * list = dxf_drwg_ent_cpy_all(gui->clip_drwg, gui->drawing, FRAME_LIFE);
				if (!list) goto default_modal;
				dxf_cpy_lay_drwg(gui->clip_drwg, gui->drawing);
				dxf_cpy_sty_drwg(gui->clip_drwg, gui->drawing);
				dxf_cpy_ltyp_drwg(gui->clip_drwg, gui->drawing);
				gui_tstyle(gui);
				
				if (gui->sel_list != NULL){
					/* sweep the selection list */
					list_node *current = list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, "PASTE");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = (dxf_node *)current->data;
								double x = gui->step_x[gui->step] - center_x;
								double y = gui->step_y[gui->step] - center_y;
								dxf_edit_move(new_ent, x, y, 0.0);
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
								
								do_add_item(gui->list_do.current, NULL, new_ent);
								
							}
						}
						current = current->next;
					}
					//list_clear(gui->sel_list);
				}
				goto default_modal;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
			if (gui->ev & EV_MOTION){
				double x = gui->step_x[gui->step] - gui->step_x[gui->step + 1];
				double y = gui->step_y[gui->step] - gui->step_y[gui->step + 1];
				graph_list_modify(gui->phanton, x, y, 1.0, 1.0, 0.0);
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
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

int gui_paste_info (gui_obj *gui){
	if (gui->modal == PASTE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Paste a selection", NK_TEXT_LEFT);
		nk_label(gui->ctx, "Enter destination point", NK_TEXT_LEFT);
	}
	return 1;
}