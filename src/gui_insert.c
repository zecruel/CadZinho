#include "gui_use.h"

int gui_insert_interactive(gui_obj *gui){
	if (gui->modal == INSERT){
		static dxf_node *new_el;
		int i;
		
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				/* verify if block exist */					
				if (dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name)){
					gui->draw_tmp = 1;
					//dxf_new_insert (char *name, double x0, double y0, double z0,int color, char *layer, char *ltype, int paper);
					new_el = dxf_new_insert (gui->blk_name,
						gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1 */
						gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
						gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
						0); /* paper space */
					gui->element = new_el;
					gui->step = 1;
					gui->en_distance = 1;
					goto next_step;
				}
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				
				drawing_ent_append(gui->drawing, new_el);
				
				
				/*=========================*/
				dxf_node *blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name);
				dxf_node *attdef, *attrib;
				i = 0;
				while (attdef = dxf_find_obj_i(blk, "ATTDEF", i)){
					attrib = dxf_attrib_cpy(attdef, gui->step_x[gui->step], gui->step_y[gui->step], 0.0);
					ent_handle(gui->drawing, attrib);
					dxf_insert_append(gui->drawing, new_el, attrib);
					
					i++;
				}
				
				/*===================*/
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				do_add_entry(&gui->list_do, "INSERT");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				goto first_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
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
		if (gui->phanton){
			gui->phanton = NULL;
		}
	next_step:
		
		gui->lock_ax_x = 0;
		gui->lock_ax_y = 0;
		gui->user_flag_x = 0;
		gui->user_flag_y = 0;

		gui->draw = 1;
	end_step:
		return 1;
}

int gui_insert_info (gui_obj *gui){
	if (gui->modal == INSERT) {
		int i;
		static int show_blk_pp = 0, show_hidden_blks = 0;
		static char txt[DXF_MAX_CHARS] = "";
		
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place a block", NK_TEXT_LEFT);
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Block Name:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->blk_name, DXF_MAX_CHARS, nk_filter_default);
		
		if (nk_button_label(gui->ctx, "Explore")) show_blk_pp = 1;
		if (show_blk_pp){
			/* select block popup */
			static struct nk_rect s = {20, 10, 420, 330};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Select Block", NK_WINDOW_CLOSABLE, s)){
				
				list_node *blk_g; /*graphic object of current block */
				dxf_node *blk, *blk_nm; /* current block and its name attribute */
				int blk_ei; /*extents flag of current block */
				/* extents and zoom parameters */
				double blk_x0, blk_y0, blk_x1, blk_y1, z, z_x, z_y, o_x, o_y;
				
				nk_layout_row_dynamic(gui->ctx, 225, 2);
				i = 0;
				int blk_idx = -1;
				if (nk_group_begin(gui->ctx, "Block_names", NK_WINDOW_BORDER)) {
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					while (blk = dxf_find_obj_i(gui->drawing->blks, "BLOCK", i)){
						
						/* get name of current block */
						blk_nm = dxf_find_attr2(blk, 2);
						if (blk_nm){
							if((blk_nm->value.s_data[0] != '*') || (show_hidden_blks)){
								if (nk_button_label(gui->ctx, blk_nm->value.s_data)){
									blk_idx = i;
									strncpy(gui->blk_name, blk_nm->value.s_data, DXF_MAX_CHARS-1);
								}
							}
						}
						
						i++;
					}
					nk_group_end(gui->ctx);
				}
				if (blk_idx >= 0){
					blk = dxf_find_obj_i(gui->drawing->blks, "BLOCK", blk_idx);
					blk_idx = -1;
					
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
					graph_list_draw(blk_g, gui->preview_img, d_param);
					
				}
				if (nk_group_begin(gui->ctx, "Block_prev", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
					/* preview img */
					nk_layout_row_dynamic(gui->ctx, 175, 1);
					nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
					
					/* current block name */
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, gui->blk_name, NK_TEXT_CENTERED);
					
					/* current block corners */
					nk_layout_row_dynamic(gui->ctx, 15, 1);
					//nk_style_push_font(gui->ctx, &font_tiny_nk); /* change font to tiny*/
					nk_label(gui->ctx, txt, NK_TEXT_CENTERED);
					//nk_style_pop_font(gui->ctx); /* back to default font*/
					
					nk_group_end(gui->ctx);
				}
				
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				
				nk_checkbox_label(gui->ctx, "Hidden", &show_hidden_blks);
				
				if (nk_button_label(gui->ctx, "Select")){
					show_blk_pp = 0;
					nk_popup_close(gui->ctx);
				}
				/*
				if (nk_button_label(gui->ctx, "test")){
					//snprintf(txt, DXF_MAX_CHARS, "text");
					blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name);
					if(blk){
						/* create a new attdef text 
						//dxf_node * dxf_new_attdef (double x0, double y0, double z0, double h,
						//char *txt, char *tag, double thick, int color, char *layer, char *ltype, int paper){

						dxf_node * new_el = dxf_new_attdef (
							0.0, 0.0, 0.0, 1.0, /* pt1, height 
							"test", "tag1",(double) thick, /* text, thickness 
							gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer 
							gui->drawing->ltypes[gui->ltypes_idx].name, 0); /* line type, paper space 
						ent_handle(gui->drawing, new_el);
						dxf_block_append(blk, new_el);
						snprintf(txt, DXF_MAX_CHARS, "attdef");
					}
				}*/
				
				nk_popup_end(gui->ctx);
			}
			else show_blk_pp = 0;
		}
	}
	return 1;
}