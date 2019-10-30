#include "gui_use.h"

int gui_select_interactive(gui_obj *gui){
	if (gui->modal != SELECT) return 0;
	
	if (gui->sel_type == SEL_ELEMENT){
		if (gui->ev & EV_ENTER){
			if (gui->element)
				list_modify(gui->sel_list, gui->element, gui->sel_mode, 0);
			
		}
		if (gui->ev & EV_CANCEL){
			gui->sel_idx++;
			
			if (gui->sel_idx >= gui->near_count)
				gui->sel_idx = 0;
			
			gui->element = NULL;
			list_node *list_el = NULL;
			if (gui->near_count > 0) {
				int i = 0;
				list_el = gui->near_list->next;
				while (list_el){
					if (gui->sel_idx == i)
						gui->element = (dxf_node *)list_el->data;
					list_el = list_el->next;
					i++;
				}
			}
			else {
				list_clear(gui->sel_list);
				gui->draw = 1;
			}
			gui->draw = 1;
		}
		if (gui->ev & EV_MOTION){
			if (gui->sel_idx >= gui->near_count)
				gui->sel_idx = 0;
			
			gui->element = NULL;
			list_node *list_el = NULL;
			if (gui->near_count > 0) {
				int i = 0;
				list_el = gui->near_list->next;
				while (list_el){
					if (gui->sel_idx == i)
						gui->element = (dxf_node *)list_el->data;
					list_el = list_el->next;
					i++;
				}
			}
			
			/* for hilite test */
			//gui->element = gui->near_el;
			gui->draw = 1;
		}
	}
	else if (gui->sel_type == SEL_RECTANGLE){
		static dxf_node *new_el;
		
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				
				gui->draw_tmp = 1;
				
				gui->step = 1;
				gui->en_distance = 1;
				gui_next_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				list_clear(gui->sel_list);
				gui->draw = 1;
				
				gui_default_modal(gui);
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				double rect_pt1[2], rect_pt2[2];
				rect_pt1[0] = (gui->step_x[gui->step - 1] < gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
				rect_pt1[1] = (gui->step_y[gui->step - 1] < gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
				rect_pt2[0] = (gui->step_x[gui->step - 1] > gui->step_x[gui->step]) ? gui->step_x[gui->step - 1] : gui->step_x[gui->step];
				rect_pt2[1] = (gui->step_y[gui->step - 1] > gui->step_y[gui->step]) ? gui->step_y[gui->step - 1] : gui->step_y[gui->step];
				
				list_node *list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				int count = 0;
				
				if (gui->step_x[gui->step - 1] > gui->step_x[gui->step])
					count = dxf_ents_isect2(list, gui->drawing, rect_pt1, rect_pt2);
				else count = dxf_ents_in_rect(list, gui->drawing, rect_pt1, rect_pt2);
				
				list_node *list_el = NULL;
				if (count > 0) {
					list_el = list->next;
					while (list_el){
						list_modify(gui->sel_list, (dxf_node *)list_el->data, gui->sel_mode, 0);
						list_el = list_el->next;
					}
				}
				
				gui_first_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
			}
			if (gui->ev & EV_MOTION){
				int closed = 1;
				/* create a new DXF lwpolyline */
				new_el = (dxf_node *) dxf_new_lwpolyline (
					gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 0.0, /* pt1, */
					0.0,  /* bulge, */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, FRAME_LIFE); /* paper space */
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step - 1], 0.0, 0.0, FRAME_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, FRAME_LIFE);
				dxf_lwpoly_append (new_el, gui->step_x[gui->step - 1], gui->step_y[gui->step], 0.0, 0.0, FRAME_LIFE);
				dxf_attr_change_i(new_el, 70, (void *) &closed, 0);
				
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
				gui->element = new_el;
			}
		}
	}
	return 1;
}

int gui_select_info (gui_obj *gui){
	if (gui->modal == SELECT) {
		static int prev_type = -1;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Select an object", NK_TEXT_LEFT);
		
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, "Toggle", gui->sel_mode == LIST_TOGGLE)) gui->sel_mode = LIST_TOGGLE;
		if (gui_tab (gui, "+", gui->sel_mode == LIST_ADD)) gui->sel_mode = LIST_ADD;
		if (gui_tab (gui, "-", gui->sel_mode == LIST_SUB)) gui->sel_mode = LIST_SUB;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, "Element", gui->sel_type == SEL_ELEMENT)) gui->sel_type = SEL_ELEMENT;
		if (gui_tab (gui, "Rectangle", gui->sel_type == SEL_RECTANGLE)) gui->sel_type = SEL_RECTANGLE;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		if (prev_type != gui->sel_type){
			prev_type = gui->sel_type;
			gui->step = 0;
		}
	}
	return 1;
}


/*--------------------------------------------- 
dxf_node *current, *start, *end;
if(dxf_find_ext_appid(gui->element, "ZECRUEL", &start, &end)){
	printf("ext data Zecruel, start = %d, end = %d\n", start, end);
	current = start;
	while (current != NULL){
		printf ("%d = ", current->value.group); 
		
		switch (current->value.t_data) {
			case DXF_STR:
				if(current->value.s_data){
					printf(current->value.s_data);
				}
				break;
			case DXF_FLOAT:
				printf("%f", current->value.d_data);
				break;
			case DXF_INT:
				printf("%d", current->value.i_data);
		}
		printf("\n");
		//printf ("%x\n", current);
		if (current == end) break;
		current = current->next;
	}
}*/
/* -------------------------------test-------------- */