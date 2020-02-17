#include "gui_use.h"

int block_use(dxf_drawing *drawing);

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
	
	static int show_hidden_blks = 0;
	static char txt[DXF_MAX_CHARS+1] = "";
	static char descr[DXF_MAX_CHARS+1] = "";
	
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
		if (nk_group_begin(gui->ctx, "Block_list", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(gui->ctx, 32, 1);
			if (nk_group_begin(gui->ctx, "Block_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
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
						if((blk_nm->value.s_data[0] != '*') || (show_hidden_blks)){
							
							sel_type = &gui->b_icon_unsel;
							if (strcmp(gui->blk_name, blk_nm->value.s_data) == 0) sel_type = &gui->b_icon_sel;
							
							if (nk_button_label_styled(gui->ctx, sel_type, blk_nm->value.s_data)){
								blk_idx = i;
								strncpy(gui->blk_name, blk_nm->value.s_data, DXF_MAX_CHARS-1);
							}
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
		if (blk_idx >= 0){
			blk = dxf_find_obj_i(gui->drawing->blks, "BLOCK", blk_idx);
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
			
			snprintf(txt, DXF_MAX_CHARS, "(%0.2f,%0.2f)-(%0.2f,%0.2f)", blk_x0, blk_y0, blk_x1, blk_y1);
			
			/* draw graphics in current preview bitmap */
			//gui->preview_img->zero_tl = 1;
			bmp_fill(gui->preview_img, gui->preview_img->bkg); /* clear bitmap */
			//graph_list_draw(blk_g, gui->preview_img, o_x, o_y, z);
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
		if (nk_group_begin(gui->ctx, "Block_prev", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* current block name */
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
						/* remove block from main structure */
						do_add_item(gui->list_do.current, blk, NULL);
						dxf_obj_subst(blk, NULL);
					}
				}
			}
			
			
		}
		
		
		
	} else {
		show_blk_mng = 0;
	}
	nk_end(gui->ctx);
	
	
	return show_blk_mng;
}

int block_use(dxf_drawing *drawing){
	/* count drawing elements related to layer */
	int ok = 0, i;
	dxf_node *current, *prev, *obj = NULL, *list[2];
	
	list[0] = NULL; list[1] = NULL;
	if (drawing){
		list[0] = drawing->ents;
		list[1] = drawing->blks;
	}
	else return 0;
	
	/* zero blocks count */
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