#include "gui_use.h"

int gui_expl_interactive(gui_obj *gui){
	if (gui->modal != EXPLODE) return 0;
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step == 1 && (!gui->sel_list->next)){
		/* if selection list is empty, back to first step */
		gui->step = 0;
		gui->free_sel = 1;
	}
	
	if (gui->step == 0){
		/* in first step, select the elements to proccess*/
		gui->en_distance = 0;
		gui->sel_ent_filter = DXF_INSERT;
		gui_simple_select(gui);
	}
	else{
		//gui->free_sel = 0;
		if (gui->ev & EV_ENTER){ /* confirm insert explosion */
			
			dxf_node *ins_ent = NULL, *current = NULL;
			ins_ent = gui->sel_list->next->data;
			dxf_node *block = NULL, *blk_name = NULL;
			
			double x = 0.0, y = 0.0, z = 0.0, angle = 0.0;
			double scale_x = 1.0, scale_y = 1.0, scale_z = 1.0;
			
			/* get insert parameters */
			current = ins_ent->obj.content;
			while (current){
				if (current->type == DXF_ATTR){ /* DXF attibute */
					switch (current->value.group){
						case 10:
							x = current->value.d_data;
							break;
						case 20:
							y = current->value.d_data;
							break;
						case 30:
							z = current->value.d_data;
							break;
						case 41:
							scale_x = current->value.d_data;
							break;
						case 42:
							scale_y = current->value.d_data;
							break;
						case 43:
							scale_z = current->value.d_data;
							break;
						case 50:
							angle = current->value.d_data;
							break;
						case 2:
							blk_name = current;
							break;
					}
				}
				current = current->next; /* go to the next in the list */
			}
			
			/* find relative block */
			if(blk_name) {
				block = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", blk_name->value.s_data);
				if(block) {
					do_add_entry(&gui->list_do, "EXPLODE");
					current = block->obj.content;
					while (current){ /* sweep elements in block */
						if (current->type == DXF_ENT){
							if (strcmp(current->obj.name, "ATTDEF") != 0){ /* skip ATTDEF elements */
								dxf_node *new_ent = dxf_ent_copy(current, DWG_LIFE);
								/* apply modifications */
								dxf_edit_scale(new_ent, scale_x, scale_y, scale_z);
								dxf_edit_rot(new_ent, angle);
								dxf_edit_move(new_ent, x, y, z);
								
								/* append to drawing */
								drawing_ent_append(gui->drawing, new_ent);
								new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
								do_add_item(gui->list_do.current, NULL, new_ent);
								
							}
						}
						current = current->next; /* go to the next in the list*/
					}
				}
				
				/* remove selected insert */
				dxf_obj_subst(ins_ent, NULL);
				do_add_item(gui->list_do.current, ins_ent, NULL);
				
			}
			gui->step = 0;
			sel_list_clear (gui);
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){ /* cancel attrib creation */
			gui_default_modal(gui);
			
		}
	}
	
	return 1;
}

int gui_expl_info (gui_obj *gui){
	if (gui->modal != EXPLODE) return 0;
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Explode insert", NK_TEXT_LEFT);
	
	if (gui->step == 0){ /* get insert element to edit */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Select a Insert element", NK_TEXT_LEFT);
	} else {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Confirm", NK_TEXT_LEFT);
	}
	return 1;
}