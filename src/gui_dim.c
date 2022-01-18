#include "gui_use.h"
#include "dxf_dim.h"

int gui_dim_interactive(gui_obj *gui){
	
	if (gui->modal != DIMENSION) return 0;
	
	if (gui->step == 0){
		
	}
	else if (gui->step == 1){
		
	}
	else{
		
	}
	
	return 1;
}

int gui_dim_info (gui_obj *gui){
	if (gui->modal != DIMENSION) return 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Place a dim", NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, "Enter start point", NK_TEXT_LEFT);
		
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	} 
	else {
		static int count = 1;
		char tmp_str[DXF_MAX_CHARS + 1];
		
		
		if (gui->step == 1) nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
		
		else nk_label(gui->ctx, "Enter distance", NK_TEXT_LEFT);
		
		dxf_node *new_dim = dxf_new_dim (gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, FRAME_LIFE); /* paper space */
		
		dxf_attr_change(new_dim, 13, &gui->step_x[0]);
		dxf_attr_change(new_dim, 23, &gui->step_y[0]);
		dxf_attr_change(new_dim, 14, &gui->step_x[1]);
		dxf_attr_change(new_dim, 24, &gui->step_y[1]);
		double angle = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0]));
		double length = sqrt(pow(gui->step_y[1] - gui->step_y[0], 2) + pow(gui->step_x[1] - gui->step_x[0], 2));
		double cosine = cos(angle);
		double sine = sin(angle);

		double dist = 3.0;
		double dir = 1.0;
		
		if (gui->step == 2){
			dist = -sine*(gui->step_x[2]  - gui->step_x[1])+ cosine*(gui->step_y[2] - gui->step_y[1]);
			dir = (dist  > 0.0) ? 1.0 : -1.0;
		}
		
		double base_x = gui->step_x[1] - sine * dist;
		double base_y = gui->step_y[1] + cosine * dist;
		dxf_attr_change(new_dim, 10, &base_x);
		dxf_attr_change(new_dim, 20, &base_y);
		
		base_x += -length/2.0 * cosine - sine * 1.0 * dir;
		base_y += -length/2.0 * sine + cosine * 1.0 * dir;
		dxf_attr_change(new_dim, 11, &base_x);
		dxf_attr_change(new_dim, 21, &base_y);
		
		angle *= 180.0/M_PI;
		dxf_attr_change(new_dim, 50, &angle);
		
		list_node *list = dxf_dim_linear_make(gui->drawing, new_dim, length, 1.0, 1.0, 2, 0, 0, 0.0, 0.0);
		
		
		/* phantom object */
		gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
		gui->element = NULL;
		gui->draw_phanton = 1;
		
		if (gui->step == 1 && gui->ev & EV_ENTER){
			
			gui->step = 2;
			gui_next_step(gui);
		}
		else if (gui->step == 2 && gui->ev & EV_ENTER){
			/* create a new DXF dim */
			snprintf(tmp_str, DXF_MAX_CHARS, "*D%d", count);
			count ++;
			dxf_node *blkrec = NULL, *blk = NULL;
			if (dxf_new_block (gui->drawing, tmp_str, "",
				(double []){0.0, 0.0, 0.0},
				0, "", "", "", "",
				"0", list, &blkrec, &blk, DWG_LIFE))
			{	
				dxf_attr_change(blk, 70, (void*)(int[]){1}); /* set block to annonimous */
				
				dxf_attr_append(blkrec, 280, (void *) (int []){1}, DWG_LIFE); /* set dimension block to snapable */
				dxf_attr_append(blkrec, 281, (void *) (int []){0}, DWG_LIFE); /* set dimension block to non scalable */
				
				/* atach block to dimension ent */
				dxf_attr_change(new_dim, 2, (void*)tmp_str);
				dxf_node *new_ent = dxf_ent_copy(new_dim, DWG_LIFE);
				drawing_ent_append(gui->drawing, new_ent);
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
				
				do_add_entry(&gui->list_do, "NEW DIMENSION"); /* undo/redo list*/
				do_add_item(gui->list_do.current, NULL, blkrec); /* undo/redo list*/
				do_add_item(gui->list_do.current, NULL, blk); /* undo/redo list*/
				do_add_item(gui->list_do.current, NULL, new_ent);
			}
			gui->draw_phanton = 0;
			gui_first_step(gui);
			
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
	}
	
	return 1;
}