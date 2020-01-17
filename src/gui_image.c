#include "gui_use.h"

int gui_image_interactive(gui_obj *gui){
	if (gui->modal != IMAGE) return 0;
	
		
	if (gui->step == 0){
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
			gui->draw_phanton = 0;
			
			gui_default_modal(gui);
		}
	}
	else{
		if (gui->ev & EV_ENTER){
			/* ends rectangle selection */
			double rect_pt1[2], rect_pt2[2];
			rect_pt1[0] = (gui->step_x[gui->step - 1] < gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
			rect_pt1[1] = (gui->step_y[gui->step - 1] < gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
			rect_pt2[0] = (gui->step_x[gui->step - 1] > gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
			rect_pt2[1] = (gui->step_y[gui->step - 1] > gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
			
			
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	if (gui->step > 0){
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		
		graph_obj *graph = graph_new(FRAME_LIFE);
	
		if (graph){
			if (gui->step_x[gui->step - 1] > gui->step_x[gui->step]){
				/* dashed rectangle, if the selection is by intersection */
				graph->patt_size = 2;
				graph->pattern[0] = 10 / gui->zoom;
				graph->pattern[1] = -10 / gui->zoom;
			}
			
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

int gui_image_info (gui_obj *gui){
	if (gui->modal == IMAGE) {
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place Raster Image", NK_TEXT_LEFT);
		
	}
	return 1;
}