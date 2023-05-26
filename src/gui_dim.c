#include "gui_use.h"
#include "dxf_dim.h"

int gui_dim_interactive(gui_obj *gui){
	
	if (gui->modal != DIM_LINEAR) return 0;
	
	if (gui->step == 0){
		
	}
	else if (gui->step == 1){
		
	}
	else{
		
	}
	
	return 1;
}

int gui_dim_linear_info (gui_obj *gui){
	if (gui->modal != DIM_LINEAR) return 0;
	static int fix_angle = 0;
	static double angle_fixed = 0.0;
	static double dist_fixed = 3.0;
	static int fix_dist = 0;
	static int custom_text = 0;
	static char user_text[DXF_MAX_CHARS+1] = "<>";
  
  gui->phanton = NULL;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a Linear Dim"), NK_TEXT_LEFT);
	
	static char sty_name[DXF_MAX_CHARS+1] = "STANDARD";
	
	nk_layout_row_template_begin(gui->ctx, 20);
	nk_layout_row_template_push_static(gui->ctx, 45);
	nk_layout_row_template_push_dynamic(gui->ctx);
	nk_layout_row_template_end(gui->ctx);
	nk_label(gui->ctx, _l("Style:"), NK_TEXT_LEFT);
	/* dimension style combo selection */
	int num_dsty = dxf_count_obj(gui->drawing->t_dimst, "DIMSTYLE");
	int h = num_dsty * 25 + 5;
	h = (h < 200)? h : 200;
	if (nk_combo_begin_label(gui->ctx,  sty_name, nk_vec2(220, h))){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		dxf_node *dsty, *dsty_nm, *next = NULL;
    
    char *name = NULL;
		/* sweep DIMSTYLE table */
		dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
		while (dsty){
			/* get name of current dimstyle */
			dsty_nm = dxf_find_attr2(dsty, 2);
      name = NULL;
      if (dsty_nm) name = (char *) strpool_cstr2( &name_pool, dsty_nm->value.str);
			if (name){
				/* dimstyle name */
        if (nk_button_label(gui->ctx, name)){
					/* select current dimstyle */
          strncpy(sty_name, name, DXF_MAX_CHARS);
					
					nk_combo_close(gui->ctx);
					break;
				}
			}
			if (next)
				dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
			else
				dsty = NULL;
		}
		
		nk_combo_end(gui->ctx);
	}
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	if (gui->step == 0) nk_label(gui->ctx, _l("Enter start point"), NK_TEXT_LEFT);
	else if (gui->step == 1) nk_label(gui->ctx, _l("Enter end point"), NK_TEXT_LEFT);
	else {
		if (fix_dist) nk_label(gui->ctx, _l("Confirm"), NK_TEXT_LEFT);
		else nk_label(gui->ctx, _l("Enter distance"), NK_TEXT_LEFT);
	}
	
	nk_checkbox_label(gui->ctx, _l("Fixed angle"), &fix_angle);
	if (fix_angle)
		angle_fixed = nk_propertyd(gui->ctx, _l("Angle"), -180.0, angle_fixed, 180.0, 1.0, 1.0);
	
	nk_checkbox_label(gui->ctx, _l("Fixed distance"), &fix_dist);
	if (fix_dist)
		dist_fixed = nk_propertyd(gui->ctx, _l("distance"), -1e9, dist_fixed, 1.0e9, SMART_STEP(dist_fixed), SMART_STEP(dist_fixed));
	
	nk_checkbox_label(gui->ctx, _l("Custom Text"), &custom_text);
	if (custom_text)
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_text, DXF_MAX_CHARS, nk_filter_default);
	
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
		dxf_dimsty dim_sty;
    dim_sty.name = strpool_inject( &name_pool, (char const*) sty_name, strlen(sty_name) );
		/*init drawing style */
		dxf_dim_get_sty(gui->drawing, &dim_sty);
		
		char tmp_str[DXF_MAX_CHARS + 1];
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim (gui->color_idx, /* color */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), /* layer */
			(char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name), /* line type */
      dxf_lw[gui->lw_idx], /* line weight */
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
		dxf_attr_change(new_dim, 3,
      (void *) strpool_cstr2( &name_pool, dim_sty.name)); 
		dxf_attr_change(new_dim, 13, &gui->step_x[0]);
		dxf_attr_change(new_dim, 23, &gui->step_y[0]);
		dxf_attr_change(new_dim, 14, &gui->step_x[1]);
		dxf_attr_change(new_dim, 24, &gui->step_y[1]);
		dxf_attr_change(new_dim, 42, &length);
		angle *= 180.0/M_PI;
		dxf_attr_change(new_dim, 50, &angle);
		
		if (custom_text)
			dxf_attr_change(new_dim, 1, user_text);
		else
			dxf_attr_change(new_dim, 1, dim_sty.post);
		
		/* distance of dimension from measure points */
		double dist = 3.0;
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
		base_x += -length/2.0 * cosine - sine * (dim_sty.gap + dim_sty.txt_size/2.0) * dir * dim_sty.scale;
		base_y += -length/2.0 * sine + cosine * (dim_sty.gap + dim_sty.txt_size/2.0) * dir * dim_sty.scale;
		dxf_attr_change(new_dim, 11, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 21, &base_y);
		
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_linear_make(gui->drawing, new_dim);
		
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
				do_add_entry(&gui->list_do, _l("Linear DIMENSION"));
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

int gui_dim_angular_info (gui_obj *gui){
	if (gui->modal != DIM_ANGULAR) return 0;
	
	static double dist_fixed = 3.0;
	static int fix_dist = 0;
	static int custom_text = 0;
	static char user_text[DXF_MAX_CHARS+1] = "<>";
  
  gui->phanton = NULL;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a Angular Dim"), NK_TEXT_LEFT);
	
	static char sty_name[DXF_MAX_CHARS+1] = "STANDARD";
	
	nk_layout_row_template_begin(gui->ctx, 20);
	nk_layout_row_template_push_static(gui->ctx, 45);
	nk_layout_row_template_push_dynamic(gui->ctx);
	nk_layout_row_template_end(gui->ctx);
	nk_label(gui->ctx, _l("Style:"), NK_TEXT_LEFT);
	/* dimension style combo selection */
	int num_dsty = dxf_count_obj(gui->drawing->t_dimst, "DIMSTYLE");
	int h = num_dsty * 25 + 5;
	h = (h < 200)? h : 200;
	if (nk_combo_begin_label(gui->ctx,  sty_name, nk_vec2(220, h))){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		dxf_node *dsty, *dsty_nm, *next = NULL;
    
    char *name = NULL;
		/* sweep DIMSTYLE table */
		dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
		while (dsty){
			/* get name of current dimstyle */
			dsty_nm = dxf_find_attr2(dsty, 2);
      name = NULL;
      if (dsty_nm) name = (char *) strpool_cstr2( &name_pool, dsty_nm->value.str);
			if (name){
				/* dimstyle name */
        if (nk_button_label(gui->ctx, name)){
					/* select current dimstyle */
          strncpy(sty_name, name, DXF_MAX_CHARS);
					
					nk_combo_close(gui->ctx);
					break;
				}
			}
			if (next)
				dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
			else
				dsty = NULL;
		}
		
		nk_combo_end(gui->ctx);
	}
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	if (gui->step == 0) nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	else if (gui->step == 1) nk_label(gui->ctx, _l("Enter start point"), NK_TEXT_LEFT);
	else if (gui->step == 2) nk_label(gui->ctx, _l("Enter end point"), NK_TEXT_LEFT);
	else {
		if (fix_dist) nk_label(gui->ctx, _l("Confirm"), NK_TEXT_LEFT);
		else nk_label(gui->ctx, _l("Enter distance"), NK_TEXT_LEFT);
	}
	
	nk_checkbox_label(gui->ctx, _l("Fixed distance"), &fix_dist);
	if (fix_dist)
		dist_fixed = nk_propertyd(gui->ctx, _l("distance"), -1e9, dist_fixed, 1.0e9, SMART_STEP(dist_fixed), SMART_STEP(dist_fixed));
	
	nk_checkbox_label(gui->ctx, _l("Custom Text"), &custom_text);
	if (custom_text)
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_text, DXF_MAX_CHARS, nk_filter_default);
	
	if (gui->step == 0){
		/* center point */
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
	else if (gui->step == 1){
		/* First point */
		if (gui->ev & EV_ENTER){
			/* to next point*/
			gui->step = 2;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
		
		/* draw a line to helps user to see direction */
		gui->draw_phanton = 0;
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			/* dashed line */
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
	} 
	else {
		dxf_dimsty dim_sty;
    dim_sty.name = strpool_inject( &name_pool, (char const*) sty_name, strlen(sty_name) );
		/*init drawing style */
		dxf_dim_get_sty(gui->drawing, &dim_sty);
		char tmp_str[DXF_MAX_CHARS + 1];
		
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim_angular (gui->color_idx, /* color */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), /* layer */
			(char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name), /* line type */
      dxf_lw[gui->lw_idx], /* line weight */
			0, FRAME_LIFE); /* paper space */
		
		dxf_attr_change(new_dim, 3,
      (void *) strpool_cstr2( &name_pool, dim_sty.name)); 
		
		/* dimension angle */
		double start = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0])); /* calculed from points */
		double end = atan2((gui->step_y[2] - gui->step_y[0]), (gui->step_x[2] - gui->step_x[0])); /* calculed from points */
		double angle = end - start;
		if (angle < 0.0) angle += 2 * M_PI;
		double an_ang = angle/2.0 + start;
		if (an_ang < 0.0) an_ang += 2 * M_PI;
		double cosine = cos(an_ang);
		double sine = sin(an_ang);
		
		
		/* distance of dimension from measure points */
		double dist = 3.0 +
			sqrt(pow(gui->step_x[1]  - gui->step_x[0], 2) + pow(gui->step_y[1] - gui->step_y[0], 2));
		double dir = 1.0;
		if(fix_dist){
			dist = dist_fixed + /* user entered distance */
				sqrt(pow(gui->step_x[1]  - gui->step_x[0], 2) + pow(gui->step_y[1] - gui->step_y[0], 2));
		}
		else if (gui->step == 3){ /* or calcule from points */
			/* It is a polarized distance in perpenticular direction from dimension base line*/
			dist = cosine*(gui->step_x[3]  - gui->step_x[0])+ sine*(gui->step_y[3] - gui->step_y[0]);
		}
		
		
		dir = (dist  > 0.0) ? 1.0 : -1.0;
		
		/* obtain dimension base point from distance */
		double base_x = gui->step_x[0] + cosine * dist;
		double base_y = gui->step_y[0] + sine * dist;
		dxf_attr_change(new_dim, 16, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 26, &base_y);
		
		/* calcule dimension annotation (text) placement point */
		base_x += cosine * dir * dim_sty.gap * dim_sty.scale;
		base_y += sine * dir * dim_sty.gap * dim_sty.scale;
		dxf_attr_change(new_dim, 11, &base_x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 21, &base_y);
		
		/* update dimension entity parameters */
		dxf_attr_change(new_dim, 13, &gui->step_x[0]);
		dxf_attr_change(new_dim, 23, &gui->step_y[0]);
		dxf_attr_change(new_dim, 15, &gui->step_x[0]);
		dxf_attr_change(new_dim, 25, &gui->step_y[0]);
		if (dist > 0.0){
			dxf_attr_change(new_dim, 10, &gui->step_x[1]);
			dxf_attr_change(new_dim, 20, &gui->step_y[1]);
			dxf_attr_change(new_dim, 14, &gui->step_x[2]);
			dxf_attr_change(new_dim, 24, &gui->step_y[2]);
		}
		else{
			dxf_attr_change(new_dim, 10, &gui->step_x[2]);
			dxf_attr_change(new_dim, 20, &gui->step_y[2]);
			dxf_attr_change(new_dim, 14, &gui->step_x[1]);
			dxf_attr_change(new_dim, 24, &gui->step_y[1]);
			angle = 2 * M_PI - angle;
			an_ang =  fmod(an_ang + M_PI, 2 * M_PI);
		}
		dxf_attr_change(new_dim, 42, &angle);
		if (custom_text)
			dxf_attr_change(new_dim, 1, user_text);
		else
			dxf_attr_change(new_dim, 1, (void*)(char[]){"<>"});
			
		/* change text justification to improve positioning */
		if ((an_ang) < M_PI*0.45)
			dxf_attr_change(new_dim, 71, (void*)(int[]){4});
		else if ((an_ang) < M_PI * 0.55)
			dxf_attr_change(new_dim, 71, (void*)(int[]){8});
		else if ((an_ang) < M_PI * 1.45)
			dxf_attr_change(new_dim, 71, (void*)(int[]){6});
		else if ((an_ang) < M_PI * 1.55)
			dxf_attr_change(new_dim, 71, (void*)(int[]){2});
		else
			dxf_attr_change(new_dim, 71, (void*)(int[]){4});
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_angular_make(gui->drawing, new_dim);
		
		/* draw phantom */
		gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
		gui->element = NULL;
		gui->draw_phanton = 1;
		
		if (gui->step == 2 && gui->ev & EV_ENTER){
			/* second point - go to confirm */
			gui->step = 3;
			gui_next_step(gui);
		}
		else if (gui->step == 3 && gui->ev & EV_ENTER){
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
				do_add_entry(&gui->list_do, _l("Angular DIMENSION"));
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

int gui_dim_radial_info (gui_obj *gui){
	if (gui->modal != DIM_RADIUS) return 0;
	static int custom_text = 0;
	static char user_text[DXF_MAX_CHARS+1] = "<>";
	static int diametric = 0;
  
  gui->phanton = NULL;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a Dimension:"), NK_TEXT_LEFT);
	
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	if (nk_option_label(gui->ctx, _l("Radial"), !diametric)) {
		diametric = 0;
	}
	if (nk_option_label(gui->ctx, _l("Diametric"), diametric)){
		diametric = 1;
	}
	
	static char sty_name[DXF_MAX_CHARS+1] = "STANDARD";
	
	nk_layout_row_template_begin(gui->ctx, 20);
	nk_layout_row_template_push_static(gui->ctx, 45);
	nk_layout_row_template_push_dynamic(gui->ctx);
	nk_layout_row_template_end(gui->ctx);
	nk_label(gui->ctx, _l("Style:"), NK_TEXT_LEFT);
	/* dimension style combo selection */
	int num_dsty = dxf_count_obj(gui->drawing->t_dimst, "DIMSTYLE");
	int h = num_dsty * 25 + 5;
	h = (h < 200)? h : 200;
	if (nk_combo_begin_label(gui->ctx,  sty_name, nk_vec2(220, h))){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		dxf_node *dsty, *dsty_nm, *next = NULL;
    
    char *name = NULL;
		/* sweep DIMSTYLE table */
		dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
		while (dsty){
			/* get name of current dimstyle */
			dsty_nm = dxf_find_attr2(dsty, 2);
      name = NULL;
      if (dsty_nm) name = (char *) strpool_cstr2( &name_pool, dsty_nm->value.str);
			if (name){
				/* dimstyle name */
        if (nk_button_label(gui->ctx, name)){
					/* select current dimstyle */
          strncpy(sty_name, name, DXF_MAX_CHARS);
					
					nk_combo_close(gui->ctx);
					break;
				}
			}
			if (next)
				dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
			else
				dsty = NULL;
		}
		
		nk_combo_end(gui->ctx);
	}
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	if (gui->step == 0){
		if (diametric) nk_label(gui->ctx, _l("Enter quadrant point"), NK_TEXT_LEFT);
		else nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	}
	else if (gui->step == 1) {
		if (diametric) nk_label(gui->ctx, _l("Enter oposite point"), NK_TEXT_LEFT);
		else nk_label(gui->ctx, _l("Enter radius point"), NK_TEXT_LEFT);
	}
	else nk_label(gui->ctx, _l("Enter location"), NK_TEXT_LEFT);
	
	nk_checkbox_label(gui->ctx, _l("Custom Text"), &custom_text);
	if (custom_text)
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_text, DXF_MAX_CHARS, nk_filter_default);
	
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
		dxf_dimsty dim_sty;
    dim_sty.name = strpool_inject( &name_pool, (char const*) sty_name, strlen(sty_name) );
		/*init drawing style */
		dxf_dim_get_sty(gui->drawing, &dim_sty);
		char tmp_str[DXF_MAX_CHARS + 1];
		
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim_radial (diametric, 
			gui->color_idx, /* color */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), /* layer */
			(char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name), /* line type */
      dxf_lw[gui->lw_idx], /* line weight */
			0, FRAME_LIFE); /* paper space */
		
		/* calcule dimension annotation (text) placement */
		double base_x = gui->step_x[0];
		double base_y = gui->step_y[0];
		if (gui->step == 2){ /* or calcule from points */
			base_x = gui->step_x[2];
			base_y = gui->step_y[2];
		}
		
		/* dimension angle */
		double angle = atan2((base_y - gui->step_y[1]), (base_x - gui->step_x[1])); /* calculed from points */
		if (angle < 0.0) angle += 2 * M_PI;
		double cosine = cos(angle);
		double sine = sin(angle);
		
		/* calcule dimension measure */
		double length = sqrt(pow(gui->step_x[1]  - gui->step_x[0], 2) + pow(gui->step_y[1] - gui->step_y[0], 2));
		
		/* update dimension entity parameters */
		dxf_attr_change(new_dim, 3,
      (void *) strpool_cstr2( &name_pool, dim_sty.name)); 
		dxf_attr_change(new_dim, 10, &gui->step_x[0]);
		dxf_attr_change(new_dim, 20, &gui->step_y[0]);
		dxf_attr_change(new_dim, 11, &base_x);
		dxf_attr_change(new_dim, 21, &base_y);
		dxf_attr_change(new_dim, 15, &gui->step_x[1]);
		dxf_attr_change(new_dim, 25, &gui->step_y[1]);
		dxf_attr_change(new_dim, 42, &length);
		
		if (custom_text)
			dxf_attr_change(new_dim, 1, user_text);
		else
			dxf_attr_change(new_dim, 1, dim_sty.post);
		
		
		/* change text justification to improve positioning */
		if ((angle) < M_PI*0.45)
			dxf_attr_change(new_dim, 71, (void*)(int[]){4});
		else if ((angle) < M_PI * 0.55)
			dxf_attr_change(new_dim, 71, (void*)(int[]){8});
		else if ((angle) < M_PI * 1.45)
			dxf_attr_change(new_dim, 71, (void*)(int[]){6});
		else if ((angle) < M_PI * 1.55)
			dxf_attr_change(new_dim, 71, (void*)(int[]){2});
		else
			dxf_attr_change(new_dim, 71, (void*)(int[]){4});
		
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_radial_make(gui->drawing, new_dim);
		
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
				if (diametric) do_add_entry(&gui->list_do, _l("Diametric DIMENSION"));
				else do_add_entry(&gui->list_do, _l("Radial DIMENSION"));
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

int gui_dim_ordinate_info (gui_obj *gui){
	if (gui->modal != DIM_ORDINATE) return 0;
	static int custom_text = 0;
	static char user_text[DXF_MAX_CHARS+1] = "<>";
	static int x_dir = 1, old_dir = 1;
	static double start = 0.0, ext = 0.0;
	double x, y;
	
  gui->phanton = NULL;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a Ordinate Dim"), NK_TEXT_LEFT);
	
	static char sty_name[DXF_MAX_CHARS+1] = "STANDARD";
	
	nk_layout_row_template_begin(gui->ctx, 20);
	nk_layout_row_template_push_static(gui->ctx, 45);
	nk_layout_row_template_push_dynamic(gui->ctx);
	nk_layout_row_template_end(gui->ctx);
	nk_label(gui->ctx, _l("Style:"), NK_TEXT_LEFT);
	/* dimension style combo selection */
	int num_dsty = dxf_count_obj(gui->drawing->t_dimst, "DIMSTYLE");
	int h = num_dsty * 25 + 5;
	h = (h < 200)? h : 200;
	if (nk_combo_begin_label(gui->ctx,  sty_name, nk_vec2(220, h))){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		dxf_node *dsty, *dsty_nm, *next = NULL;
    
    char *name = NULL;
		/* sweep DIMSTYLE table */
		dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
		while (dsty){
			/* get name of current dimstyle */
			dsty_nm = dxf_find_attr2(dsty, 2);
      name = NULL;
      if (dsty_nm) name = (char *) strpool_cstr2( &name_pool, dsty_nm->value.str);
			if (name){
				/* dimstyle name */
        if (nk_button_label(gui->ctx, name)){
					/* select current dimstyle */
          strncpy(sty_name, name, DXF_MAX_CHARS);
					
					nk_combo_close(gui->ctx);
					break;
				}
			}
			if (next)
				dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
			else
				dsty = NULL;
		}
		
		nk_combo_end(gui->ctx);
	}
	
	/* ordinate direction */
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	if (nk_option_label(gui->ctx, _l("X"), x_dir)) {
		x_dir = 1;
	}
	if (nk_option_label(gui->ctx, _l("Y"), !x_dir)){
		x_dir = 0;
	}
	if(x_dir != old_dir){
		/* go to first step, if user change the direction */
		gui_first_step(gui);
	}
	old_dir = x_dir;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	
	if (gui->step == 0) nk_label(gui->ctx, _l("Enter start point"), NK_TEXT_LEFT);
	else if (gui->step == 1) nk_label(gui->ctx, _l("Enter extension"), NK_TEXT_LEFT);
	else nk_label(gui->ctx, _l("Next ordinate"), NK_TEXT_LEFT);
	
	nk_checkbox_label(gui->ctx, _l("Custom Text"), &custom_text);
	if (custom_text)
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_text, DXF_MAX_CHARS, nk_filter_default);
	
	if (gui->step == 0){
		/* frst point */
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			/* to next point*/
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
			
			gui->step_x[2] = gui->step_x[0];
			gui->step_y[2] = gui->step_y[0];
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	} 
	else {
		dxf_dimsty dim_sty;
    dim_sty.name = strpool_inject( &name_pool, (char const*) sty_name, strlen(sty_name) );
		/*init drawing style */
		dxf_dim_get_sty(gui->drawing, &dim_sty);
		
		start = (x_dir) ? gui->step_x[0] : gui->step_y[0];
		ext = (x_dir) ? gui->step_y[1] : gui->step_x[1];
		
		char tmp_str[DXF_MAX_CHARS + 1];
		
		/* create a temporary DIMENSION entity */
		dxf_node *new_dim = dxf_new_dim_ordinate (gui->color_idx, /* color */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), /* layer */
			(char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name), /* line type */
      dxf_lw[gui->lw_idx], /* line weight */
			0, FRAME_LIFE); /* paper space */
		
		double dir = (((x_dir) ? gui->step_y[1] - gui->step_y[0] : gui->step_x[1] - gui->step_x[0]) > 0.0) ? 1.0 : -1.0;
		double cosine = (x_dir) ? 1.0 : 0.0;
		double sine = (x_dir) ? 0.0 : 1.0;
		
		x = cosine * gui->step_x[2] + sine * ext;
		y = sine * gui->step_y[2] + cosine * ext;
		
		/* calcule dimension linear length (measure). It is a polarized distance in base line direction */
		double length = 0.0;
		length = cosine*(gui->step_x[2]  - start) + sine*(gui->step_y[2] - start);
		
		/* update dimension entity parameters */
		dxf_attr_change(new_dim, 3,
      (void *) strpool_cstr2( &name_pool, dim_sty.name)); 
		dxf_attr_change(new_dim, 10, &gui->step_x[0]);
		dxf_attr_change(new_dim, 20, &gui->step_y[0]);
		dxf_attr_change(new_dim, 13, &gui->step_x[2]);
		dxf_attr_change(new_dim, 23, &gui->step_y[2]);
		dxf_attr_change(new_dim, 14, &x);
		dxf_attr_change(new_dim, 24, &y);
		dxf_attr_change(new_dim, 42, &length);
		if (x_dir) dxf_attr_change(new_dim, 70, (void *) (int []){102});
		else  dxf_attr_change(new_dim, 70, (void *) (int []){38});
		
		if (custom_text)
			dxf_attr_change(new_dim, 1, user_text);
		else
			dxf_attr_change(new_dim, 1, dim_sty.post);
		
		/* calcule dimension annotation (text) placement point (outer from perpenticular direction)*/
		x += dir * dim_sty.gap * sine * dim_sty.scale;
		y +=  dir * dim_sty.gap * cosine * dim_sty.scale;
		dxf_attr_change(new_dim, 11, &x); /* update dimension entity parameters */
		dxf_attr_change(new_dim, 21, &y);
		/* change text justification to improve positioning */
		if (dir > 0.0)
			dxf_attr_change(new_dim, 71, (void*)(int[]){4});
		else
			dxf_attr_change(new_dim, 71, (void*)(int[]){6});
		
		/* create dimension block contents as a list of entities ("render" the dimension "picture") */
		list_node *list = dxf_dim_ordinate_make(gui->drawing, new_dim);
		
		/* draw phantom */
		gui->phanton = dxf_list_parse(gui->drawing, list, 0, FRAME_LIFE);
		gui->element = NULL;
		gui->draw_phanton = 1;
		
		if (gui->ev & EV_ENTER){
			/* confirm - create a new DXF dim */
			gui->step = 2;
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
				do_add_entry(&gui->list_do, _l("Ordinate DIMENSION"));
				do_add_item(gui->list_do.current, NULL, blkrec);
				do_add_item(gui->list_do.current, NULL, blk);
				do_add_item(gui->list_do.current, NULL, new_ent);
			}
			
			
		}
		else if (gui->ev & EV_CANCEL){ /* back to first point, if user cancel operation */
			gui_first_step(gui);
		}
	}
	
	return 1;
}

int gui_dim_mng (gui_obj *gui){
	int show_config = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 500;
	gui->next_win_h = 480;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "config", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, _l("Dimension Style Manager"), nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		static int dsty_i = -1, show_name = 0;
		struct nk_style_button *sel_type;
		static char new_name[DXF_MAX_CHARS+1] = "";
		
		dxf_dimsty_use(gui->drawing); /* update DIMSTYLEs in use */
		
		//nk_layout_row_dynamic(gui->ctx, 420, 2);
		nk_layout_row_template_begin(gui->ctx, 430);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_push_static(gui->ctx, 200);
		nk_layout_row_template_end(gui->ctx);
		if (nk_group_begin(gui->ctx, _l("List of styles"), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			
			nk_layout_row_dynamic(gui->ctx, 32, 1);
			if (nk_group_begin(gui->ctx, "dimsty_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_template_begin(gui->ctx, 20);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_push_static(gui->ctx, 50);
				nk_layout_row_template_push_static(gui->ctx, 8);
				nk_layout_row_template_end(gui->ctx);
				
				if (nk_button_label(gui->ctx, _l("Name"))){
					
				}
				if (nk_button_label(gui->ctx, _l("Used"))){
					
				}
				nk_group_end(gui->ctx);
			}
			
			nk_layout_row_dynamic(gui->ctx, 280, 1);
			if (nk_group_begin(gui->ctx, "dimsty_name", NK_WINDOW_BORDER)) {
				nk_layout_row_template_begin(gui->ctx, 20);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_push_static(gui->ctx, 50);
				nk_layout_row_template_end(gui->ctx);
				
				dxf_node *dsty, *dsty_nm, *next = NULL;
				
				/* sweep DIMSTYLE table */
				i = 0; next = NULL;
				dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
				while (dsty){
					/* get name of current dimstyle */
					dsty_nm = dxf_find_attr2(dsty, 2);
					if (dsty_nm){
						
						/* verify if curren dimstyle is selected */
						sel_type = &gui->b_icon_unsel;
						if (dsty_i == i) sel_type = &gui->b_icon_sel;
						/* dimstyle name */
						if (nk_button_label_styled(gui->ctx, sel_type,
              (void *) strpool_cstr2( &name_pool, dsty_nm->value.str))){ 
							/* select current dimstyle */
							dsty_i = i;
						}
						/* verify if DIMSTYLE is used in drawing, by count in layer index*/
						if (dsty->obj.layer > 0)
							nk_label(gui->ctx, "x", NK_TEXT_CENTERED);
						else nk_label(gui->ctx, " ", NK_TEXT_CENTERED);
					}
					i++;
					if (next)
						dsty = dxf_find_obj_nxt(gui->drawing->t_dimst, &next, "DIMSTYLE");
					else
						dsty = NULL;
				}
				if (dsty_i >= i) dsty_i = -1;
				nk_group_end(gui->ctx);
			}
			nk_layout_row_dynamic(gui->ctx, 10, 1);
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			if (nk_button_label(gui->ctx, _l("Create"))){
				show_name = 1;
				new_name[0] = 0;
			}
			if (nk_button_label(gui->ctx, _l("Delete"))){
				dxf_node *dsty = NULL;
				if (dsty_i >= 0)
					dsty = dxf_find_obj_i(gui->drawing->t_dimst, "DIMSTYLE", dsty_i);
				if (dsty){
					if (dsty->obj.layer > 0){
						snprintf(gui->log_msg, 63, _l("Error: Don't remove DIMSTYLE in use"));
					}
					else{
						/* remove layer from main structure */
						do_add_entry(&gui->list_do, _l("Remove DIMSTYLE"));
						do_add_item(gui->list_do.current, dsty, NULL);
						dxf_obj_subst(dsty, NULL);
						dsty_i = -1;
					}
				}
			}
			nk_group_end(gui->ctx);
		}
		if (nk_group_begin(gui->ctx, _l("Selected parameters"), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			static char dimscale_str[64] = "1";
			static char dimlfac_str[64] = "1";
			static char dimdec_str[64] = "2";
			static char dimpost_str[DXF_MAX_CHARS+1] = "<>";
			static char dimasz_str[64] = "1";
			static char dimexo_str[64] = "1";
			static char dimexe_str[64] = "1";
			static char dimtxt_str[64] = "1";
			static char dimgap_str[64] = "1";
			static int adv_param = 0;
			nk_flags res;
			dxf_node *dsty = NULL;
			if (dsty_i >= 0)
				dsty = dxf_find_obj_i(gui->drawing->t_dimst, "DIMSTYLE", dsty_i);
			if (dsty){
				dxf_dimsty dim_sty;
				dxf_node *dsty_nm = dxf_find_attr2(dsty, 2);
				if (dsty_nm){
          dim_sty.name = dsty_nm->value.str;
				}
				
				dxf_dim_get_sty(gui->drawing, &dim_sty);
				
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label_colored(gui->ctx, strpool_cstr2( &name_pool, dim_sty.name),
          NK_TEXT_CENTERED, nk_rgb(255,255,0));
				/* edit global dim scale */
				nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.6, 0.4});
				nk_label(gui->ctx, _l("Global Scale"), NK_TEXT_LEFT);
				res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimscale_str, 63, nk_filter_float);
				
				if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
					nk_edit_unfocus(gui->ctx);
					if (strlen(dimscale_str)) /* update parameter value */
						dim_sty.scale = atof(dimscale_str);
					snprintf(dimscale_str, 63, "%.9g", dim_sty.scale);
					
					/* update in DXF main struct */
					dxf_dim_update_sty(gui->drawing, &dim_sty);
				}
        else if (!(res & NK_EDIT_ACTIVE)){
					snprintf(dimscale_str, 63, "%.9g", dim_sty.scale);
				}
				
				/* edit measure dim scale */
				nk_label(gui->ctx, _l("Meas. Factor"), NK_TEXT_LEFT);
				res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimlfac_str, 63, nk_filter_float);
				
				if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
					nk_edit_unfocus(gui->ctx);
					if (strlen(dimlfac_str)) /* update parameter value */
						dim_sty.an_scale = atof(dimlfac_str);
					snprintf(dimlfac_str, 63, "%.9g", dim_sty.an_scale);
					/* update in DXF main struct */
					dxf_dim_update_sty(gui->drawing, &dim_sty);
				}
        else if (!(res & NK_EDIT_ACTIVE)){
					snprintf(dimlfac_str, 63, "%.9g", dim_sty.an_scale);
				}
				
				/* edit decimal places (precision) */
				//nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.6, 0.4});
				nk_label(gui->ctx, _l("Dec. places"), NK_TEXT_LEFT);
				res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimdec_str, 63, nk_filter_decimal);
				
				if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
					nk_edit_unfocus(gui->ctx);
					if (strlen(dimdec_str)) /* update parameter value */
						dim_sty.dec = atoi(dimdec_str);
					snprintf(dimdec_str, 63, "%d", dim_sty.dec);
					/* update in DXF main struct */
					dxf_dim_update_sty(gui->drawing, &dim_sty);
				}
        else if (!(res & NK_EDIT_ACTIVE)){
					snprintf(dimdec_str, 63, "%d", dim_sty.dec);
				}
				
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, _l("Annotation text:"), NK_TEXT_LEFT);
				res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimpost_str, DXF_MAX_CHARS, nk_filter_default);
				
				if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
					nk_edit_unfocus(gui->ctx);
					strncpy(dim_sty.post, dimpost_str, DXF_MAX_CHARS);
					/* update in DXF main struct */
					dxf_dim_update_sty(gui->drawing, &dim_sty);
				}
        else if (!(res & NK_EDIT_ACTIVE)){
					strncpy(dimpost_str, dim_sty.post, DXF_MAX_CHARS);
				}
				
				nk_label(gui->ctx, _l("Annotation text style:"), NK_TEXT_LEFT);
				char sty_name[DXF_MAX_CHARS+1] = "";
				int num_tstyles = gui->drawing->num_tstyles;
				dxf_tstyle *t_sty = gui->drawing->text_styles;
				if (dim_sty.tstyle >= 0)
					strncpy(sty_name,  /* select current style */
            strpool_cstr2( &name_pool, t_sty[dim_sty.tstyle].name),
            DXF_MAX_CHARS);
				/* text style combo selection */
				int h = num_tstyles * 25 + 5;
				h = (h < 200)? h : 200;
				if (nk_combo_begin_label(gui->ctx,  sty_name, nk_vec2(220, h))){
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					int j = 0;
					for (j = 0; j < num_tstyles; j++){
            if (!t_sty[j].name) continue; /* show only the named styles */
						if (nk_button_label(gui->ctx,
              strpool_cstr2( &name_pool, t_sty[j].name))){
							dim_sty.tstyle = j;
							
							/* update in DXF main struct */
							dxf_dim_update_sty(gui->drawing, &dim_sty);
							
							nk_combo_close(gui->ctx);
							break;
						}
					}
					nk_combo_end(gui->ctx);
				}
				
				nk_label(gui->ctx, _l("Terminator:"), NK_TEXT_LEFT);
				const char *term_typ[] = { "Filled", "Open", "Open30", "Open90", "Closed", "Oblique", "ArchTick", "None"};
				if (nk_combo_begin_label(gui->ctx,  dim_sty.a_type, nk_vec2(150, 150))){
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					int j = 0;
					for (j = 0; j < 8; j++){
						if (nk_button_label(gui->ctx, term_typ[j])){
							strncpy(dim_sty.a_type, term_typ[j], DXF_MAX_CHARS); /* select current style */
							/* update in DXF main struct */
							dxf_dim_update_sty(gui->drawing, &dim_sty);
							
							nk_combo_close(gui->ctx);
							break;
						}
					}
					nk_combo_end(gui->ctx);
				}
				
				nk_checkbox_label(gui->ctx, _l("Advanced"), &adv_param);
				if (adv_param) {
					/* arrow size */
					nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.6, 0.4});
					nk_label(gui->ctx, _l("Term. size"), NK_TEXT_LEFT);
					res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimasz_str, 63, nk_filter_float);
					
					if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
						nk_edit_unfocus(gui->ctx);
						if (strlen(dimasz_str)) /* update parameter value */
							dim_sty.a_size = atof(dimasz_str);
						snprintf(dimasz_str, 63, "%.9g", dim_sty.a_size);
						
						/* update in DXF main struct */
						dxf_dim_update_sty(gui->drawing, &dim_sty);
					}
					else if (!(res & NK_EDIT_ACTIVE)){
						snprintf(dimasz_str, 63, "%.9g", dim_sty.a_size);
					}
          
					/* extension line offset */
					nk_label(gui->ctx, _l("Offset"), NK_TEXT_LEFT);
					res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimexo_str, 63, nk_filter_float);
					
					if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
						nk_edit_unfocus(gui->ctx);
						if (strlen(dimexo_str)) /* update parameter value */
							dim_sty.ext_ofs = atof(dimexo_str);
						snprintf(dimexo_str, 63, "%.9g", dim_sty.ext_ofs);
						
						/* update in DXF main struct */
						dxf_dim_update_sty(gui->drawing, &dim_sty);
					}
          else if (!(res & NK_EDIT_ACTIVE)){
						snprintf(dimexo_str, 63, "%.9g", dim_sty.ext_ofs);
					}
					
					/* extension line extend */
					nk_label(gui->ctx, _l("Extension"), NK_TEXT_LEFT);
					res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimexe_str, 63, nk_filter_float);
					
					if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
						nk_edit_unfocus(gui->ctx);
						if (strlen(dimexe_str)) /* update parameter value */
							dim_sty.ext_e = atof(dimexe_str);
						snprintf(dimexe_str, 63, "%.9g", dim_sty.ext_e);
						
						/* update in DXF main struct */
						dxf_dim_update_sty(gui->drawing, &dim_sty);
					}
          else if (!(res & NK_EDIT_ACTIVE)){
						snprintf(dimexe_str, 63, "%.9g", dim_sty.ext_e);
					}
					
					/* text size */
					nk_label(gui->ctx, _l("Text size"), NK_TEXT_LEFT);
					res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimtxt_str, 63, nk_filter_float);
					
					if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
						nk_edit_unfocus(gui->ctx);
						if (strlen(dimtxt_str)) /* update parameter value */
							dim_sty.txt_size = atof(dimtxt_str);
						snprintf(dimtxt_str, 63, "%.9g", dim_sty.txt_size);
						
						/* update in DXF main struct */
						dxf_dim_update_sty(gui->drawing, &dim_sty);
					}
          else if (!(res & NK_EDIT_ACTIVE)){
						snprintf(dimtxt_str, 63, "%.9g", dim_sty.txt_size);
					}
					
					/* text gap */
					nk_label(gui->ctx, _l("Text gap"), NK_TEXT_LEFT);
					res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, dimgap_str, 63, nk_filter_float);
					
					if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
						nk_edit_unfocus(gui->ctx);
						if (strlen(dimgap_str)) /* update parameter value */
							dim_sty.gap = atof(dimgap_str);
						snprintf(dimgap_str, 63, "%.9g", dim_sty.gap);
						
						/* update in DXF main struct */
						dxf_dim_update_sty(gui->drawing, &dim_sty);
					}
          else if (!(res & NK_EDIT_ACTIVE)){
						snprintf(dimgap_str, 63, "%.9g", dim_sty.gap);
					}
				}
			}
			nk_group_end(gui->ctx);
		}
		
		/* popup to entering new DIMSTYLE name */
		if ((show_name)){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, _l("Dim Style Name"), NK_WINDOW_CLOSABLE, nk_rect(10, 20, 220, 100))){
				
				/* get name */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, new_name, DXF_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, _l("OK"))){
					dxf_dimsty dim_sty;
          dim_sty.name = strpool_inject( &name_pool, (char const*) new_name, strlen(new_name) );
					if (dxf_dim_get_sty(gui->drawing, &dim_sty) != -1){
						/* fail to  create, commonly name already exists */
						snprintf(gui->log_msg, 63, _l("Error: DIMSTYLE already exists"));
					}
					else {
						/* create and attach in drawing main structure */
						dxf_new_dim_sty (gui->drawing, dim_sty);
						
						nk_popup_close(gui->ctx);
						show_name = 0;
						dsty_i = -1;
					}
					
				}
			/* ------------ cancel or close window ---------------- */
				if (nk_button_label(gui->ctx, _l("Cancel"))){
					nk_popup_close(gui->ctx);
					show_name = 0;
					dsty_i = -1;
				}
				nk_popup_end(gui->ctx);
			} else {
				show_name = 0;
				dsty_i = -1;
			}
		}
		
	} else show_config = 0;
	nk_end(gui->ctx);
	
	return show_config;
}