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
	static int fix_angle = 0;
	static double angle_fixed = 0.0;
	static double scale = 1.0;
	static double an_scale = 1.0;
	static double dist_fixed = 3.0;
	static int fix_dist = 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Place a dim", NK_TEXT_LEFT);
	
	if (gui->step == 0) nk_label(gui->ctx, "Enter start point", NK_TEXT_LEFT);
	else if (gui->step == 1) nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
	else {
		if (fix_dist) nk_label(gui->ctx, "Confirm", NK_TEXT_LEFT);
		else nk_label(gui->ctx, "Enter distance", NK_TEXT_LEFT);
	}
	
	scale = nk_propertyd(gui->ctx, "Scale", 1e-9, scale, 1.0e9, SMART_STEP(scale), SMART_STEP(scale));
	an_scale = nk_propertyd(gui->ctx, "an_scale", 1e-9, an_scale, 1.0e9, SMART_STEP(an_scale), SMART_STEP(an_scale));
	
	nk_checkbox_label(gui->ctx, "Fixed angle", &fix_angle);
	if (fix_angle)
		angle_fixed = nk_propertyd(gui->ctx, "Angle", -180.0, angle_fixed, 180.0, 1.0, 1.0);
	
	nk_checkbox_label(gui->ctx, "Fixed distance", &fix_dist);
	if (fix_dist)
		dist_fixed = nk_propertyd(gui->ctx, "dist_fixed", -1e9, dist_fixed, 1.0e9, SMART_STEP(dist_fixed), SMART_STEP(dist_fixed));
	
	if (gui->step == 0){
		/* frst point */
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			/* to next point*/
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	} 
	else {
		char tmp_str[DXF_MAX_CHARS + 1];
		
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim (gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, FRAME_LIFE); /* paper space */
		
		/* dimension angle */
		double angle;
		if (fix_angle) angle = angle_fixed * M_PI / 180.0; /* user entered angle or */
		else angle = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0])); /* calculed from points */
		double cosine = cos(angle);
		double sine = sin(angle);
		
		/* calcule dimension linear length (measure). It is a polarized distance in base line direction */
		double length = cosine*(gui->step_x[1]  - gui->step_x[0]) + sine*(gui->step_y[1] - gui->step_y[0]);
		
		/* update dimension entity parameters */
		dxf_attr_change(new_dim, 13, &gui->step_x[0]);
		dxf_attr_change(new_dim, 23, &gui->step_y[0]);
		dxf_attr_change(new_dim, 14, &gui->step_x[1]);
		dxf_attr_change(new_dim, 24, &gui->step_y[1]);
		dxf_attr_change(new_dim, 42, &length);
		angle *= 180.0/M_PI;
		dxf_attr_change(new_dim, 50, &angle);
		
		/* distance of dimension from measure points */
		double dist = 3.0 * scale;
		double dir = 1.0;
		if(fix_dist) dist = dist_fixed; /* user entered distance */
		else if (gui->step == 2){ /* or calcule from points */
			/* It is a polarized distance in perpenticular direction from dimension base line*/
			dist = -sine*(gui->step_x[2]  - gui->step_x[1])+ cosine*(gui->step_y[2] - gui->step_y[1]);
		}
		dir = (dist  > 0.0) ? 1.0 : -1.0;
		
		/* obtain dimension base point from distance */
		double base_x = gui->step_x[1] - sine * dist;
		double base_y = gui->step_y[1] + cosine * dist;
		dxf_attr_change(new_dim, 10, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 20, &base_y);
		
		/* calcule dimension annotation (text) placement point (outer from perpenticular direction)*/
		base_x += -length/2.0 * cosine - sine * 1.0 * dir * scale;
		base_y += -length/2.0 * sine + cosine * 1.0 * dir * scale;
		dxf_attr_change(new_dim, 11, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 21, &base_y);
		
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_linear_make(gui->drawing, new_dim, scale, an_scale, 2, 0, 0, 0.0, 0.0);
		
		/* draw phantom */
		gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
		gui->element = NULL;
		gui->draw_phanton = 1;
		
		if (gui->step == 1 && gui->ev & EV_ENTER){
			/* second point - go to confirm */
			gui->step = 2;
			gui_next_step(gui);
		}
		else if (gui->step == 2 && gui->ev & EV_ENTER){
			/* confirm - create a new DXF dim */
			
			/* first, create the "picture" block */ 
			int last_dim = dxf_find_last_dim (gui->drawing); /* get last dim number available*/
			snprintf(tmp_str, DXF_MAX_CHARS, "*D%d", last_dim); /* block name - "*D" + sequential number*/
			dxf_node *blkrec = NULL, *blk = NULL;
			if (dxf_new_block (gui->drawing, tmp_str, "",
				(double []){0.0, 0.0, 0.0},
				0, "", "", "", "",
				"0", list, &blkrec, &blk, DWG_LIFE))
			{	
				dxf_attr_change(blk, 70, (void*)(int[]){1}); /* set block to annonimous */
				/* atach block to dimension ent */
				dxf_attr_change(new_dim, 2, (void*)tmp_str);
				/* copy temporary dimension to drawing buffer */
				dxf_node *new_ent = dxf_ent_copy(new_dim, DWG_LIFE);
				drawing_ent_append(gui->drawing, new_ent);
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , DWG_LIFE);
				
				/* undo/redo list*/
				do_add_entry(&gui->list_do, "Linear DIMENSION");
				do_add_item(gui->list_do.current, NULL, blkrec);
				do_add_item(gui->list_do.current, NULL, blk);
				do_add_item(gui->list_do.current, NULL, new_ent);
			}
			gui->draw_phanton = 0;
			gui_first_step(gui);
			
		}
		else if (gui->ev & EV_CANCEL){ /* back to first point, if user cancel operation */
			gui_first_step(gui);
		}
	}
	
	return 1;
}