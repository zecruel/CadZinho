#include "gui_use.h"

int gui_scale_interactive(gui_obj *gui){
	if (gui->modal == SCALE){
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* phantom object */
				gui->phanton = dxf_list_parse(gui->drawing, gui->sel_list, 0, 0);
				graph_list_modify(gui->phanton, gui->step_x[gui->step]*(1 - gui->scale), gui->step_y[gui->step]*(1 - gui->scale), gui->scale, gui->scale, 0.0);
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
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				if (gui->sel_list != NULL){
					list_node *current = gui->sel_list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, "SCALE");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
								
								dxf_edit_scale(new_ent, gui->scale, gui->scale, gui->scale);
								dxf_edit_move(new_ent, gui->step_x[gui->step - 1]*(1 - gui->scale), gui->step_y[gui->step - 1]*(1 - gui->scale), 0.0);
								dxf_edit_move(new_ent, gui->step_x[gui->step] - gui->step_x[gui->step - 1], gui->step_y[gui->step] - gui->step_y[gui->step - 1], 0.0);
								
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
								//drawing_ent_append(gui->drawing, new_ent);
								
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
								
								current->data = new_ent;
							}
						}
						current = current->next;
					}
					current = gui->sel_list->next;
				}
				goto first_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto first_step;
			}
			if (gui->ev & EV_MOTION){
				graph_list_modify(gui->phanton, gui->step_x[gui->step] - gui->step_x[gui->step + 1], gui->step_y[gui->step] - gui->step_y[gui->step + 1], 1.0, 1.0, 0.0);
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

int gui_scale_info (gui_obj *gui){
	if (gui->modal == SCALE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Scale a selection", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter base point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter destination point", NK_TEXT_LEFT);
		}
		gui->scale = nk_propertyd(gui->ctx, "Scale", 0.0d, gui->scale, 100.0d, 0.1d, 0.1d);
	}
	return 1;
}