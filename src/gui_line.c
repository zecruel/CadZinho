#include "gui_use.h"

int gui_line_interactive(gui_obj *gui){
	if (gui->modal == LINE){
		static dxf_node *new_el;
		
		if (gui->step == 0){
			gui->free_sel = 0;
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* create a new DXF line */
				new_el = (dxf_node *) dxf_new_line (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1, pt2 */
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
				dxf_attr_change(new_el, 11, &gui->step_x[gui->step]);
				dxf_attr_change(new_el, 21, &gui->step_y[gui->step]);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "LINE");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				
				new_el = (dxf_node *) dxf_new_line (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1, pt2 */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE); /* paper space */
				
				gui->element = new_el;
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 11, &gui->step_x[gui->step]);
				dxf_attr_change(new_el, 21, &gui->step_y[gui->step]);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	return 1;
}

int gui_line_info (gui_obj *gui){
	if (gui->modal == LINE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Place a single line"), NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
		}
	}
	return 1;
}