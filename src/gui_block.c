#include "gui_use.h"

int block_use(dxf_drawing *drawing);
int block_rename(dxf_drawing *drawing, char *curr_name, char *new_name);

int gui_block_interactive(gui_obj *gui){
	if (gui->modal == NEW_BLK){
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				/* verify if text is valid */
				if (strlen(gui->blk_name) > 0){
					if(!gui->text2tag)
						dxf_new_block(gui->drawing, gui->blk_name, "0", gui->sel_list, &gui->list_do, DWG_LIFE);
					else dxf_new_block2(gui->drawing, gui->blk_name, gui->tag_mark, "0", gui->sel_list, &gui->list_do, DWG_LIFE);
				}
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
	}
	
	return 1;
}

int gui_block_info (gui_obj *gui){
	if (gui->modal == NEW_BLK) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Create a new block from selection", NK_TEXT_LEFT);
		
		nk_label(gui->ctx, "Block Name:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->blk_name, DXF_MAX_CHARS, nk_filter_default);
		nk_checkbox_label(gui->ctx, "Text to Tags", &gui->text2tag);
		nk_label(gui->ctx, "Tag mark:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->tag_mark, DXF_MAX_CHARS, nk_filter_default);
		
	}
	return 1;
}

/* block manager window */
int gui_blk_mng (gui_obj *gui){
	int i, show_blk_mng = 1;
	
	static int show_hidden_blks = 0, show_blk_name = 0;
	static char txt[DXF_MAX_CHARS+1] = "";
	static char descr[DXF_MAX_CHARS+1] = "";
	static char new_name[DXF_MAX_CHARS+1] = "";
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 550;
	gui->next_win_h = 400;
	
	if (nk_begin(gui->ctx, "Blocks Manager", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		struct nk_style_button *sel_type;
		
		list_node *blk_g; /*graphic object of current block */
		dxf_node *blk, *blk_nm; /* current block and its name attribute */
		int blk_ei; /*extents flag of current block */
		/* extents and zoom parameters */
		double blk_x0, blk_y0, blk_x1, blk_y1, z, z_x, z_y, o_x, o_y;
		
		block_use(gui->drawing); /* update blocks in use*/
		
		/* dynamic width for block list and fixed width for preview image */
		nk_layout_row_template_begin(gui->ctx, 280);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_push_static(gui->ctx, 190);
		nk_layout_row_template_end(gui->ctx);
		i = 0;
		int blk_idx = -1;
		
		/*  show block list */
		if (nk_group_begin(gui->ctx, "Block_list", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* header */
			nk_layout_row_dynamic(gui->ctx, 32, 1);
			if (nk_group_begin(gui->ctx, "Block_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				/* dynamic width for block name and fixed width for use mark */
				nk_layout_row_template_begin(gui->ctx, 22);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_push_static(gui->ctx, 50);
				nk_layout_row_template_push_static(gui->ctx, 8);
				nk_layout_row_template_end(gui->ctx);
				
				if (nk_button_label(gui->ctx, "Name")){
					
				}
				if (nk_button_label(gui->ctx, "Used")){
					
				}
				nk_group_end(gui->ctx);
			}
			/* list */
			nk_layout_row_dynamic(gui->ctx, 225, 1);
			if (nk_group_begin(gui->ctx, "Block_names", NK_WINDOW_BORDER)) {
				nk_layout_row_template_begin(gui->ctx, 20);
				nk_layout_row_template_push_dynamic(gui->ctx);
				nk_layout_row_template_push_static(gui->ctx, 50);
				nk_layout_row_template_end(gui->ctx);
				
				while (blk = dxf_find_obj_i(gui->drawing->blks, "BLOCK", i)){
				
					/* get name of current block */
					blk_nm = dxf_find_attr2(blk, 2);
					if (blk_nm){
						/* show block name, according hidden option */
						if((blk_nm->value.s_data[0] != '*') || (show_hidden_blks)){ /* hidden blocks name starts with '*' */
							
							/* verify if curren block is selected */
							sel_type = &gui->b_icon_unsel;
							if (strcmp(gui->blk_name, blk_nm->value.s_data) == 0) sel_type = &gui->b_icon_sel;
							/* block name */
							if (nk_button_label_styled(gui->ctx, sel_type, blk_nm->value.s_data)){
								/* select current block */
								blk_idx = i;
								strncpy(gui->blk_name, blk_nm->value.s_data, DXF_MAX_CHARS);
							}
							/* verify if block is used in drawing, by count in layer index*/
							if (blk->obj.layer > 0)
								nk_label(gui->ctx, "x", NK_TEXT_CENTERED);
							else nk_label(gui->ctx, " ", NK_TEXT_CENTERED);
						}
					}
					
					i++;
				}
				nk_group_end(gui->ctx);
			}
			nk_group_end(gui->ctx);
		}
		/* update the selected block information and its preview image */
		if (blk_idx >= 0){
			blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name);
			blk_idx = -1;
			
			/* get description of current block */
			blk_nm = dxf_find_attr2(blk, 1);
			if (blk_nm){
				strncpy(descr, blk_nm->value.s_data, DXF_MAX_CHARS);
				
			}
			
			blk_ei = 0;
			/* get graphics of current block*/
			blk_g = dxf_graph_parse(gui->drawing, blk, 0, FRAME_LIFE);
			/* get extents parameters of current block*/
			graph_list_ext(blk_g, &blk_ei, &blk_x0, &blk_y0, &blk_x1, &blk_y1);
			
			/* calcule the zoom and offset for preview */
			z_x = fabs(blk_x1 - blk_x0)/gui->preview_img->width;
			z_y = fabs(blk_y1 - blk_y0)/gui->preview_img->height;
			z = (z_x > z_y) ? z_x : z_y;
			if (z <= 0) z =1;
			else z = 1/(1.1 * z);
			o_x = blk_x0 - (fabs((blk_x1 - blk_x0)*z - gui->preview_img->width)/2)/z;
			o_y = blk_y0 - (fabs((blk_y1 - blk_y0)*z - gui->preview_img->height)/2)/z;
			
			/* show block extention */
			snprintf(txt, DXF_MAX_CHARS, "(%0.2f,%0.2f)-(%0.2f,%0.2f)", blk_x0, blk_y0, blk_x1, blk_y1);
			
			/* draw graphics in current preview bitmap */
			bmp_fill(gui->preview_img, gui->preview_img->bkg); /* clear bitmap */
			struct draw_param d_param;
			d_param.ofs_x = o_x;
			d_param.ofs_y = o_y;
			d_param.scale = z;
			d_param.list = NULL;
			d_param.subst = NULL;
			d_param.len_subst = 0;
			d_param.inc_thick = 0;
			graph_list_draw(blk_g, gui->preview_img, d_param);
			
		}
		/* Selected block detailed information and preview image */
		if (nk_group_begin(gui->ctx, "Block_prev", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* selected block name */
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label_colored(gui->ctx, gui->blk_name, NK_TEXT_CENTERED, nk_rgb(255,255,0));
			
			/* preview img */
			nk_layout_row_dynamic(gui->ctx, 175, 1);
			nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
			
			/* current block corners */
			nk_layout_row_dynamic(gui->ctx, 15, 1);
			nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny*/
			nk_label(gui->ctx, txt, NK_TEXT_CENTERED);
			nk_style_pop_font(gui->ctx); /* back to default font*/
			
			/* description */
			nk_layout_row_dynamic(gui->ctx, 50, 1);
			nk_label_colored_wrap(gui->ctx, descr, nk_rgb(100,115,255));
			
			nk_group_end(gui->ctx);
		}
		
		/* option to show hidden blocks */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_checkbox_label(gui->ctx, "Hidden", &show_hidden_blks);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		
		/* delete selected Block */
		if (nk_button_label(gui->ctx, "Remove")){
			block_use(gui->drawing); /* update blocks in use for sure*/
			
			/*get block object by selected name */
			blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name);
			if(blk) {
				/* don't remove block in use */
				if (blk->obj.layer){ /* uses block's layer index to count */
					snprintf(gui->log_msg, 63, "Error: Don't remove Block in use");
				}
				else{
					do_add_entry(&gui->list_do, "Remove Block");
					/* remove block from main structure */
					do_add_item(gui->list_do.current, blk, NULL);
					dxf_obj_subst(blk, NULL);
					
					/*get BLOCK_RECORD object by selected name */
					blk = dxf_find_obj_descr2(gui->drawing->blks_rec, "BLOCK_RECORD", gui->blk_name);
					if(blk) {
						/* remove block record from main structure */
						do_add_item(gui->list_do.current, blk, NULL);
						dxf_obj_subst(blk, NULL);
					}
				}
			}
		}
		/* rename selected block */
		if (nk_button_label(gui->ctx, "Rename")){
			/* verify if block point by name exists in structure */
			if(dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name)){
				/* show rename popup */
				show_blk_name = 1;
				new_name[0] = 0;
				strncpy(new_name, gui->blk_name, DXF_MAX_CHARS);
			}
		}
		
		if ((show_blk_name)){ /* rename popup interface */
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "New Block Name", NK_WINDOW_CLOSABLE, nk_rect(200, 40, 220, 100))){
				
				/* enter new name */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, new_name, DXF_MAX_CHARS, nk_filter_default);
				
				/* confirm */
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "Rename")){
					/* verify if exists other block with same name */
					if(dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", new_name)){
						snprintf(gui->log_msg, 63, "Error: exists Block with same name");
					}
					else{ /* proceed to rename */
						block_rename(gui->drawing, gui->blk_name, new_name);
						/* update informations */
						strncpy(gui->blk_name, new_name, DXF_MAX_CHARS);
						blk_idx = 1;
						
						show_blk_name = 0;
						nk_popup_close(gui->ctx);
					}
				}
				/* cancel - close popup */
				if (nk_button_label(gui->ctx, "Cancel")){
					show_blk_name = 0;
					nk_popup_close(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			} else {
				show_blk_name = 0;
			}
		}
		
	} else {
		show_blk_mng = 0;
	}
	nk_end(gui->ctx);
	
	
	return show_blk_mng;
}

int block_use(dxf_drawing *drawing){
	/* count blocks in use in drawing */
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[2];
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	/* init blocks count */
	current = drawing->blks;
	if (current) current = current->obj.content;
	while (current){ /* sweep elements in section */
		if (current->type == DXF_ENT){
			if (strcmp(current->obj.name, "BLOCK") == 0){
				/* uses block's layer index to count */
				current->obj.layer= 0;
				
				/* get name of current block */
				dxf_node * blk_nm = dxf_find_attr2(current, 2);
				if (blk_nm){
					char name[DXF_MAX_CHARS + 1];
					strncpy(name, blk_nm->value.s_data, DXF_MAX_CHARS);
					str_upp(name);
					/* mark used if is a system block*/
					if (strcmp(name, "*MODEL_SPACE") == 0) current->obj.layer= 1;
					else if (strcmp(name, "*PAPER_SPACE") == 0) current->obj.layer= 1;
				}
			}
		}
		current = current->next; /* go to the next in the list*/
	}
	
	for (i = 0; i< 2; i++){ /* look in BLOCKS and ENTITIES sections */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section */
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				if ((strcmp(current->obj.name, "INSERT") == 0) ||
					(strcmp(current->obj.name, "DIMENSION")) == 0){
					dxf_node *block = NULL, *blk_name = NULL;
					blk_name = dxf_find_attr2(current, 2);
					if(blk_name) {
						block = dxf_find_obj_descr2(drawing->blks, "BLOCK", blk_name->value.s_data);
						if(block) {
							/* uses block's layer index to count */
							block->obj.layer++;
						}
					}
				}
				/* search also in sub elements */
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content;
					continue;
				}
			}
				
			current = current->next; /* go to the next in the list*/
			
			if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
				current = NULL;
				break;
			}

			/* ============================================================= */
			while (current == NULL){
				/* end of list sweeping */
				if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
					//printf("para\n");
					current = NULL;
					break;
				}
				/* try to back in structure hierarchy */
				prev = prev->master;
				if (prev){ /* up in structure */
					/* try to continue on previous point in structure */
					current = prev->next;
					
				}
				else{ /* stop the search if structure ends */
					current = NULL;
					break;
				}
			}
		}
	}
	
	return ok;
}

int block_rename(dxf_drawing *drawing, char *curr_name, char *new_name){
	/* count drawing elements related to layer */
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[2];
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	if (!curr_name || !new_name) return 0;
	if (strlen(curr_name) == 0 || strlen (new_name) == 0) return 0;
	
	
	/* copy string for secure manipulation */
	char old_cpy[DXF_MAX_CHARS], *curr_name_cpy;
	char new_cpy[DXF_MAX_CHARS], *new_name_cpy;
	strncpy(old_cpy, curr_name, DXF_MAX_CHARS);
	strncpy(new_cpy, new_name, DXF_MAX_CHARS);
	/* remove trailing spaces */
	curr_name_cpy = trimwhitespace(old_cpy);
	new_name_cpy = trimwhitespace(new_cpy);
	
	
	/* first, rename main block object */
	current = dxf_find_obj_descr2(drawing->blks, "BLOCK", curr_name_cpy);
	if(current) {
		dxf_attr_change(current, 2, new_name_cpy);
	}
	else return 0;
	
	/* then, rename block_record object*/
	current = dxf_find_obj_descr2(drawing->blks_rec, "BLOCK_RECORD", curr_name_cpy);
	if(current) {
		dxf_attr_change(current, 2, new_name_cpy);
	}	
	
	/* change to upper case for  consistent comparison*/
	char curr_upp[DXF_MAX_CHARS];
	strncpy(curr_upp, curr_name_cpy, DXF_MAX_CHARS);
	str_upp(curr_upp);
	
	for (i = 0; i< 2; i++){ /* look in BLOCKS and ENTITIES sections */
		obj = list[i];
		current = obj;
		while (current){ /* sweep elements in section */
			ok = 1;
			prev = current;
			if (current->type == DXF_ENT){
				if ((strcmp(current->obj.name, "INSERT") == 0) ||
					(strcmp(current->obj.name, "DIMENSION")) == 0){
					dxf_node *block = NULL, *blk_name = NULL;
					blk_name = dxf_find_attr2(current, 2);
					if(blk_name) {
						/* change to upper case for  consistent comparison*/
						char name_upp[DXF_MAX_CHARS];
						strncpy(name_upp, blk_name->value.s_data, DXF_MAX_CHARS);
						str_upp(name_upp);
						
						/* verify if is a looking name*/
						if (strcmp(name_upp, curr_upp) == 0){
							/* change to new name */
							blk_name->value.s_data[0] = 0;
							strncpy(blk_name->value.s_data, new_name_cpy, DXF_MAX_CHARS);
						}
					}
				}
				/* search also in sub elements */
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content;
					continue;
				}
			}
				
			current = current->next; /* go to the next in the list*/
			
			if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
				current = NULL;
				break;
			}

			/* ============================================================= */
			while (current == NULL){
				/* end of list sweeping */
				if ((prev == NULL) || (prev == obj)){ /* stop the search if back on initial entity */
					//printf("para\n");
					current = NULL;
					break;
				}
				/* try to back in structure hierarchy */
				prev = prev->master;
				if (prev){ /* up in structure */
					/* try to continue on previous point in structure */
					current = prev->next;
					
				}
				else{ /* stop the search if structure ends */
					current = NULL;
					break;
				}
			}
		}
	}
	
	return ok;
}