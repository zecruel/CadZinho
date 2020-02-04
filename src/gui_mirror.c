#include "gui_use.h"

int gui_mirror_interactive(gui_obj *gui){
	if (gui->modal == MIRROR){
		static list_node *list = NULL;
		list_node *current = NULL;
		dxf_node *new_ent = NULL;
		
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
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				
				
				gui->draw_tmp = 1;
				/* phantom object */
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
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
					/* sweep the selection list */
					current = gui->sel_list->next;
					
					if (current != NULL){
						do_add_entry(&gui->list_do, "MIRROR");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
								dxf_edit_mirror(new_ent, gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step - 1], gui->step_y[gui->step - 1]);
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
								
								if(!gui->keep_orig){
									dxf_obj_subst((dxf_node *)current->data, new_ent);
									do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
								}
								else{
									drawing_ent_append(gui->drawing, new_ent);
									do_add_item(gui->list_do.current, NULL, new_ent);
								}
								
								current->data = new_ent;
							}
						}
						current = current->next;
					}
					//list_clear(gui->sel_list);
				}
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				/*current = list->next;
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							dxf_edit_mirror(((dxf_node *)current->data), gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step - 1], gui->step_y[gui->step - 1]);
						}
					}
					current = current->next;
				}*/
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							dxf_edit_mirror(new_ent, gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step - 1], gui->step_y[gui->step - 1]);
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
			}
		}
	}
	return 1;
}

int gui_mirror_info (gui_obj *gui){
	if (gui->modal == MIRROR) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Mirror a selection", NK_TEXT_LEFT);
		nk_checkbox_label(gui->ctx, "Keep Original", &gui->keep_orig);
		
		if (gui->step == 0){
			nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, "Set the reflection line", NK_TEXT_LEFT);
			nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Set the reflection line", NK_TEXT_LEFT);
			nk_label(gui->ctx, "Enter second point", NK_TEXT_LEFT);
		}
	}
	return 1;
}