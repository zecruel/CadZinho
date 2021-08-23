#include "gui_use.h"

int gui_mtext_interactive(gui_obj *gui){
	if (gui->modal == MTEXT){
		static dxf_node *new_el;
		char *blank = "";
		int attch_pt = (2 - gui->t_al_v) * 3 + gui->t_al_h +1;
		if (gui->step == 0){
			gui->free_sel = 1;
			gui->draw_tmp = 1;
			/* create a new DXF text */
			//dxf_node * dxf_new_mtext (double x0, double y0, double z0, double h,char *txt[], int num_txt, int color, char *layer, char *ltype, int lw, int paper)
			char *teste[] = {""};
			
			new_el = (dxf_node *) dxf_new_mtext (
				gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->txt_h, /* pt1, height */
				teste, 1, /* text, */
				gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
				gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
				0, DWG_LIFE); /* paper space */
			gui->element = new_el;
			//dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
			//dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
			dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
			dxf_attr_change(new_el, 71, &attch_pt);
			gui->step = 1;
			gui_next_step(gui);
		}
		else{
			if (gui->ev & EV_ENTER){
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				//dxf_attr_change_i(new_el, 11, &gui->step_x[gui->step], -1);
				//dxf_attr_change_i(new_el, 21, &gui->step_y[gui->step], -1);
				dxf_attr_change(new_el, 40, &gui->txt_h);
				dxf_attr_change(new_el, 41, &gui->rect_w);
				dxf_attr_change(new_el, 71, &attch_pt);
				//dxf_attr_change(new_el, 1, gui->txt);
				//dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
				//dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
				/*
				printf("before = %d\n", dxf_count_attr(new_el, 3));
				
				dxf_node *final_str = dxf_find_attr2(new_el, 1);
				dxf_attr_insert_before(final_str, 3, (void *)"ins");
				
				printf("after = %d\n", dxf_count_attr(new_el, 3));
				*/
				
				char *text = nk_str_get(&(gui->text_edit.string));
				int len = nk_str_len_char(&(gui->text_edit.string));
				if (!text) text = blank;
				mtext_change_text (new_el, text, len, DWG_LIFE);
				
				dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "MTEXT");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				gui_first_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				//dxf_attr_change_i(new_el, 11, &gui->step_x[gui->step], -1);
				//dxf_attr_change_i(new_el, 21, &gui->step_y[gui->step], -1);
				dxf_attr_change(new_el, 40, &gui->txt_h);
				dxf_attr_change(new_el, 41, &gui->rect_w);
				dxf_attr_change(new_el, 71, &attch_pt);
				//dxf_attr_change(new_el, 1, gui->txt);
				
				
				char *text = nk_str_get(&(gui->text_edit.string));
				int len = nk_str_len_char(&(gui->text_edit.string));
				if (!text) text = blank;
				mtext_change_text (new_el, text, len, DWG_LIFE);
				
				
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				//dxf_attr_change_i(new_el, 72, &gui->t_al_h, -1);
				//dxf_attr_change_i(new_el, 73, &gui->t_al_v, -1);
				dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	return 1;
}

int gui_mtext_info (gui_obj *gui){
	if (gui->modal == MTEXT) {
		static int mtext_pop = 0;
		int num_tstyles = gui->drawing->num_tstyles;
		dxf_tstyle *t_sty = gui->drawing->text_styles;
		
		if (gui->t_sty_idx >= num_tstyles) gui->t_sty_idx = 0;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place an inteli text", NK_TEXT_LEFT);
			
		nk_label(gui->ctx, "Style:", NK_TEXT_LEFT);
		
		int h = num_tstyles * 25 + 5;
		h = (h < 200)? h : 200;
		if (nk_combo_begin_label(gui->ctx,  t_sty[gui->t_sty_idx].name, nk_vec2(220, h))){
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			int j = 0;
			for (j = 0; j < num_tstyles; j++){
				
				if (nk_button_label(gui->ctx, t_sty[j].name)){
					gui->t_sty_idx = j; /* select current style */
					
					nk_combo_close(gui->ctx);
					break;
				}
			}
			nk_combo_end(gui->ctx);
		}
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		
		nk_label(gui->ctx, "Text:", NK_TEXT_LEFT);
		
		if (nk_button_label(gui->ctx,  "Edit")){
			mtext_pop = 1;
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		gui->txt_h = nk_propertyd(gui->ctx, "Text Height", 1e-9, gui->txt_h, 1e9, SMART_STEP(gui->txt_h), SMART_STEP(gui->txt_h));
		gui->rect_w = nk_propertyd(gui->ctx, "Rect width", 0.0, gui->rect_w, 1e9, SMART_STEP(gui->rect_w), SMART_STEP(gui->rect_w));
		
		if (gui->t_al_v > 2) gui->t_al_v = 0;
		if (gui->t_al_h > 2) gui->t_al_h = 0;
		
		gui->t_al_v = nk_combo(gui->ctx, text_al_v + 1, 3, gui->t_al_v, 20, nk_vec2(100, 80));
		gui->t_al_h = nk_combo(gui->ctx, text_al_h, 3, gui->t_al_h, 20, nk_vec2(100, 80));
		
		if (mtext_pop){
			
			static struct nk_rect s = {150, 10, 420, 330};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Edit inteli text", NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE, s)){
				#if(0)
				/* wrap text in text edit widget */
				char *text = nk_str_get(&(gui->text_edit.string));
				
				double w = 0, line_w = 0;
				if (text){
					int i = 0, last_spc = 0;
					int ofs, code_p;
					
					while (i < nk_str_len_char(&(gui->text_edit.string))){ /* sweep the string */
						
						ofs = utf8_to_codepoint(text + i, &code_p);
						
						/* find the good point for break a line (in space caracter) */
						if ((code_p == ' ') || (code_p == '\t')){
							last_spc = i;
							ofs = 1;
						}
						/* convert line break to space temporaly, until find a good point to break */
						else if (code_p == '\v'){
							//text[i] = ' ';
							//last_spc = i;
							nk_str_delete_chars(&(gui->text_edit.string), i, 1);
							if (gui->text_edit.cursor >= i) gui->text_edit.cursor--;
							continue;
						}
						/* consider a \n caracter as a paragraph break */
						else if (code_p == '\n'){
							/* reset the line parameters */
							last_spc = 0;
							line_w = 0;
							ofs = 1;
						}
						
						/* get graphical width of current glyph */
						font_parse_cp((struct tfont *)gui->ui_font.userdata.ptr, code_p, 0, FRAME_LIFE, &w);
						/* update width of current line */
						line_w += w * gui->ui_font.height;
						
						if (line_w > 370){ /* verify  if current line width exceeds the drawing area */
							/* consider a tolerance of two glyphs to avoid repetitive breaks */
							int tolerance = 0;
							char *near_line = strpbrk(text + i, "\v\n");
							if (near_line) tolerance = near_line - text - i; /* tolerance until the next break */
							else tolerance = nk_str_len_char(&(gui->text_edit.string)) - i; /* tolerance until the string end */
							if (tolerance > 3){ /* need to break */
								if (last_spc){ /* if has a good point for break, use it */
									//text[last_spc] = '\v';
									nk_str_insert_text_char(&(gui->text_edit.string), last_spc + 1, "\v", 1);
									if (gui->text_edit.cursor >= last_spc) gui->text_edit.cursor++;
									/* start the current line in break point */
									i = last_spc + 1;
									ofs = 1;
									last_spc = 0;
									line_w = 0;
								}
								else{
									nk_str_insert_text_char(&(gui->text_edit.string), i, "\v", 1);
									if (gui->text_edit.cursor >= i) gui->text_edit.cursor++;
									line_w = 0;
								}
							}
						}
						
						i += ofs;
						text = nk_str_get(&(gui->text_edit.string));
					}
				}
				
				#endif
				nk_layout_row_dynamic(gui->ctx, 270, 1);
				
				//nk_edit_buffer(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
				nk_edit_buffer_wrap(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
				
				nk_popup_end(gui->ctx);
			}
			else mtext_pop = 0;
		}
	}
	return 1;
}