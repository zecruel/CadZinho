#include "gui_use.h"
/* view and modify entities vertex */

int gui_vertex_interactive(gui_obj *gui){
	if (gui->modal != VERTEX) return 0;
	
  gui->phanton = NULL;
  
	static dxf_node * vert_x, * vert_y, * vert_z, * bulge;
	
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
		/* select a vertex */
		gui->draw_vert = 1; /* show vertexes of selected entity */
		if (gui->ev & EV_ENTER){ /* new point entered */
			/* verify if any vertex is selected */
			int vert_idx = dxf_get_near_vert(gui->element, gui->step_x[gui->step], gui->step_y[gui->step], 10.0/gui->zoom);
			if(vert_idx > -1 && vert_idx == gui->vert_idx ){
				/* second click over same vertex */
				gui_next_step(gui); /* go to next step (modify vertex) */
				gui->step = 3;
				
				gui->step_x[5] = gui->step_x[gui->step];
				gui->step_y[5] = gui->step_y[gui->step];
				gui->step_x[6] = gui->step_x[gui->step];
				gui->step_y[6] = gui->step_y[gui->step];
			}
			else gui->vert_idx = vert_idx; /* hilite selected vertex */
			gui->draw = 1;
		}
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			if (gui->vert_idx > -1){
				gui->vert_idx = -1;
			}
			else{
				sel_list_clear (gui);
				gui_first_step(gui);
				gui->step = 0;
			}
		}
	}
	
	else if (gui->step >= 3){
		/* copy selected entity on a temporary object - not here */
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
		/* get pointers to coordinates objects directly in entity */
		if (dxf_get_vert_idx(gui->element, gui->vert_idx, &vert_x, &vert_y, &vert_z, &bulge))
			gui->step = 5;
		else gui->step = 2;
	}
	else if (gui->step == 5){
		/* modify vertex position */
		if (gui->ev & EV_MOTION){
			vert_x->value.d_data = gui->step_x[gui->step];
			vert_y->value.d_data = gui->step_y[gui->step];
		}
		if (gui->ev & EV_ENTER){
			vert_x->value.d_data = gui->step_x[gui->step];
			vert_y->value.d_data = gui->step_y[gui->step];
			gui->step = 6;
		}
    gui->draw_phanton = 1;
    gui->phanton = dxf_graph_parse(gui->drawing, gui->element, 0 , FRAME_LIFE);
	}
	
	return 1;
}

int gui_vertex_info (gui_obj *gui){
	/* view and modify entities vertex */
	if (gui->modal != VERTEX) return 0;
	static dxf_node *ent = NULL, *new_ent = NULL;
  
  gui->phanton = NULL;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Edit Vertex"), NK_TEXT_LEFT);
	if (gui->step == 0){ /* get elements to edit */
		nk_label(gui->ctx, _l("Select a element"), NK_TEXT_LEFT);
	}
	else if (gui->step == 1){ /* init the interface */
		/* get information of first element in selection list */
		ent = gui->sel_list->next->data;
		gui->element = ent;
		/* clear select list and mantain only first entity */
		sel_list_clear (gui);
		list_modify(gui->sel_list, gui->element, LIST_TOGGLE, SEL_LIFE);

		gui->step = 2;
	}
	else if (gui->step == 2){
		gui->element = ent; /* reinit entity */
		if (gui->vert_idx > -1){
			nk_label(gui->ctx, _l("Click again to modify"), NK_TEXT_LEFT);
		}
		else{
			nk_label(gui->ctx, _l("Select Vertex"), NK_TEXT_LEFT);
		}
	}
	else{
		nk_label(gui->ctx, _l("Confirm modification"), NK_TEXT_LEFT);
	}
	if (gui->step >= 2) { /* all inited */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){40, 120});
		/* show entity type */
		nk_label(gui->ctx, _l("Type:"), NK_TEXT_RIGHT);
		nk_label_colored(gui->ctx,
      strpool_cstr2( &obj_pool, ent->obj.id),
      NK_TEXT_LEFT, nk_rgb(255,255,0));
		
		if (gui->vert_idx > -1){
			/* show selected vertex information */
			dxf_node * vert_x, * vert_y, * vert_z, * bulge;
			if (dxf_get_vert_idx(gui->element, gui->vert_idx, &vert_x, &vert_y, &vert_z, &bulge)){
				char tmp_str[64];
				nk_layout_row(gui->ctx, NK_STATIC, 15, 2, (float[]){110, 60});
				snprintf(tmp_str, 63, "%d", gui->vert_idx);
				nk_label(gui->ctx, _l("Selected vertex:"), NK_TEXT_RIGHT);
				nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
				
				nk_layout_row(gui->ctx, NK_STATIC, 15, 2, (float[]){40, 120});
				if (vert_x){
					snprintf(tmp_str, 63, "%.9g", vert_x->value.d_data);
					nk_label(gui->ctx, _l("X:"), NK_TEXT_RIGHT);
					nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
				}
				if (vert_y){
					snprintf(tmp_str, 63, "%.9g", vert_y->value.d_data);
					nk_label(gui->ctx, _l("Y:"), NK_TEXT_RIGHT);
					nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
				}
				if (vert_z){
					snprintf(tmp_str, 63, "%.9g", vert_z->value.d_data);
					nk_label(gui->ctx, _l("Z:"), NK_TEXT_RIGHT);
					nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
				}
				if (bulge){
					if (gui->step < 5){
						nk_label(gui->ctx, _l("Bulge:"), NK_TEXT_RIGHT);
						snprintf(tmp_str, 63, "%.9g", bulge->value.d_data);
						nk_label_colored(gui->ctx, tmp_str, NK_TEXT_LEFT, nk_rgb(255,255,0));
						
					}
					else{
						/* modify bulge of current vertex */
						nk_layout_row_dynamic(gui->ctx, 20, 1);
						double b = nk_propertyd(gui->ctx, _l("#Bulge"), -10.0, bulge->value.d_data, 10.0, 0.1, 0.1f);
						
						if (fabs(b - bulge->value.d_data) > 1.0e-9){
							bulge->value.d_data = b;
							
							gui->draw_phanton = 1;
							gui->phanton = dxf_graph_parse(gui->drawing, gui->element, 0 , FRAME_LIFE);
							gui->draw = 1;
						}
					}
				}
			}
		}
		if (gui->step == 3) {
			/* copy selected entity on a temporary object */
			new_ent = dxf_ent_copy(ent, 0);
			if (new_ent){
				gui->element = new_ent;
				gui->step = 4;
			} else gui->step = 2; /* fail to copy entity */
		}
		if (gui->step == 6) { /* confirm vertex modification */
			new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
			dxf_obj_subst(ent, new_ent);
			
			/* update undo/redo list */
			do_add_entry(&gui->list_do, _l("EDIT VERTEX"));
			do_add_item(gui->list_do.current, ent, new_ent);
      
      /* for DIMENSIONS - remake the block "picture" */
      dxf_node *blk, *blk_rec, *blk_old, *blk_rec_old;
      if ( dxf_dim_rewrite (gui->drawing, new_ent, &blk, &blk_rec, &blk_old, &blk_rec_old)){
        do_add_item(gui->list_do.current, blk_old, NULL);
        do_add_item(gui->list_do.current, blk_rec_old, NULL);
        do_add_item(gui->list_do.current, NULL, blk);
        do_add_item(gui->list_do.current, NULL, blk_rec);
      }
			
			ent = new_ent;
			gui->sel_list->next->data = new_ent;
			gui->step = 2;
			gui->draw = 1;
		}
	}
	return 1;
}