#include "gui_use.h"

int gui_scale_interactive(gui_obj *gui){
	if (gui->modal == SCALE){
		list_node *list = NULL;
		list_node *current = NULL;
		dxf_node *new_ent = NULL;
		double scale_x = gui->scale_x;
		double scale_y = gui->scale_y;
		
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
      /* user cancel operation */
      if (gui->ev & EV_CANCEL){
        gui->element = NULL;
        gui_default_modal(gui);
        gui->step = 0;
      }
		}
		else if (gui->step == 1){
			/* user enters a pivot point for scale */
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
        
        gui->phanton = NULL;
				
				if (gui->scale_mode != SCALE_3POINTS){
					scale_x = gui->scale_x; /* Entered factor - default mode */
					scale_y = gui->scale_y; /* Entered factor - default mode */
					
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
								dxf_edit_scale(new_ent, scale_x, scale_y, 1.0);
								dxf_edit_move(new_ent, gui->step_x[1]*(1 - scale_x), gui->step_y[1]*(1 - scale_y), 0.0);
								//dxf_edit_move(new_ent, gui->step_x[2] - gui->step_x[1], gui->step_y[2] - gui->step_y[1], 0.0);
								
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
		else if (gui->step == 2 && gui->scale_mode != SCALE_3POINTS){
			/* completes the operation in active scale factor mode */
			
			/* get and adjust scale */
			scale_x = gui->scale_x;
			scale_y = gui->scale_y;
			
			if (gui->ev & EV_ENTER){
				new_ent = NULL;
				current = gui->sel_list->next;
				if (current != NULL){
					/* add to undo/redo list */
					do_add_entry(&gui->list_do, _l("SCALE"));
				}
				/* sweep selection list */
				while (current != NULL){
					if (current->data){ /* current entity */
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
							/* apply modifications */
							dxf_edit_scale(new_ent, scale_x, scale_y, 1.0);
							dxf_edit_move(new_ent, gui->step_x[1]*(1 - scale_x), gui->step_y[1]*(1 - gui->scale_y), 0.0);
							dxf_edit_move(new_ent, gui->step_x[2] - gui->step_x[1], gui->step_y[2] - gui->step_y[1], 0.0);
							
							/* draw the new entity */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
							
							if(!gui->keep_orig){ /* add the new entity in drawing, replacing the old one  */
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
								
								/* for DIMENSIONS - remake the block "picture" */
								dxf_node *blk, *blk_rec, *blk_old, *blk_rec_old;
								if ( dxf_dim_rewrite (gui->drawing, new_ent, &blk, &blk_rec, &blk_old, &blk_rec_old)){
									do_add_item(gui->list_do.current, blk_old, NULL);
									do_add_item(gui->list_do.current, blk_rec_old, NULL);
									do_add_item(gui->list_do.current, NULL, blk);
									do_add_item(gui->list_do.current, NULL, blk_rec);
								}
							}
							else{ /* maintains the original entity*/
								drawing_ent_append(gui->drawing, new_ent);
								do_add_item(gui->list_do.current, NULL, new_ent);
								
								/* for DIMENSIONS - make the block "picture" */
								dxf_node *blk, *blk_rec;
								if ( dxf_dim_make_blk (gui->drawing, new_ent, &blk, &blk_rec)){
									do_add_item(gui->list_do.current, NULL, blk);
									do_add_item(gui->list_do.current, NULL, blk_rec);
								}
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
							dxf_edit_scale(new_ent, scale_x, scale_y, 1.0);
							dxf_edit_move(new_ent, gui->step_x[1]*(1 - scale_x), gui->step_y[1]*(1 - gui->scale_y), 0.0);
							dxf_edit_move(new_ent, gui->step_x[2] - gui->step_x[1], gui->step_y[2] - gui->step_y[1], 0.0);
							
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
		else if (gui->step == 2 && gui->scale_mode == SCALE_3POINTS){
      
      gui->phanton = NULL;
			/* in 3 points mode, user enter first point */
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
		else if (gui->step == 3 && gui->scale_mode == SCALE_3POINTS){
			/* completes the operation in 3 points mode */
			
			if (gui->proportional){
				scale_x = sqrt(pow(gui->step_y[3] - gui->step_y[1], 2)
					+ pow(gui->step_x[3] - gui->step_x[1], 2))
					/
					sqrt(pow(gui->step_y[2] - gui->step_y[1], 2) 
					+ pow(gui->step_x[2] - gui->step_x[1], 2));
				scale_y = scale_x;
			}
			else {
				scale_x = fabs((gui->step_x[3] - gui->step_x[1]) /
					(gui->step_x[2] - gui->step_x[1]));
				scale_y = fabs((gui->step_y[3] - gui->step_y[1]) /
					(gui->step_y[2] - gui->step_y[1]));
			}
			
			
			if (gui->ev & EV_ENTER){
				current = gui->sel_list->next;
				new_ent = NULL;
				if (current != NULL){
					/* add to undo/redo list */
					do_add_entry(&gui->list_do, _l("SCALE"));
				}
				/* sweep selection list */
				while (current != NULL){
					if (current->data){ /* current entity */
						if (((dxf_node *)current->data)->type == DXF_ENT){
							/* get a copy of current entity */
							new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
							/* apply modifications */
							dxf_edit_scale(new_ent, scale_x, scale_y, 1.0);
							dxf_edit_move(new_ent, gui->step_x[1]*(1 - scale_x), gui->step_y[1]*(1 - scale_y), 0.0);
							//dxf_edit_move(new_ent, gui->step_x[2] - gui->step_x[1], gui->step_y[2] - gui->step_y[1], 0.0);
							
							/* draw the new entity */
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
							
							if(!gui->keep_orig){ /* add the new entity in drawing, replacing the old one  */
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
								/* for DIMENSIONS - remake the block "picture" */
								dxf_node *blk, *blk_rec, *blk_old, *blk_rec_old;
								if ( dxf_dim_rewrite (gui->drawing, new_ent, &blk, &blk_rec, &blk_old, &blk_rec_old)){
									do_add_item(gui->list_do.current, blk_old, NULL);
									do_add_item(gui->list_do.current, blk_rec_old, NULL);
									do_add_item(gui->list_do.current, NULL, blk);
									do_add_item(gui->list_do.current, NULL, blk_rec);
								}
							}
							else{ /* maintains the original entity*/
								drawing_ent_append(gui->drawing, new_ent);
								do_add_item(gui->list_do.current, NULL, new_ent);
								/* for DIMENSIONS - make the block "picture" */
								dxf_node *blk, *blk_rec;
								if ( dxf_dim_make_blk (gui->drawing, new_ent, &blk, &blk_rec)){
									do_add_item(gui->list_do.current, NULL, blk);
									do_add_item(gui->list_do.current, NULL, blk_rec);
								}
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
							dxf_edit_scale(new_ent, scale_x, scale_y, 1.0);
							dxf_edit_move(new_ent, gui->step_x[1]*(1 - scale_x), gui->step_y[1]*(1 - scale_y), 0.0);
							//dxf_edit_move(new_ent, gui->step_x[2] - gui->step_x[1], gui->step_y[2] - gui->step_y[1], 0.0);
							
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

int gui_scale_info (gui_obj *gui){
	if (gui->modal == SCALE) {
    static char mode[2][DXF_MAX_CHARS + 1];
    strncpy(mode[0], _l("Active factor"), DXF_MAX_CHARS);
    strncpy(mode[1], _l("3 points"), DXF_MAX_CHARS);
    
    char *mode_addr[] = {mode[0], mode[1]};
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Scale a selection"), NK_TEXT_LEFT);
		
		int h = 2 * 25 + 5;
		gui->scale_mode = nk_combo(gui->ctx, (const char **)mode_addr, 2, gui->scale_mode, 20, nk_vec2(150, h));
		
		nk_checkbox_label(gui->ctx, _l("Keep Original"), &gui->keep_orig);
		nk_checkbox_label(gui->ctx, _l("Proportional"), &gui->proportional);
		
		if (gui->scale_mode != SCALE_3POINTS){
			
			if (gui->proportional){
				gui->scale_x = nk_propertyd(gui->ctx, _l("Factor"), 1e-9, gui->scale_x, 1.0e9, SMART_STEP(gui->scale_x), SMART_STEP(gui->scale_x));
				gui->scale_y = gui->scale_x;
			}
			else {
				gui->scale_x = nk_propertyd(gui->ctx, _l("X factor"), 1e-9, gui->scale_x, 1.0e9, SMART_STEP(gui->scale_x), SMART_STEP(gui->scale_x));
				gui->scale_y = nk_propertyd(gui->ctx, _l("Y factor"), 1e-9, gui->scale_y, 1.0e9, SMART_STEP(gui->scale_y), SMART_STEP(gui->scale_y));
			}
		}
		
		if (gui->step == 0){
			nk_label(gui->ctx, _l("Select/Add element"), NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, _l("Enter pivot point"), NK_TEXT_LEFT);
		} else if (gui->step == 2){
			if (gui->scale_mode == SCALE_3POINTS)
				nk_label(gui->ctx, _l("First point"), NK_TEXT_LEFT);
			else
				nk_label(gui->ctx, _l("Confirm scale"), NK_TEXT_LEFT);
		}
		else {
			/* in 3 points mode, show the efective scale factor */
			nk_label(gui->ctx, _l("End point"), NK_TEXT_LEFT);
			char ang_str[64];
			double scale_x = sqrt(pow(gui->step_y[3] - gui->step_y[1], 2) + pow(gui->step_x[3] - gui->step_x[1], 2) )/
				sqrt(pow(gui->step_y[2] - gui->step_y[1], 2) + pow(gui->step_x[2] - gui->step_x[1], 2));
			
			
			snprintf(ang_str, 63, _l("Factor=%.4g"), scale_x);
			nk_label(gui->ctx, ang_str, NK_TEXT_LEFT);
		}
	}
	return 1;
}