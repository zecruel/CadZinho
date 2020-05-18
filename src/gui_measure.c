#include "gui_use.h"

int gui_measure_interactive(gui_obj *gui){
	int i;
	if (gui->modal == MEASURE){
		
		if (gui->step == 0){
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){ /* first point entered */
				gui->element = NULL;
				gui->step = 1;
				gui->en_distance = 1;
				gui->step_x[gui->step] = gui->step_x[gui->step-1];
				gui->step_y[gui->step] = gui->step_y[gui->step-1];
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){ /* quit measure mode */
				gui_default_modal(gui);
			}
		}
		else{
			if (gui->ev & EV_ENTER){ /* new point entered */
				if(gui->step < 998) gui->step++;
				gui->step_x[gui->step] = gui->step_x[gui->step-1];
				gui->step_y[gui->step] = gui->step_y[gui->step-1];
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){ /* back to first point */
				gui_first_step(gui);
			}
			
			/* draw a sequence line to helps user */
			gui->draw_phanton = 0;
			gui->phanton = list_new(NULL, FRAME_LIFE);
			graph_obj *graph = graph_new(FRAME_LIFE);
			if (graph){
				gui->draw_phanton = 1;
				/* dashed line */
				graph->patt_size = 2;
				graph->pattern[0] = 10 / gui->zoom;
				graph->pattern[1] = -10 / gui->zoom;
				/* conect entered points */
				for (i = 0; i < gui->step; i++){
					line_add(graph, gui->step_x[i], gui->step_y[i], 0,
						gui->step_x[i+1], gui->step_y[i+1], 0);
				}
				list_node * new_node = list_new(graph, FRAME_LIFE);
				list_push(gui->phanton, new_node);
			}
		}
	}
	return 1;
}

int gui_measure_info (gui_obj *gui){
	if (gui->modal == MEASURE) {
		static char pt_str[64];
		static char dist_str[64], x_str[64], y_str[64], total_str[64];
		double distance = 0.0, total = 0.0, x = 0.0, y = 0.0;
		int i;
		static int init = 0;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Measure Distance", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter next point", NK_TEXT_LEFT);
			
			/* calcule distance between previous and current points */
			x = gui->step_x[gui->step] - gui->step_x[gui->step-1];
			y = gui->step_y[gui->step] - gui->step_y[gui->step-1];
			distance = sqrt(x*x + y*y);
			
			/* update strings if has relevant information */
			if (fabs(distance) > 1e-9){
				snprintf(dist_str, 63, "Distance: %.9g", distance);
				snprintf(x_str, 63, "X: %.9g", x);
				snprintf(y_str, 63, "Y: %.9g", y);
				
				/* calcule total distance along all points entered */
				total = 0.0;
				for (i = 0; i < gui->step; i++){
					x = gui->step_x[i+1] - gui->step_x[i];
					y = gui->step_y[i+1] - gui->step_y[i];
					total += sqrt(x*x + y*y);
				}
				snprintf(pt_str, 63, "Points: %d", gui->step + 1);
				snprintf(total_str, 63, "Total: %.9g", total);
				
				init = 1; /* strings updated*/
			}
		}
		
		if (init){ /* show informations */
			nk_layout_row_dynamic(gui->ctx, 15, 1);
			nk_label(gui->ctx, dist_str, NK_TEXT_LEFT);
			nk_label(gui->ctx, x_str, NK_TEXT_LEFT);
			nk_label(gui->ctx, y_str, NK_TEXT_LEFT);
			nk_label(gui->ctx, pt_str, NK_TEXT_LEFT);
			nk_label(gui->ctx, total_str, NK_TEXT_LEFT);
		}
	}
	return 1;
}