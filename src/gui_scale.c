#include "gui_use.h"

int gui_scale_interactive(gui_obj *gui){
	if (gui->modal == SCALE){
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
				graph_list_modify(gui->phanton, gui->step_x[gui->step]*(1 - gui->scale_x), gui->step_y[gui->step]*(1 - gui->scale_x), gui->scale_x, gui->scale_x, 0.0);
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
						do_add_entry(&gui->list_do, "SCALE");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
								
								dxf_edit_scale(new_ent, gui->scale_x, gui->scale_x, gui->scale_x);
								dxf_edit_move(new_ent, gui->step_x[gui->step - 1]*(1 - gui->scale_x), gui->step_y[gui->step - 1]*(1 - gui->scale_x), 0.0);
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
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				graph_list_modify(gui->phanton, gui->step_x[gui->step] - gui->step_x[gui->step + 1], gui->step_y[gui->step] - gui->step_y[gui->step + 1], 1.0, 1.0, 0.0);
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			}
		}
	}
	return 1;
}

int gui_scale_info (gui_obj *gui){
	if (gui->modal == SCALE) {
		/*static char scale_str[64];
		static int init = 0;
		nk_flags res;
		
		if (!init){
			snprintf(scale_str, 63, "%.9g", gui->scale_x);
			init = 1;
		}
		*/
		static const char *mode[] = {"Active factor","3 points"};
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Scale a selection", NK_TEXT_LEFT);
		
		int h = 2 * 25 + 5;
		gui->scale_mode = nk_combo(gui->ctx, mode, 2, gui->scale_mode, 20, nk_vec2(150, h));
		
		if (gui->step == 0){
			nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, "Enter base point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter destination point", NK_TEXT_LEFT);
		}
		gui->scale_x = nk_propertyd(gui->ctx, "Scale", 1.0e-9d, gui->scale_x, 1.0e9d, 0.1d, 0.1d);
		
		
		#if(0)
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, scale_str, 63, nk_filter_float);
		if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
			if (strlen(scale_str)){
				gui->scale_x = atof(scale_str);
				snprintf(scale_str, 63, "%.9g", gui->scale_x);
			}
		}
		#endif
	}
	return 1;
}