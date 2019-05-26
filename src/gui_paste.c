#include "gui_use.h"

int gui_paste_interactive(gui_obj *gui){
	if (gui->modal == PASTE){
		//static dxf_node *new_el;
		
		if (gui->step == 0){
			if (!gui->clip_drwg) goto default_modal;
			gui->draw_tmp = 1;
			/* phantom object */
			//dxf_ents_parse(gui->clip_drwg);
			gui->phanton = dxf_graph_parse(gui->clip_drwg, gui->clip_drwg->ents, 0, ONE_TIME);
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
				goto default_modal;
				#if(0)
				if (gui->sel_list != NULL){
					/* sweep the selection list */
					list_node *current = gui->sel_list->next;
					dxf_node *new_ent = NULL;
					if (current != NULL){
						do_add_entry(&gui->list_do, "PASTE");
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
								
								current->data = new_ent;
							}
						}
						current = current->next;
					}
					//list_clear(gui->sel_list);
				}
				goto first_step;
				#endif
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
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

int gui_paste_info (gui_obj *gui){
	if (gui->modal == PASTE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Paste a selection", NK_TEXT_LEFT);
		nk_label(gui->ctx, "Enter destination point", NK_TEXT_LEFT);
	}
	return 1;
}