#include "gui_use.h"

int gui_arc_interactive(gui_obj *gui){
	
	if (gui->modal != ARC) return 0;
	static int dir = 0;
	static double prev_a = 0;
	static double acc_a = 0;
	
	static dxf_node *new_el;
	if (gui->step == 0){
		if (gui->ev & EV_ENTER){
			dir = 0;
			
			//gui->element = new_el;
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
	else if (gui->step == 1){
		if (gui->ev & EV_ENTER){
			prev_a = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0]));
			if (prev_a < 0) prev_a = 2*M_PI + prev_a;
			acc_a = 0.0;
			gui->step = 2;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
	}
	else{
		if (gui->ev & EV_ENTER){
			double radius = sqrt(pow((gui->step_x[1] - gui->step_x[0]), 2) + pow((gui->step_y[1] - gui->step_y[0]), 2));
			double start = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0]));
			start *= 180/M_PI;
			double end = atan2((gui->step_y[2] - gui->step_y[0]), (gui->step_x[2] - gui->step_x[0]));
			end *= 180/M_PI;
			
			if (dir == -1){
				double tmp = start;
				start = end;
				end = tmp;
			}
			
			/* create a new DXF arc */
			new_el = (dxf_node *) dxf_new_arc (
				gui->step_x[0], gui->step_y[0], 0.0, /* center */
				radius, start, end,/* radius, start angle, end angle */
				gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, DWG_LIFE); /* paper space */
			
			new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
			drawing_ent_append(gui->drawing, new_el);
			
			do_add_entry(&gui->list_do, "ARC");
			do_add_item(gui->list_do.current, NULL, new_el);
			
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
		
	}
	
	if (gui->step == 1){
		gui->draw_phanton = 0;
		
		double radius = sqrt(pow((gui->step_x[1] - gui->step_x[0]), 2) + pow((gui->step_y[1] - gui->step_y[0]), 2));
		gui->phanton = list_new(NULL, FRAME_LIFE);

		graph_obj *graph = graph_new(FRAME_LIFE);
	
		if (graph){
			gui->draw_phanton = 1;
			
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			
			graph_arc(graph, gui->step_x[0], gui->step_y[0], 0.0, radius, 0, 0, 1);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
	}
	else if (gui->step > 1 && gui->ev & EV_MOTION){
		gui->draw_phanton = 0;
		
		double radius = sqrt(pow((gui->step_x[1] - gui->step_x[0]), 2) + pow((gui->step_y[1] - gui->step_y[0]), 2));
		double start = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0]));
		double end = atan2((gui->step_y[2] - gui->step_y[0]), (gui->step_x[2] - gui->step_x[0]));
		
		if (start < 0) start = 2*M_PI + start;
		if (end < 0) end = 2*M_PI + end;
		
		
		
		if (dir == 0){
			double a = end - start;
			if (a < 0) {
				dir = -1;
			}
			else dir = 1;
		}
		
		
		/*
		if (fabs(acc_a) >= (2*M_PI - 0.2) ){
			acc_a = 0.0;
			dir *= -1;
		}
		
		acc_a += (end - prev_a) * dir;
		
		prev_a = end;*/
		
		if (dir == -1){
			double tmp = start;
			start = end;
			end = tmp;
		}
		
		
		end *= 180/M_PI;
		start *= 180/M_PI;
		
		
		
		gui->phanton = list_new(NULL, FRAME_LIFE);

		graph_obj *graph = graph_new(FRAME_LIFE);
	
		if (graph){
			gui->draw_phanton = 1;
			
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			
			graph_arc(graph, gui->step_x[0], gui->step_y[0], 0.0, radius, end, start, 1);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		graph = graph_new(FRAME_LIFE);

		if (graph){
			gui->draw_phanton = 1;
			
			graph_arc(graph, gui->step_x[0], gui->step_y[0], 0.0, radius, start, end, 1);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
	}
	
	return 1;
}

int gui_arc_info (gui_obj *gui){
	if (gui->modal != ARC) return 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Place a arc", NK_TEXT_LEFT);
	if (gui->step == 0){
		nk_label(gui->ctx, "Enter center point", NK_TEXT_LEFT);
	} else {
		nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
	}
	
	return 1;
}