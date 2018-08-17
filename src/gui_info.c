#include "gui_info.h"

void nk_dxf_ent_info (struct nk_context *ctx, dxf_node *ent, int id){ /* print the entity structure */
	/* use Nuklear widgets to show a DXF entity structure */
	/* this function is non recursive */
	
	int i = 0;
	int level = 0;
	dxf_node *current, *prev;
	int tree_st[10]; /* max of ten levels of entities inside entities*/
	char text[401];
	
	text[0] = 0;
	id *= 1000; /* each Tree widget has a unique id, up to 1000 inside main entity*/
	
	current = ent;
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/* DXF entities are show as Tree widget */
			if (current->obj.name){
				id++; /* increment id for child Trees */
				if (level == 0){ /* verify if is the first Tree */
					if (tree_st[level] = nk_tree_push_id(ctx, NK_TREE_TAB, current->obj.name, NK_MINIMIZED, id)) {
						/* if Tree is not minimized, start the placement of child widgets */
						nk_layout_row_dynamic(ctx, 13, 1);
						nk_label(ctx, "-----", NK_TEXT_LEFT);
					}
				}
				else if (tree_st[level - 1]){ /* verify if the up level Tree is not minimized */
					if (tree_st[level] = nk_tree_push_id(ctx, NK_TREE_TAB, current->obj.name, NK_MINIMIZED, id)) {
						/* if Tree is not minimized, start the placement of child widgets */
						nk_layout_row_dynamic(ctx, 13, 1);
						nk_label(ctx, "-----", NK_TEXT_LEFT);
					}
				}
				else{
					tree_st[level] = 0;
				}
			}
			if (current->obj.content){
				/* starts entity content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				level++;
			}
		}
		else if (current->type == DXF_ATTR){
			/* DXF attributes are show as Label widget */
			/* combine Group and Value, acording its type, in a string */
			i = snprintf (text, 400, "%d = ", current->value.group);
			switch (current->value.t_data) {
				case DXF_STR:
					if(current->value.s_data){
						i += snprintf (text + i, 400 - i, current->value.s_data);
					}
					break;
				case DXF_FLOAT:
					i += snprintf (text + i, 400 - i, "%f", current->value.d_data);
					break;
				case DXF_INT:
					i += snprintf (text + i, 400 - i, "%d", current->value.i_data);
			}
			if (tree_st[level - 1]){ /* verify if the up level Tree is not minimized */
				nk_label(ctx, text, NK_TEXT_LEFT);
			}
			/* clear string */
			i = 0; text[0] = 0;
			
			current = current->next; /* go to the next in the list */
		}
		/* if the contents sweep reachs to end, try to up in structure */
		while (current == NULL){ 
			if (prev == ent){ /* back to original entity */
				/* ends the top level Tree, if not minimized */
				if (tree_st[level - 1]){
					nk_tree_pop(ctx);
				}
				current = NULL; /* ends loop */
				break;
			}
			
			prev = prev->master;
			if (prev){
				current = prev->next; /* continue loop */
				level --; /* up in structure*/
				/* ends the current Tree, if not minimized */
				if (tree_st[level]){
					nk_tree_pop(ctx);
				}
				/* back to original entity */
				if (prev == ent){
					current = NULL; /* ends loop */
					break;
				}
			}
			else{ /* if structure ends */
				current = NULL; /* ends loop */
				break;
			}
		}
		
		if ((level < 0) || (level > 9)){/* verify if level is out of admissible range */
			current = NULL; /* ends loop */
			break;
		}
	}
}

int info_win (gui_obj *gui){
	int show_info = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 200;
	gui->next_win_h = 300;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Info", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, "Info", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "BLK:", NK_TEXT_LEFT);
		i = 1;
		nk_dxf_ent_info (gui->ctx, gui->drawing->blks, i);
		
		nk_label(gui->ctx, "ENTS:", NK_TEXT_LEFT);
		if (gui->sel_list != NULL){				
			list_node *current = gui->sel_list->next;
			// starts the content sweep 
			i = 2;
			while (current != NULL){
				if (current->data){
					if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							
						// -------------------------------------------
						nk_dxf_ent_info (gui->ctx, (dxf_node *)current->data, i);
						i++;
						
						//---------------------------------------
					}
				}
				current = current->next;
			}
		}
		//nk_popup_end(gui->ctx);
	} else show_info = 0;
	nk_end(gui->ctx);
	
	return show_info;
}