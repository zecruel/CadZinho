#include "gui_use.h"

int gui_vertex_interactive(gui_obj *gui){
	if (gui->modal != VERTEX) return 0;
	
	static dxf_node * vert_x, * vert_y, * vert_z;
	
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step >= 1 && (!gui->sel_list->next)){
		/* if selection list is empty, back to first step */
		gui->step = 0;
		gui->free_sel = 1;
	}
	
	if (gui->step == 0){
		/* in first step, select the elements to proccess*/
		gui->en_distance = 0;
		gui->draw_vert = 0;
		gui->sel_ent_filter = ~DXF_NONE;
		gui_simple_select(gui);
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
			gui->step = 0;
		}
	}
	else if (gui->step == 2){
		
		gui->draw_vert = 1;
		
		if (gui->ev & EV_ENTER){ /* new point entered */
			//int dxf_get_near_vert(dxf_node *obj, double pt_x, double pt_y, double clearance)
			
			int vert_idx = dxf_get_near_vert(gui->element, gui->step_x[gui->step], gui->step_y[gui->step], 10.0/gui->zoom);
			if(vert_idx > -1 && vert_idx == gui->vert_idx ){
				gui_next_step(gui);
				gui->step = 3;
				
				gui->step_x[5] = gui->step_x[gui->step];
				gui->step_y[5] = gui->step_y[gui->step];
				gui->step_x[6] = gui->step_x[gui->step];
				gui->step_y[6] = gui->step_y[gui->step];
			}
			else gui->vert_idx = vert_idx;
			gui->draw = 1;
		}
		
		
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			sel_list_clear (gui);
			gui_first_step(gui);
			gui->step = 0;
		}
	}
	
	else if (gui->step >= 3){
		
		gui->draw_vert = 1;
		
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->vert_idx = -1;
			gui_next_step(gui);
			gui->step = 2;
			gui->draw = 1;
		}
	}
	if (gui->step == 4){
		if (dxf_get_vert_idx(gui->element, gui->vert_idx, &vert_x, &vert_y, &vert_z))
			gui->step = 5;
		else gui->step = 2;
	}
	else if (gui->step == 5){
		if (gui->ev & EV_MOTION){
			vert_x->value.d_data = gui->step_x[gui->step];
			vert_y->value.d_data = gui->step_y[gui->step];
			gui->draw_phanton = 1;
			gui->phanton = dxf_graph_parse(gui->drawing, gui->element, 0 , FRAME_LIFE);
		}
		if (gui->ev & EV_ENTER){
			vert_x->value.d_data = gui->step_x[gui->step];
			vert_y->value.d_data = gui->step_y[gui->step];
			gui->step = 6;
		}
	}
	
	return 1;
}

int gui_vertex_info (gui_obj *gui){
	/* view and modify entities vertex */
	if (gui->modal != VERTEX) return 0;
	static dxf_node *ent = NULL, *new_ent = NULL;
	
	dxf_node *tmp;
	
	char tmp_str[DXF_MAX_CHARS+1];
	int i, j;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Edit Vertex", NK_TEXT_LEFT);
	if (gui->step == 0){ /* get elements to edit */
		nk_label(gui->ctx, "Select a element", NK_TEXT_LEFT);
	}
	else if (gui->step == 1){ /* init the interface */
		/* get information of first element in selection list */
		ent = gui->sel_list->next->data;
		gui->element = ent;

		gui->step = 2;
	}
	else if (gui->step == 2){
		gui->element = ent;
	}
	if (gui->step >= 2) { /* all inited */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		 /* show entity type */
		nk_label(gui->ctx, "Type:", NK_TEXT_RIGHT);
		nk_label_colored(gui->ctx, ent->obj.name, NK_TEXT_LEFT, nk_rgb(255,255,0));
		
		if (gui->step == 3) {
			new_ent = dxf_ent_copy(ent, 0);
			if (new_ent){
				gui->element = new_ent;
				gui->step = 4;
			} else gui->step = 2;
			//int dxf_get_vert_idx(dxf_node *obj, int idx, dxf_node ** vert_x, dxf_node ** vert_y, dxf_node ** vert_z){
		}
		if (gui->step == 6) {
			new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
			dxf_obj_subst(ent, new_ent);
			
			/* update undo/redo list */
			do_add_entry(&gui->list_do, "EDIT VERTEX");
			do_add_item(gui->list_do.current, ent, new_ent);
			
			ent = new_ent;
			gui->sel_list->next->data = new_ent;
			
			gui->step = 2;
			gui->draw = 1;
		}
		
	}
	
	

	return 1;
}