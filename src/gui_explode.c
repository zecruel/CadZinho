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
			
			dxf_node *ins_ent = NULL;
			ins_ent = gui->sel_list->next->data;
			
			/* explode insert entity in a list of entities */
			list_node * list = dxf_edit_expl_ins(gui->drawing, ins_ent, gui->expl_mode);
			
			
			/* sweep the  list */
			list_node *current = NULL;
			if (list) current = list->next;
			dxf_node *new_ent = NULL;
			if (current != NULL){ /* verify if is a valid list */
				/* create a item in do/redo list */
				do_add_entry(&gui->list_do, "EXPLODE");
				/* remove selected insert */
				do_add_item(gui->list_do.current, ins_ent, NULL);
				dxf_obj_subst(ins_ent, NULL);
				
			}
			while (current != NULL){
				if (current->data){
					if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
						/*copy entity to permanent memory */
						new_ent = dxf_ent_copy((dxf_node *)current->data, DWG_LIFE);
						/* append to drawing */
						drawing_ent_append(gui->drawing, new_ent);
						new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
						do_add_item(gui->list_do.current, NULL, new_ent);
					}
				}
				current = current->next;
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
	
		/* attributes to text options*/
		nk_label(gui->ctx, "Attributes to text:", NK_TEXT_LEFT);
		int tag = gui->expl_mode & EXPL_TAG;
		int value = gui->expl_mode & EXPL_VALUE;
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_checkbox_label(gui->ctx, "Value", &value);
		nk_checkbox_label(gui->ctx, "Tag", &tag);
		
		if(tag) gui->expl_mode |= EXPL_TAG;
		else gui->expl_mode &= ~EXPL_TAG;
		if(value) gui->expl_mode |= EXPL_VALUE;
		else gui->expl_mode &= ~EXPL_VALUE;
	}
	
	return 1;
}