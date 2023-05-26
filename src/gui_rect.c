#include "gui_use.h"

int gui_rect_interactive(gui_obj *gui){
	if (gui->modal == RECT){
		static dxf_node *new_el;
		
		if (gui->step == 0){
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				
				gui->draw_tmp = 1;
				int closed = 1;
				/* create a new DXF lwpolyline */
				new_el = (dxf_node *) dxf_new_lwpolyline (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1, */
					0.0,  /* bulge, */
					gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
					/* line type, line weight */
          (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
          dxf_lw[gui->lw_idx],
					0, DWG_LIFE); /* paper space */
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, DWG_LIFE);
				dxf_attr_change_i(new_el, 70, (void *) &closed, 0);
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
				
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 1);
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 3);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, _l("RECT"));
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui_first_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 1);
				dxf_attr_change_i(new_el, 10, (void *) &gui->step_x[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 2);
				dxf_attr_change_i(new_el, 20, (void *) &gui->step_y[gui->step], 3);
				dxf_attr_change(new_el, 6,
          (void *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name));
				dxf_attr_change(new_el, 8,
          (void *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name));
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	return 1;
}

int gui_rect_info (gui_obj *gui){
	if (gui->modal == RECT) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Place a rectangle"), NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, _l("Enter first point"), NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, _l("Enter end point"), NK_TEXT_LEFT);
		}
	}
	return 1;
}