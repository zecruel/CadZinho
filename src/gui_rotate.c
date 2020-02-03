#include "gui_use.h"

int gui_rotate_interactive(gui_obj *gui){
	if (gui->modal == ROTATE){
		if (gui->step == 0) {
			/* try to go to next step */
			gui->step = 1;
			gui->free_sel = 0;
		}
		/* verify if elements in selection list */
		if (gui->step == 1 && (!gui->sel_list->next || (gui->ev & EV_ADD))){
			/* if selection list is empty, back to first step */
			gui->step = 0;
			gui->free_sel = 1;
		}
		
		if (gui->step == 0){
			/* in first step, select the elements to proccess*/
			gui->en_distance = 0;
			gui_simple_select(gui);
		}
		else if (gui->step == 1){
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* phantom object */
				gui->phanton = dxf_list_parse(gui->drawing, gui->sel_list, 0, 0);
				graph_list_modify(gui->phanton, gui->step_x[gui->step], gui->step_y[gui->step], 1.0, 1.0, gui->angle);
				gui->element = NULL;
				gui->draw_phanton = 1;
				gui->en_distance = 1;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				gui->step = 2;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				if (gui->sel_list != NULL){
					list_node *current = gui->sel_list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, "ROTATE");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
								dxf_edit_move(new_ent, -gui->step_x[gui->step - 1], -gui->step_y[gui->step - 1], 0.0);
								dxf_edit_rot(new_ent, gui->angle);
								dxf_edit_move(new_ent, gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0.0);
								
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
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				graph_list_modify(gui->phanton, 0.0, 0.0, 1.0, 1.0, gui->angle);
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			}
		}
	}
	return 1;
}

int gui_rotate_info (gui_obj *gui){
	if (gui->modal == ROTATE) {
		static const char *mode[] = {"Active angle","3 points"};
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Rotate a selection", NK_TEXT_LEFT);
		int h = 2 * 25 + 5;
		gui->rot_mode = nk_combo(gui->ctx, mode, 2, gui->rot_mode, 20, nk_vec2(150, h));
		
		if (gui->step == 0){
			nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, "Enter pivot point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Confirm rotation", NK_TEXT_LEFT);
		}
		gui->angle = nk_propertyd(gui->ctx, "Angle", -180.0d, gui->angle, 180.0d, 0.1d, 0.1d);
	}
	return 1;
}