#include "gui_use.h"

void sel_list_append(list_node *list, dxf_node *ent){
	if (list && ent){
		if (list_find_data(list, ent)){
			//printf ("ja existe!\n");
		}
		else{
			list_node * new_el = list_new(ent, 0);
			if (new_el){
				list_push(list, new_el);
			}
		}
	}
}

int gui_select_interactive(gui_obj *gui){
	if (gui->modal == SELECT){
		if (gui->ev & EV_ENTER){
			sel_list_append(gui->sel_list, gui->element);
			/* -------------------------------test-------------- */
			
			/*dxf_edit_move (gui->element, 0.0 , 0.0, 0.0);
			
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
			
			//dxf_ent_attract (dxf_node * obj, enum attract_type type, double pos_x, double pos_y, double sensi, double *ret_x, double *ret_y)
			if (gui->element){
				if(gui->element->type == DXF_ENT){
					//dxf_ent_print2(gui->element);
					/*
					printf("%s\n",gui->element->obj.name);
					if (dxf_ident_ent_type (gui->element)  ==  DXF_INSERT){
						dxf_node *volta = dxf_find_attr2(gui->element, 2);
						if (volta){
							printf("%s\n",volta->value.s_data);
						}
					}*/
				}
			}
			//double ret_x, ret_y;
			
			/*---------------------------------------------  */
		}
		if (gui->ev & EV_CANCEL){
			list_clear(gui->sel_list);
			gui->draw = 1;
		}
		if (gui->ev & EV_MOTION){				
			/* for hilite test */
			gui->element = gui->near_el;
			gui->draw = 1;
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

int gui_select_info (gui_obj *gui){
	if (gui->modal == SELECT) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Select an object", NK_TEXT_LEFT);
	}
	return 1;
}