#include "gui_use.h"

int gui_rect_interactive(gui_obj *gui){
	if (gui->modal == RECT){
		static dxf_node *new_el;
		
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				
				gui->draw_tmp = 1;
				int closed = 1;
				/* create a new DXF lwpolyline */
				new_el = (dxf_node *) dxf_new_lwpolyline (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1, */
					0.0,  /* bulge, */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE); /* paper space */
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_attr_change_i(new_el, 70, (void *) &closed, 0);
				gui->element = new_el;
				gui->step = 1;
				gui->en_distance = 1;
				goto next_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 1);
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 3);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "RECT");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				goto first_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto first_step;
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 1);
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 3);
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	goto end_step;
	default_modal:
		gui->modal = SELECT;
	first_step:
		gui->en_distance = 0;
		gui->draw_tmp = 0;
		gui->element = NULL;
		gui->draw = 1;
		gui->step = 0;
		gui->draw_phanton = 0;
		//if (gui->phanton){
		//	gui->phanton = NULL;
		//}
	next_step:
		
		gui->lock_ax_x = 0;
		gui->lock_ax_y = 0;
		gui->user_flag_x = 0;
		gui->user_flag_y = 0;

		gui->draw = 1;
	end_step:
		return 1;
}

int gui_rect_info (gui_obj *gui){
	if (gui->modal == RECT) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place a rectangle", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
		}
	}
	return 1;
}