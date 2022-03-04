#include "gui_use.h"

int gui_zoom_info (gui_obj *gui){
	if (gui->modal != ZOOM) return 0;
	//static int prev_type = -1;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Zoom in rectangle", NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, "Enter first corner", NK_TEXT_LEFT);
		if (gui->ev & EV_ENTER){
			/* starts rectangle - first corner*/
			gui->draw_phanton = 1;
			
			gui->step_x[gui->step + 1] = gui->step_x[gui->step];
			gui->step_y[gui->step + 1] = gui->step_y[gui->step]; 
			
			gui->step = 1;
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			
			gui->draw = 1;
			gui->draw_phanton = 0;
			
			gui_default_modal(gui);
		}
	}
	else{
		nk_label(gui->ctx, "Enter zoom area", NK_TEXT_LEFT);
		if (gui->ev & EV_ENTER){
			/* ends rectangle zoom */
			double rect_pt1[2], rect_pt2[2];
			rect_pt1[0] = (gui->step_x[gui->step - 1] < gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
			rect_pt1[1] = (gui->step_y[gui->step - 1] < gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
			rect_pt2[0] = (gui->step_x[gui->step - 1] > gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
			rect_pt2[1] = (gui->step_y[gui->step - 1] > gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
			
			double zoom_x = 1.0, zoom_y = 1.0;
	
			zoom_x = fabs(rect_pt2[0] - rect_pt1[0]) / gui->win_w;
			zoom_y = fabs(rect_pt2[1] - rect_pt1[1]) / gui->win_h;
			
			gui->zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
			gui->zoom = 1/(1.1 * (gui->zoom));
			
			gui->ofs_x = rect_pt1[0] - (fabs((rect_pt2[0] - rect_pt1[0]) * gui->zoom - gui->win_w) / 2.0) / gui->zoom;
			gui->ofs_y = rect_pt1[1] - (fabs((rect_pt2[1] - rect_pt1[1]) * gui->zoom - gui->win_h) / 2.0) / gui->zoom;
			
			
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			/* cancel rectangle zoom */
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	if (gui->step > 0){
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		
		graph_obj *graph = graph_new(FRAME_LIFE);
	
		if (graph){
			
			/* //dashed rectangle
			if (gui->step_x[gui->step - 1] > gui->step_x[gui->step]){
				graph->patt_size = 2;
				graph->pattern[0] = 10 / gui->zoom;
				graph->pattern[1] = -10 / gui->zoom;
			}*/
			
			line_add(graph, gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0,
				gui->step_x[gui->step], gui->step_y[gui->step - 1], 0);
			line_add(graph, gui->step_x[gui->step], gui->step_y[gui->step - 1], 0,
				gui->step_x[gui->step], gui->step_y[gui->step], 0);
			line_add(graph, gui->step_x[gui->step], gui->step_y[gui->step], 0,
				gui->step_x[gui->step - 1], gui->step_y[gui->step], 0);
			line_add(graph, gui->step_x[gui->step - 1], gui->step_y[gui->step], 0,
				gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
	}
	
	return 1;
}