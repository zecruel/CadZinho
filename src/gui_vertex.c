#include "gui_use.h"

int gui_vertex_interactive(gui_obj *gui){
	if (gui->modal != VERTEX) return 0;
	
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step >= 1 && (!gui->sel_list->next || (gui->ev & EV_ADD))){
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
	else if (gui->step >= 1){
		gui->element = NULL;
		gui->draw_vert = 1;
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			sel_list_clear (gui);
			gui_first_step(gui);
			gui->step = 0;
		}
	}
	return 1;
}

int gui_vertex_info (gui_obj *gui){
	/* view and modify entities vertex */
	if (gui->modal != VERTEX) return 0;
	static dxf_node *ent = NULL;
	
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

		gui->step = 2;
	}
	else { /* all inited */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 110});
		 /* show entity type */
		nk_label(gui->ctx, "Type:", NK_TEXT_RIGHT);
		nk_label_colored(gui->ctx, ent->obj.name, NK_TEXT_LEFT, nk_rgb(255,255,0));
		
		
		
		
	}

	return 1;
}