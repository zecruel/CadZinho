#include "gui_use.h"

int gui_expl_interactive(gui_obj *gui){
	if (gui->modal != EXPLODE) return 0;
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
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
	else{
		//gui->free_sel = 0;
		
		if (gui->ev & EV_ENTER){ /* confirm elment explosion */
			
			dxf_node *ent = NULL;
			list_node *curr_sel = gui->sel_list->next;
			int init_do = 0;
			
			while(curr_sel){
				ent = (dxf_node *) curr_sel->data;
				list_node * list = NULL;
				if( (strcmp(ent->obj.name, "INSERT") == 0) && (gui->expl_mode & EXPL_INS) ){
					/* explode insert entity in a list of entities */
					list = dxf_edit_expl_ins(gui->drawing, ent, gui->expl_mode);
				}
				else if( ((strcmp(ent->obj.name, "LWPOLYLINE") == 0) ||
					(strcmp(ent->obj.name, "POLYLINE") == 0)) && 
					(gui->expl_mode & EXPL_POLY) ){
					/* explode polyline entity in a list of lines and arcs */
					list = dxf_edit_expl_poly(gui->drawing, ent, gui->expl_mode);
				}
				else if( (strcmp(ent->obj.name, "DIMENSION") == 0) && (gui->expl_mode & EXPL_DIM) ){
					/* explode dimension entity in a list of entities */
					list = dxf_edit_expl_dim(gui->drawing, ent, gui->expl_mode);
				}
				else if(gui->expl_mode & (EXPL_RAW_LINE | EXPL_RAW_PLINE)){
					list = dxf_edit_expl_raw(gui->drawing, ent, gui->expl_mode);
				}
				/* sweep the  list */
				list_node *current = NULL;
				if (list) current = list->next;
				dxf_node *new_ent = NULL;
				if (current != NULL){ /* verify if is a valid list */
					if(!init_do){
						/* create a entry in do/redo list */
						do_add_entry(&gui->list_do, "EXPLODE");
						init_do = 1;
					}
					/* remove current element (will substituted for its components) */
					dxf_obj_subst(ent, NULL);
					do_add_item(gui->list_do.current, ent, NULL);
					
					
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
				curr_sel = curr_sel->next;
			}
			gui->step = 0;
			sel_list_clear (gui);
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){ /* cancel explosion */
			gui_default_modal(gui);
			
		}
	}
	
	return 1;
}

int gui_expl_info (gui_obj *gui){
	if (gui->modal != EXPLODE) return 0;
	
	if (gui->step == 0){ /* get elements to edit */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Explode elements", NK_TEXT_LEFT);
		nk_label(gui->ctx, "Select/Add element", NK_TEXT_LEFT);
	} else {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Explode elements", NK_TEXT_LEFT);
		
		int ins = gui->expl_mode & EXPL_INS;
		int poly = gui->expl_mode & EXPL_POLY;
		int dim = gui->expl_mode & EXPL_DIM;
		int mtext = gui->expl_mode & EXPL_MTEXT;
		int txt_chr = gui->expl_mode &	EXPL_CHAR;
		int hatch = gui->expl_mode & EXPL_HATCH;
		int raw_line = gui->expl_mode & EXPL_RAW_LINE;
		int raw_pline = gui->expl_mode & EXPL_RAW_PLINE;
		int tag = gui->expl_mode & EXPL_TAG;
		int value = gui->expl_mode & EXPL_VALUE;
		
		int raw = raw_line || raw_pline;
		int prev_raw = raw;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_checkbox_label(gui->ctx, "Insert", &ins);
		if(ins) gui->expl_mode |= EXPL_INS;
		else gui->expl_mode &= ~EXPL_INS;
		if (ins){
			/* attributes to text options*/
			nk_layout_row_dynamic(gui->ctx, 45, 1);
			if (nk_group_begin(gui->ctx, "Attributes to text", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(gui->ctx, 15, 1);
				nk_label(gui->ctx, "Attributes to text:", NK_TEXT_LEFT);
				nk_layout_row_dynamic(gui->ctx, 15, 2);
				nk_checkbox_label(gui->ctx, "Value", &value);
				nk_checkbox_label(gui->ctx, "Tag", &tag);
				
				if(tag) gui->expl_mode |= EXPL_TAG;
				else gui->expl_mode &= ~EXPL_TAG;
				if(value) gui->expl_mode |= EXPL_VALUE;
				else gui->expl_mode &= ~EXPL_VALUE;
				nk_group_end(gui->ctx);
			}
		}
		
		nk_layout_row_dynamic(gui->ctx, 18, 2);
		nk_checkbox_label(gui->ctx, "Poly Line", &poly);
		if(poly) gui->expl_mode |= EXPL_POLY;
		else gui->expl_mode &= ~EXPL_POLY;
		
		nk_checkbox_label(gui->ctx, "Dimension", &dim);
		if(dim) gui->expl_mode |= EXPL_DIM;
		else gui->expl_mode &= ~EXPL_DIM;
		
		/*  // TODO
		nk_checkbox_label(gui->ctx, "M Text", &mtext);
		if(mtext) gui->expl_mode |= EXPL_MTEXT;
		else gui->expl_mode &= ~EXPL_MTEXT;
		
		nk_checkbox_label(gui->ctx, "Char", &txt_chr);
		if(txt_chr) gui->expl_mode |= EXPL_CHAR;
		else gui->expl_mode &= ~EXPL_CHAR;
		
		nk_checkbox_label(gui->ctx, "Hatch", &hatch);
		if(hatch) gui->expl_mode |= EXPL_HATCH;
		else gui->expl_mode &= ~EXPL_HATCH;
		*/
		
		nk_checkbox_label(gui->ctx, "Raw", &raw);
		if (raw){
			if (!prev_raw) gui->expl_mode |= EXPL_RAW_LINE;
			/* attributes to text options*/
			nk_layout_row_dynamic(gui->ctx, 30, 1);
			if (nk_group_begin(gui->ctx, "Raw options", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_option_label(gui->ctx, "Line", raw_line)) {
					gui->expl_mode |= EXPL_RAW_LINE;
					gui->expl_mode &= ~EXPL_RAW_PLINE;
					raw_pline = 0;
				}
				if (nk_option_label(gui->ctx, "Poly", raw_pline)){
					gui->expl_mode |= EXPL_RAW_PLINE;
					gui->expl_mode &= ~EXPL_RAW_LINE;
				}
				
				nk_group_end(gui->ctx);
			}
		}
		else gui->expl_mode &= ~(EXPL_RAW_LINE | EXPL_RAW_PLINE);
	}
	
	return 1;
}