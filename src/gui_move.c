#include "gui_use.h"

int gui_move_interactive(gui_obj *gui){
	if (gui->modal == MOVE){
		//static dxf_node *new_el;
		
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
			gui->sel_ent_filter = ~DXF_NONE;
			gui_simple_select(gui);
		}
		else if (gui->step == 1){
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* phantom object */
				gui->phanton = dxf_list_parse(gui->drawing, gui->sel_list, 0, 0);
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
					list_node *current = gui->sel_list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, "MOVE");
					}
					while (current != NULL){
						if (current->data){
							if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
								new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
								dxf_edit_move(new_ent, gui->step_x[gui->step] - gui->step_x[gui->step - 1], gui->step_y[gui->step] - gui->step_y[gui->step - 1], 0.0);
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
								//drawing_ent_append(gui->drawing, new_ent);
								
								dxf_obj_subst((dxf_node *)current->data, new_ent);
								
								do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);
								
								dxf_node *blk, *blk_rec, *blk_old, *blk_rec_old;
								if ( dxf_dim_rewrite (gui->drawing, new_ent, &blk, &blk_rec, &blk_old, &blk_rec_old)){
									do_add_item(gui->list_do.current, blk_old, NULL);
									do_add_item(gui->list_do.current, blk_rec_old, NULL);
									do_add_item(gui->list_do.current, NULL, blk);
									do_add_item(gui->list_do.current, NULL, blk_rec);
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
				graph_list_modify(gui->phanton, gui->step_x[gui->step] - gui->step_x[gui->step + 1], gui->step_y[gui->step] - gui->step_y[gui->step + 1], 1.0, 1.0, 0.0);
				gui->step_x[gui->step + 1] = gui->step_x[gui->step];
				gui->step_y[gui->step + 1] = gui->step_y[gui->step];
			}
		}
	}
	return 1;
}

int gui_move_info (gui_obj *gui){
	if (gui->modal == MOVE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Move a selection", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, "Enter base point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter destination point", NK_TEXT_LEFT);
		}
	}
	return 1;
}