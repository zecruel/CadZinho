#include "gui_use.h"

int gui_point_info (gui_obj *gui){
	if (gui->modal != SINGLE_POINT) return 0;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a single point"), NK_TEXT_LEFT);
  nk_label(gui->ctx, _l("Enter destination point"), NK_TEXT_LEFT);
	
	
	dxf_node *new_el;
	gui->free_sel = 0;
	gui->draw_tmp = 1;
	/* temp entity */
	new_el = (dxf_node *) dxf_new_point (
		gui->step_x[gui->step], gui->step_y[gui->step], 0.0,
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, FRAME_LIFE); /* paper space */
	new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , FRAME_LIFE);
	gui->element = new_el;
	
	
	if (gui->ev & EV_ENTER){
		gui->draw_tmp = 1;
		/* create a new DXF point */
		new_el = (dxf_node *) dxf_new_point (
			gui->step_x[gui->step], gui->step_y[gui->step], 0.0,
			gui->color_idx, /* color, layer */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
			/* line type, line weight */
      (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
      dxf_lw[gui->lw_idx],
			0, DWG_LIFE); /* paper space */
		new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , DWG_LIFE);
		
		drawing_ent_append(gui->drawing, new_el);
		
		do_add_entry(&gui->list_do, _l("POINT"));
		do_add_item(gui->list_do.current, NULL, new_el);
	}
	else if (gui->ev & EV_CANCEL){
		gui_default_modal(gui);
	}
	
	return 1;
}