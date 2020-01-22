#include "gui_use.h"
#include "stb_image.h"

int gui_image_interactive(gui_obj *gui){

	if (gui->modal != IMAGE) return 0;
	
	static double x = 0.0, y = 0.0;
	
	if (gui->step == 0 && (gui->ev & EV_CANCEL)){
		gui->draw_phanton = 0;
		gui_default_modal(gui);
	}
		
	if (gui->step == 1){
		if (gui->ev & EV_ENTER){
			/* starts rectangle - first corner*/
			gui->draw_phanton = 1;
			
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
		
		if (gui->proportional){
			/* mantain the original image proportion */
			double w_ratio = (double)gui->image_w/(double)gui->image_h;
			double h_ratio = (double)gui->image_h/(double)gui->image_w;
			double sig = 1.0;
			
			/* choose mandatory axis to calcule final dimension */
			if (fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1])*h_ratio <
				fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1])*w_ratio){
				sig =1.0;
				if ((gui->step_y[gui->step] - gui->step_y[gui->step - 1]) > -0.0)
					sig = (gui->step_x[gui->step] - gui->step_x[gui->step - 1] > -0.0)? 1.0: -1.0;
				y = gui->step_y[gui->step];
				x = gui->step_x[gui->step - 1] + sig * (gui->step_y[gui->step] - gui->step_y[gui->step - 1]) * w_ratio;
			}
			else{
				sig =1.0;
				if ((gui->step_x[gui->step] - gui->step_x[gui->step - 1]) > -0.0)
					sig = (gui->step_y[gui->step] - gui->step_y[gui->step - 1] > -0.0)? 1.0: -1.0;
				x = gui->step_x[gui->step];
				y = gui->step_y[gui->step - 1] + sig * (gui->step_x[gui->step] - gui->step_x[gui->step - 1]) * h_ratio;
			}
		}
		else{
			/* without proportion (probaly cause image distortion) */
			x = gui->step_x[gui->step];
			y = gui->step_y[gui->step];
		}
		
		/* draw the rectangle */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			line_add(graph, gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0,
				x, gui->step_y[gui->step - 1], 0);
			line_add(graph, x, gui->step_y[gui->step - 1], 0,
				x, y, 0);
			line_add(graph, x, y, 0,
				gui->step_x[gui->step - 1], y, 0);
			line_add(graph, gui->step_x[gui->step - 1], y, 0,
				gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
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
				gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, DWG_LIFE); /* paper space */
			
			/* draw entity */
			new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
			drawing_ent_append(gui->drawing, new_el);
			
			/* append to undo/redo list */
			do_add_entry(&gui->list_do, "IMAGE");
			do_add_item(gui->list_do.current, NULL, new_el);
			
			/* restart the proccess */
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
			gui_first_step(gui);
		}
	}
	
	return 1;
}

int gui_image_info (gui_obj *gui){
	if (gui->modal == IMAGE) {
		static char path[DXF_MAX_CHARS];
		static int path_len = 0;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place Raster Image", NK_TEXT_LEFT);
		nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		nk_edit_string(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, path, &path_len, DXF_MAX_CHARS, nk_filter_default);
		nk_checkbox_label(gui->ctx, "Proportional", &gui->proportional);
		
		if(gui->step == 0){
			if (nk_button_label(gui->ctx, "Attach")){
				path[path_len] = 0;
				int w, h, comp;
				if(stbi_info (path, &w, &h, &comp)){
					gui->image_w = w;
					gui->image_h = h;
					
					strncpy(gui->image_path, path, DXF_MAX_CHARS - 1);
					
					gui->step = 1;
				}
			}
		}
		else if(gui->step == 1){
			nk_label(gui->ctx, "Enter first corner", NK_TEXT_LEFT);
		}
		else if(gui->step == 2){
			nk_label(gui->ctx, "Enter last corner", NK_TEXT_LEFT);
		}
		
	}
	return 1;
}