#include "gui_use.h"

int gui_circle_interactive(gui_obj *gui){
	
	if (gui->modal != CIRCLE) return 0;
	if (gui->circle_mode == CIRCLE_FULL){
		static dxf_node *new_el;
		if (gui->step == 0){
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* create a new DXF circle */
				new_el = (dxf_node *) dxf_new_circle (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, /* pt1, radius */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE); /* paper space */
				gui->element = new_el;
				gui->step = 1;
				gui->en_distance = 1;
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				double radius = sqrt(pow((gui->step_x[gui->step] - gui->step_x[gui->step - 1]), 2) + pow((gui->step_y[gui->step] - gui->step_y[gui->step - 1]), 2));
				dxf_attr_change(new_el, 40, &radius);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "CIRCLE");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui_first_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
			}
			if (gui->ev & EV_MOTION){
				double x1 = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
				double y1 = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
				double radius = sqrt(pow((gui->step_x[gui->step] - gui->step_x[gui->step - 1]), 2) + pow((gui->step_y[gui->step] - gui->step_y[gui->step - 1]), 2));
				
				dxf_attr_change(new_el, 40, &radius);
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	else {
		static int dir = 0;
		static double prev_a = 0;
		static double acc_a = 0;
		
		static dxf_node *new_el;
		if (gui->step == 0){
			gui->free_sel = 0;
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
	}
	return 1;
}

int gui_circle_info (gui_obj *gui){
	static const char *mode[] = {"Full circle","Circular arc"};
	if (gui->modal == CIRCLE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place a circular arc", NK_TEXT_LEFT);
		
		int h = 2 * 25 + 5;
		gui->circle_mode = nk_combo(gui->ctx, mode, 2, gui->circle_mode, 20, nk_vec2(150, h));
		
		
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter center point", NK_TEXT_LEFT);
		}
		else if (gui->step == 1){
			if (gui->circle_mode == CIRCLE_FULL){
				nk_label(gui->ctx, "Enter circle end point", NK_TEXT_LEFT);
			}
			else {
				nk_label(gui->ctx, "Enter arc start point", NK_TEXT_LEFT);
			}
		}
		else {
			if (gui->circle_mode == CIRCLE_FULL){
				gui->step = 0;
			}
			else {
				nk_label(gui->ctx, "Enter arc end point", NK_TEXT_LEFT);
			}
		}
	}
	return 1;
}