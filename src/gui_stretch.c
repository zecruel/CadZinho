#include "gui_use.h"

int gui_stretch_interactive(gui_obj *gui){

	if (gui->modal != STRETCH) return 0;
	
	static double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0, ref_x, ref_y;
	
	if (gui->step == 0){
		gui->free_sel = 0;
		if (gui->ev & EV_ENTER){
			/* starts rectangle - first corner*/
			gui->draw_phanton = 1;
      
      x0 = gui->step_x[gui->step];
			y0 = gui->step_y[gui->step];
      x1 = x0; y1 = y0;
			
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
			
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_default_modal(gui);
		}
	}
	else if (gui->step == 1){
		
		x1 = gui->step_x[gui->step];
    y1 = gui->step_y[gui->step];
		
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
      gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
      
			gui->step = 2;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	else if (gui->step == 2){
		
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
      gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
			ref_x = gui->step_x[gui->step];
      ref_y = gui->step_y[gui->step];
      gui->step = 3;
			gui->en_distance = 1;
			gui_next_step(gui);
		
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	else if (gui->step == 3){
		double ox, oy;
		ox = gui->step_x[gui->step] - ref_x;
    oy = gui->step_y[gui->step] - ref_y;
		
		/* draw the rectangles */
		/* first position */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
      /* dashed rectangle */
      graph->patt_size = 2;
      graph->pattern[0] = 10 / gui->zoom;
      graph->pattern[1] = -10 / gui->zoom;
      
			line_add(graph, x0, y0, 0, x1, y0, 0);
			line_add(graph, x1, y0, 0, x1, y1, 0);
			line_add(graph, x1, y1, 0, x0, y1, 0);
			line_add(graph, x0, y1, 0, x0, y0, 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		/* final position */
		graph_obj *graph2 = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph2, x0 + ox, y0 + oy, 0, x1 + ox, y0 + oy, 0);
			line_add(graph2, x1 + ox, y0 + oy, 0, x1 + ox, y1 + oy, 0);
			line_add(graph2, x1 + ox, y1 + oy, 0, x0 + ox, y1 + oy, 0);
			line_add(graph2, x0 + ox, y1 + oy, 0, x0 + ox, y0 + oy, 0);
			
			list_node * new_node = list_new(graph2, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
      #if(0)
			/* place image */
			double rect_pt1[2], rect_pt2[2], u[3], v[3];
			
			/* sort rectangle corners */
			rect_pt1[0] = (x < gui->step_x[gui->step - 1]) ? x : gui->step_x[gui->step - 1];
			rect_pt1[1] = (y < gui->step_y[gui->step - 1]) ? y : gui->step_y[gui->step - 1];
			rect_pt2[0] = (x > gui->step_x[gui->step - 1]) ? x : gui->step_x[gui->step - 1];
			rect_pt2[1] = (y > gui->step_y[gui->step - 1]) ? y : gui->step_y[gui->step - 1];
			
			
			/* get dimmension vectors */
			u[0] = 0.0;
			u[1] = 0.0;
			u[2] = 0.0;
			
			v[0] = 0.0;
			v[1] = 0.0;
			v[2] = 0.0;
			
			u[0] = fabs(rect_pt2[0] - rect_pt1[0])/(double)gui->image_w;
			v[1] = fabs(rect_pt2[1] - rect_pt1[1])/(double)gui->image_h;
			
			/* create image entity */
			dxf_node * new_el = dxf_new_image (gui->drawing,
				rect_pt1[0], rect_pt1[1], 0.0,
				u, v, (double)gui->image_w, (double)gui->image_h,
				gui->image_path,
				gui->color_idx, /* color, layer */
        (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
				/* line type, line weight */
        (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
        dxf_lw[gui->lw_idx],
				0, DWG_LIFE); /* paper space */
			
			/* draw entity */
			new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
			drawing_ent_append(gui->drawing, new_el);
			
			/* append to undo/redo list */
			do_add_entry(&gui->list_do, _l("STRETCH"));
			do_add_item(gui->list_do.current, NULL, new_el);
			#endif
      
      /* sort rectangle corners */
      double rect_pt1[2], rect_pt2[2];
      rect_pt1[0] = (x0 < x1) ? x0 : x1;
      rect_pt1[1] = (y0 < y1) ? y0 : y1;
      rect_pt2[0] = (x0 > x1) ? x0 : x1;
      rect_pt2[1] = (y0 > y1) ? y0 : y1;
      
      /* list of objects to select */
      list_node *list = list_new(NULL, FRAME_LIFE);
      list_clear(list);
      int count = 0;
      
      /* get inside objects and also all intersecting to rectangle */
      count = dxf_ents_isect2(list, gui->drawing, rect_pt1, rect_pt2);
      
      
      
			/* restart the proccess */
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->step = 2;
		}
	}
	
	return 1;
}

int gui_stretch_info (gui_obj *gui){
	if (gui->modal == STRETCH) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Stretch inside"), NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, _l("First corner"), NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			nk_label(gui->ctx, _l("Last corner"), NK_TEXT_LEFT);
		}
		else if (gui->step == 2){
			nk_label(gui->ctx, _l("Enter base point"), NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, _l("Enter destination point"), NK_TEXT_LEFT);
		}
	}
	return 1;
}