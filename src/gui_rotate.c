#include "gui_use.h"

int gui_rotate_interactive(gui_obj *gui){
	if (gui->modal == ROTATE){
		list_node *list = NULL;
		list_node *current = NULL;
		dxf_node *new_ent = NULL;
		double angle = gui->angle;
		
		if (gui->step == 0) {
			/* try to go to next step */
			gui->step = 1;
			gui->free_sel = 0;
			gui->phanton = NULL;
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
			/* user enters a pivot point for rotation */
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				
				gui->element = NULL;
				gui->draw_phanton = 1;
				gui->en_distance = 1;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				gui->step = 2;
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
				
				if (gui->rot_mode != ROT_3POINTS){
					angle = gui->angle; /* Entered angle - default mode */
					
					/* make phantom list */
					list = list_new(NULL, FRAME_LIFE);
					list_clear(list);
					
					/* sweep selection list */
					current = gui->sel_list->next;
					while (current != NULL){
						if (current->data){ /* current entity */
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								/* get a temporary copy of current entity */
								new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
								list_node * new_el = list_new(new_ent, FRAME_LIFE);
								/* apply modifications */
								dxf_edit_move(new_ent, -gui->step_x[1], -gui->step_y[1], 0.0);
								dxf_edit_rot(new_ent, angle);
								dxf_edit_move(new_ent, gui->step_x[1], gui->step_y[1], 0.0);
								
								if (new_el){
									list_push(list, new_el);
								}
							}
						}
						current = current->next;
					}
					/* draw phantom */
					gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
				}
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
		else if (gui->step == 2 && gui->rot_mode != ROT_3POINTS){
			/* completes the operation in active angle mode */
			
			/* get and adjust angle */
			angle = gui->angle;
			angle = fmod(angle, 360.0);
			if (angle < 0) angle += 360.0;
			
			if (gui->ev & EV_ENTER){
				new_ent = NULL;
				current = gui->sel_list->next;
				if (current != NULL){
					/* add to undo/redo list */
					do_add_entry(&gui->list_do, "ROTATE");
				}
				/* sweep selection list */
				while (current != NULL){
					if (current->data){ /* current entity */
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
							/* apply modifications */
							dxf_edit_move(new_ent, -gui->step_x[1], -gui->step_y[1], 0.0);
							dxf_edit_rot(new_ent, angle);
							dxf_edit_move(new_ent, gui->step_x[1], gui->step_y[1], 0.0);
							
							/* draw the new entity */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
							
							if(!gui->keep_orig){ /* add the new entity in drawing, replacing the old one  */
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
							}
							else{ /* maintains the original entity*/
								drawing_ent_append(gui->drawing, new_ent);
								do_add_item(gui->list_do.current, NULL, new_ent);
							}
							
							current->data = new_ent; /* replancing in selection list */
						}
					}
					current = current->next;
				}
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				
				/* make phantom list */
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				
				/* sweep selection list */
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){ /* current entity*/
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							/* get a temporary copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							/* apply modifications */
							dxf_edit_move(new_ent, -gui->step_x[1], -gui->step_y[1], 0.0);
							dxf_edit_rot(new_ent, angle);
							dxf_edit_move(new_ent, gui->step_x[1], gui->step_y[1], 0.0);
							
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				/* draw phantom */
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
			}
		}
		else if (gui->step == 2 && gui->rot_mode == ROT_3POINTS){
			/* in 3 points mode, user enter first point to determine start angle */
			if (gui->ev & EV_ENTER){
				/* make phantom list */
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				
				/* sweep selection list */
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){ /* current entity*/
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							/* get a temporary copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				/* draw phantom */
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
				
				gui->step = 3;
				gui_next_step(gui);
				
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			
		}
		else if (gui->step == 3 && gui->rot_mode == ROT_3POINTS){
			/* completes the operation in 3 points mode */
			/* calcule angle diference between previous and current data point*/
			angle = atan2(gui->step_y[3] - gui->step_y[1], gui->step_x[3] - gui->step_x[1]) - 
				atan2(gui->step_y[2] - gui->step_y[1], gui->step_x[2] - gui->step_x[1]);
			/*adjust angle */
			angle = fmod(angle, 2.0 * M_PI);
			if (angle < 0) angle += 2.0 * M_PI;
			angle *= 180.0/M_PI;
			
			if (gui->ev & EV_ENTER){
				current = gui->sel_list->next;
				new_ent = NULL;
				if (current != NULL){
					/* add to undo/redo list */
					do_add_entry(&gui->list_do, "ROTATE");
				}
				/* sweep selection list */
				while (current != NULL){
					if (current->data){ /* current entity */
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
							/* apply modifications */
							dxf_edit_move(new_ent, -gui->step_x[1], -gui->step_y[1], 0.0);
							dxf_edit_rot(new_ent, angle);
							dxf_edit_move(new_ent, gui->step_x[1], gui->step_y[1], 0.0);
							
							/* draw the new entity */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
							
							if(!gui->keep_orig){ /* add the new entity in drawing, replacing the old one  */
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
							}
							else{ /* maintains the original entity*/
								drawing_ent_append(gui->drawing, new_ent);
								do_add_item(gui->list_do.current, NULL, new_ent);
							}
							
							current->data = new_ent; /* replancing in selection list */
						}
					}
					current = current->next;
				}
				gui_first_step(gui);
				gui->step = 1;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 1;
			}
			if (gui->ev & EV_MOTION){
				
				/* make phantom list */
				list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				
				/* sweep selection list */
				current = gui->sel_list->next;
				while (current != NULL){
					if (current->data){ /* current entity*/
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							/* get a temporary copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, FRAME_LIFE);
							list_node * new_el = list_new(new_ent, FRAME_LIFE);
							/* apply modifications */
							dxf_edit_move(new_ent, -gui->step_x[1], -gui->step_y[1], 0.0);
							dxf_edit_rot(new_ent, angle);
							dxf_edit_move(new_ent, gui->step_x[1], gui->step_y[1], 0.0);
							
							if (new_el){
								list_push(list, new_el);
							}
						}
					}
					current = current->next;
				}
				/* draw phantom */
				gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
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
		
		nk_checkbox_label(gui->ctx, "Keep Original", &gui->keep_orig);
		
		int h = 2 * 25 + 5;
		gui->rot_mode = nk_combo(gui->ctx, mode, 2, gui->rot_mode, 20, nk_vec2(150, h));
		
		if (gui->step == 0){
			nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, "Enter pivot point", NK_TEXT_LEFT);
		} else if (gui->step == 2){
			if (gui->rot_mode == ROT_3POINTS)
				nk_label(gui->ctx, "First point", NK_TEXT_LEFT);
			else
				nk_label(gui->ctx, "Confirm rotation", NK_TEXT_LEFT);
		}
		else {
			/* in 3 points mode, show the efective rotation angle */
			nk_label(gui->ctx, "End point", NK_TEXT_LEFT);
			char ang_str[64];
			double angle = atan2(gui->step_y[3] - gui->step_y[1], gui->step_x[3] - gui->step_x[1]) - 
				atan2(gui->step_y[2] - gui->step_y[1], gui->step_x[2] - gui->step_x[1]);
			
			angle = fmod(angle, 2.0 * M_PI);
			if (angle < 0) angle += 2.0 * M_PI;
			angle *= 180.0/M_PI;
			
			snprintf(ang_str, 63, "Angle=%.4g°", angle);
			nk_label(gui->ctx, ang_str, NK_TEXT_LEFT);
		}
		if (gui->rot_mode != ROT_3POINTS)
			gui->angle = nk_propertyd(gui->ctx, "Angle", -180.0, gui->angle, 180.0, 0.1, 0.1f);
	}
	return 1;
}