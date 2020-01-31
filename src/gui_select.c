#include "gui_use.h"

int gui_select_interactive(gui_obj *gui){
	if (gui->modal != SELECT) return 0;
	
	gui->free_sel = 1;
	
	static int tmp_rect = 0;
	
	/* select single element */
	if ((gui->sel_type == SEL_ELEMENT || gui->sel_type == SEL_INTL) && !tmp_rect){
		if (gui->ev & EV_ENTER){ /* user hit an enter point */
			if (gui->element)
				/* select an object if cursor is over */
				list_modify(gui->sel_list, gui->element, gui->sel_mode, SEL_LIFE);
			else if (gui->sel_type == SEL_INTL){
				/* if not any hovered object, change type to start rectangle selection temporarily*/
				//gui->sel_type = SEL_RECTANGLE;
				tmp_rect = 1;
			}
		}
		if (gui->ev & EV_CANCEL){
			/* try to switch to another object that is overlaid */
			gui->sel_idx++;
			if (gui->sel_idx >= gui->near_count)
				gui->sel_idx = 0; /* back to list begin */
			
			gui->element = NULL;
			list_node *list_el = NULL;
			if (gui->near_count > 0) {
				/* examine the list of objects near cursor */
				int i = 0;
				list_el = gui->near_list->next;
				while (list_el){
					if (gui->sel_idx == i)
						/* show the candidate object */
						gui->element = (dxf_node *)list_el->data;
					list_el = list_el->next;
					i++;
				}
			}
			else {
				/* if not any hovered object, clear selection */
				sel_list_clear (gui);
				gui->draw = 1;
			}
			gui->draw = 1;
		}
		if (gui->ev & EV_MOTION){ /* cursor motion */
			if (gui->sel_idx >= gui->near_count)
				gui->sel_idx = 0;
			
			/* examine the list of objects near cursor */
			gui->element = NULL;
			list_node *list_el = NULL;
			if (gui->near_count > 0) {
				int i = 0;
				list_el = gui->near_list->next;
				while (list_el){
					if (gui->sel_idx == i)
						/* show the candidate object */
						gui->element = (dxf_node *)list_el->data;
					list_el = list_el->next;
					i++;
				}
			}
			gui->draw = 1;
		}
	}
	/* rectangle selection */
	if (gui->sel_type == SEL_RECTANGLE || tmp_rect){
		//static dxf_node *new_el;
		
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
				/* clear selection */
				sel_list_clear (gui);
				gui->draw = 1;
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
				
				/* list of objects to select */
				list_node *list = list_new(NULL, FRAME_LIFE);
				list_clear(list);
				int count = 0;
				
				if (gui->step_x[gui->step - 1] > gui->step_x[gui->step])
					/* if rectangle is back ward (right to left), then get inside objects 
					and also all intersecting to rectangle */
					count = dxf_ents_isect2(list, gui->drawing, rect_pt1, rect_pt2);
				/* otherside, only all inside objects */
				else count = dxf_ents_in_rect(list, gui->drawing, rect_pt1, rect_pt2);
				
				/* add to main selection list */
				list_node *list_el = NULL;
				if (count > 0) {
					list_el = list->next;
					while (list_el){
						list_modify(gui->sel_list, (dxf_node *)list_el->data, gui->sel_mode, SEL_LIFE);
						list_el = list_el->next;
					}
				}
				gui->draw_phanton = 0;
				gui_first_step(gui);
				if (tmp_rect) {
					/* return to single element selection, if entered temporarily*/
					//gui->sel_type = SEL_ELEMENT;
					tmp_rect = 0;
				}
			}
			else if (gui->ev & EV_CANCEL){
				/* cancel rectangle selection */
				gui->draw_phanton = 0;
				gui_first_step(gui);
				if (tmp_rect) {
					/* return to single element selection, if entered temporarily*/
					//gui->sel_type = SEL_ELEMENT;
					tmp_rect = 0;
				}
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
	}
	return 1;
}

int gui_select_info (gui_obj *gui){
	if (gui->modal == SELECT) {
		//static int prev_type = -1;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Select objects", NK_TEXT_LEFT);
		nk_label(gui->ctx, "Mode:", NK_TEXT_LEFT);
		
		/* Selection mode option - toggle, add or remove */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, "Toggle", gui->sel_mode == LIST_TOGGLE)) gui->sel_mode = LIST_TOGGLE;
		if (gui_tab (gui, "+", gui->sel_mode == LIST_ADD)) gui->sel_mode = LIST_ADD;
		if (gui_tab (gui, "-", gui->sel_mode == LIST_SUB)) gui->sel_mode = LIST_SUB;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Type:", NK_TEXT_LEFT);
		
		/* Selection type option - single element or by rectangle */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 30, 4);
		if (gui_tab_img (gui, gui->svg_bmp[SVG_I_SEL],
			gui->sel_type == SEL_INTL, 30))
			gui->sel_type = SEL_INTL;
		if (gui_tab_img (gui, gui->svg_bmp[SVG_SINGLE_SEL],
			gui->sel_type == SEL_ELEMENT, 30))
			gui->sel_type = SEL_ELEMENT;
		if (gui_tab_img (gui, gui->svg_bmp[SVG_RECT_SEL],
			gui->sel_type == SEL_RECTANGLE, 30))
			gui->sel_type = SEL_RECTANGLE;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		/*
		if (prev_type != gui->sel_type){
			prev_type = gui->sel_type;
			gui->step = 0;
		}*/
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