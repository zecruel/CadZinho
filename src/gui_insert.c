#include "gui_use.h"

int gui_insert_interactive(gui_obj *gui){
	if (gui->modal == INSERT){
		static dxf_node *new_el;
		int i;
		
		
		if (gui->step == 0 && (gui->ev & EV_CANCEL)){
			/* cancel insert mode */
			gui_default_modal(gui);
		}
		else if (gui->step == 1){
			gui->free_sel = 0;
			/* verify if block exist */					
			if (dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name)){
				gui->draw_tmp = 1;
				/* create a candidate insert element */
				new_el = dxf_new_insert (gui->blk_name,
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1 */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE);
				/* go to next step */
				gui->element = new_el;
				gui->step = 2;
				gui->en_distance = 1;
				gui_next_step(gui);
			}
		}
		else if (gui->step == 2){
			if (gui->ev & EV_ENTER){ /* confirm */
				/* insertion point */
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				
				/* convert block's ATTDEF to ATTRIBUTES*/
				dxf_node *blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name);
				dxf_node *attdef, *attrib;
				i = 0;
				/* get attdef */
				while (attdef = dxf_find_obj_i(blk, "ATTDEF", i)){
					/* convert and append to insert with translate, rotation and scale */
					attrib = dxf_attrib_cpy(attdef, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->scale_x, gui->angle, DWG_LIFE);
					ent_handle(gui->drawing, attrib);
					dxf_insert_append(gui->drawing, new_el, attrib, DWG_LIFE);
					
					i++;
				}
				
				/* append to drawing */
				drawing_ent_append(gui->drawing, new_el);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , DWG_LIFE);
				do_add_entry(&gui->list_do, "INSERT");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				/* prepare to new insert */
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				gui_first_step(gui);
				gui->step = 1;
				new_el = NULL;
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
				gui->step = 0;
			}
			if (gui->ev & EV_MOTION){
				/* Modify insert parameters */
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				dxf_attr_change(new_el, 41, &gui->scale_x);
				dxf_attr_change(new_el, 42, &gui->scale_y);
				dxf_attr_change(new_el, 43, &gui->scale_x);//
				double angle = gui->angle;
				if (angle <= 0.0) angle = 360.0 - angle;
				angle = fmod(angle, 360.0);
				dxf_attr_change(new_el, 50, &angle);
				/* generate current graph to show */
				if (new_el) new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , FRAME_LIFE);
			}
		}
	}
	return 1;
}

int gui_insert_info (gui_obj *gui){
	if (gui->modal == INSERT) {
		int i;
		static int show_blk_pp = 0, show_hidden_blks = 0;
		static char txt[DXF_MAX_CHARS+1] = "";
		static char descr[DXF_MAX_CHARS+1] = "";
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place a Insert", NK_TEXT_LEFT);
		
		if (gui->step == 0){ /* choose a block description */
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, "Choose Block:", NK_TEXT_LEFT);
			
			/* enter a block name directly */
			nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->blk_name, DXF_MAX_CHARS, nk_filter_default);
			
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			if (nk_button_label(gui->ctx, "OK")){ /* try go to next step */
				/* check if block exists */
				if (dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", gui->blk_name)){
					gui->step = 1;
					gui_next_step(gui);
				}
				else {
					snprintf(gui->log_msg, 63, "Error: Block not found");
				}
			}
			
			/* explore to view available blocks in drawing */
			if (nk_button_label(gui->ctx, "Explore")) show_blk_pp = 1;
		}
		else{ /* define parameters and place a insert */
			/* show refered block name */
			nk_layout_row_template_begin(gui->ctx, 20);
			nk_layout_row_template_push_static(gui->ctx, 50);
			nk_layout_row_template_push_dynamic(gui->ctx);
			nk_layout_row_template_end(gui->ctx);
			nk_label(gui->ctx, "Block:", NK_TEXT_RIGHT);
			nk_label_colored(gui->ctx, gui->blk_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
			
			/* dinamicaly, choose a place point and confirm */
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, "Enter place point", NK_TEXT_LEFT);
			
			/* scale and rotation parameters */
			gui->scale_x = nk_propertyd(gui->ctx, "Scale", 0.0d, gui->scale_x, 100000.0d, 0.1d, 0.1d);
			gui->scale_y = gui->scale_x; //gui->scale_z = gui->scale_x;
			gui->angle = nk_propertyd(gui->ctx, "Angle", -180.0d, gui->angle, 180.0d, 0.1d, 0.1d);
		}
		if (show_blk_pp){
			/* explore and select block popup */
			static struct nk_rect s = {20, -80, 420, 380};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Select Block", NK_WINDOW_CLOSABLE, s)){
				
				list_node *blk_g; /*graphic object of current block */
				dxf_node *blk, *blk_nm; /* current block and its name attribute */
				int blk_ei; /*extents flag of current block */
				/* extents and zoom parameters */
				double blk_x0, blk_y0, blk_x1, blk_y1, z, z_x, z_y, o_x, o_y;
				
				nk_layout_row_dynamic(gui->ctx, 280, 2);
				i = 0;
				int blk_idx = -1;
				/* list of available blocks */
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
				/* option to show hidden blocks */
				nk_checkbox_label(gui->ctx, "Hidden", &show_hidden_blks);
				
				if (nk_button_label(gui->ctx, "Select")){ /* select block and close popup */
					show_blk_pp = 0;
					nk_popup_close(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			}
			else show_blk_pp = 0;
		}
	}
	return 1;
}