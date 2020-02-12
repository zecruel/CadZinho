#include "gui_use.h"
#include "gui_lay.h"
#include "gui_file.h"

extern const char *dxf_seed_2007;

int gui_tools_win (gui_obj *gui){

	if (nk_begin(gui->ctx, "Toolbox", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)){
			
		//if (nk_tree_push(gui->ctx, NK_TREE_TAB, "Place", NK_MAXIMIZED)) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Modify:", NK_TEXT_LEFT);
		
		nk_layout_row_static(gui->ctx, 28, 28, 6);
		
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CURSOR]))){
			gui->modal = SELECT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT_E]))){
			gui->modal = ED_TEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MOVE]))){
			gui->modal = MOVE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DUPLI]))){
			gui->modal = DUPLI;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SCALE]))){
			gui->modal = SCALE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ROT]))){
			gui->modal = ROTATE;
			gui->step = 0;
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MIRROR]))){
			gui->modal = MIRROR;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BLOCK]))){
			gui->modal = NEW_BLK;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPLODE]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EDIT]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG_E]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FIND]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RULER]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_STYLE]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TRASH]))){
			gui->action = DELETE;
			gui->step = 0;
		}
			
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place:", NK_TEXT_LEFT);
			
		nk_layout_row_static(gui->ctx, 28, 28, 6);
		
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LINE]))){
			gui->modal = LINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PLINE]))){
			gui->modal = POLYLINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RECT]))){
			gui->modal = RECT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT]))){
			gui->modal = TEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CIRCLE]))){
			gui->modal = CIRCLE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ELIPSE]))){
			gui->modal = ELLIPSE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ARC]))){
			gui->modal = ARC;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SPLINE]))){
			gui->modal = SPLINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HATCH]))){
			gui->modal = HATCH;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_I_TEXT]))){
			gui->modal = MTEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BOOK]))){
			gui->modal = INSERT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_IMAGE]))){
			gui->modal = IMAGE;
			gui->step = 0;
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 1); /*space*/
		
		struct nk_vec2 wid_pos = nk_widget_position(gui->ctx);
		struct nk_vec2 win_pos = nk_window_get_position(gui->ctx);
		struct nk_rect win_cont = nk_window_get_content_region(gui->ctx);
	
		nk_layout_row_dynamic(gui->ctx, win_cont.h - (wid_pos.y - win_pos.y), 1);
		if (nk_group_begin(gui->ctx, "especific", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			gui_select_info (gui);
			gui_line_info (gui);
			gui_pline_info (gui);
			gui_circle_info (gui);
			gui_rect_info (gui);
			gui_text_info (gui);
			gui_mtext_info (gui);
			gui_move_info (gui);
			gui_dupli_info (gui);
			gui_scale_info (gui);
			gui_rotate_info (gui);
			gui_mirror_info (gui);
			gui_insert_info (gui);
			gui_block_info (gui);
			gui_hatch_info (gui);
			gui_paste_info (gui);
			gui_ed_text_info (gui);
			gui_spline_info (gui);
			gui_arc_info (gui);
			gui_ellip_info (gui);
			gui_image_info (gui);
			
			if (gui->modal == SCRIPT) {
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, "Runing Script", NK_TEXT_LEFT);
			}
			
			nk_group_end(gui->ctx);
		}
	}
	nk_end(gui->ctx);
	return 1;
}

int gui_main_win(gui_obj *gui){
	int i;
	
	if (nk_begin(gui->ctx, "Main", nk_rect(2, 2, gui->win_w - 4, 6 + 4 + ICON_SIZE + 4 + 6 + 4 + ICON_SIZE + 4 + 6 + 8),
	NK_WINDOW_BORDER)){
		/* first line */
		nk_layout_row_begin(gui->ctx, NK_STATIC, ICON_SIZE + 12, 8);
		
		/* file tools*/
		nk_layout_row_push(gui->ctx, 6*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "file", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NEW]))){
				while (dxf_read (gui->drawing, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &gui->progress) > 0){
	
				}
				gui->layer_idx = dxf_lay_idx (gui->drawing, "0");
				gui->ltypes_idx = dxf_ltype_idx (gui->drawing, "BYLAYER");
				gui->t_sty_idx = dxf_tstyle_idx (gui->drawing, "STANDARD");
				gui->color_idx = 256;
				gui->lw_idx = DXF_LW_LEN;
				gui->curr_path[0] = 0;
				gui->drwg_hist_pos ++;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_OPEN]))){
				gui->action = FILE_OPEN;
				gui->show_app_file = 1;
				
				gui->curr_path[0] = 0;
				
				gui->path_ok = 0;
				gui->hist_new = 1;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SAVE]))){
				gui->action = FILE_SAVE;
				gui->show_app_file = 1;
				gui->path_ok = 0;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPORT]))){
				gui->action = EXPORT;
				gui->show_app_file = 1;
				gui->path_ok = 0;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CLOSE]))){
				printf("CLOSE\n");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PRINT]))){
				//printf("PRINT\n");
				//print_pdf(gui->drawing);
				gui->show_print = 1;
			}
			
			nk_group_end(gui->ctx);
		}
		
		/* clipboard tools*/
		nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "clipboard", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			
			
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_COPY]))){
				gui->action = YANK;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CUT]))){
				gui->action = CUT;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PASTE]))){
				gui->action = START_PASTE;
				
			}
			
			nk_group_end(gui->ctx);
		}
		
		/* undo/redo tools*/
		nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "undo-redo", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UNDO]))){
				gui->action = UNDO;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_REDO]))){
				gui->action = REDO;
			}
			nk_group_end(gui->ctx);
		}
		
		/* managers*/
		nk_layout_row_push(gui->ctx, 5*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "managers", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LAYERS]))){
				//printf("Layers\n");
				gui->show_lay_mng = 1;
				//sel_tmp = -1;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FONT]))){
				//printf("FONT\n");
				gui->show_tstyles_mng = 1;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LTYPE]))){
				printf("Line types\n");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PUZZLE]))){
				printf("Blocks\n");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAGS]))){
				printf("APPID\n");
			}
			nk_group_end(gui->ctx);
		}
		
		/* zoom tools*/
		nk_layout_row_push(gui->ctx, 4*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "zoom", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_P]))){
				gui->action = VIEW_ZOOM_P;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_M]))){
				gui->action = VIEW_ZOOM_M;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_W]))){
				gui->action = VIEW_ZOOM_W;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_A]))){
				gui->action = VIEW_ZOOM_EXT;
			}
			nk_group_end(gui->ctx);
		}
		
		/* pan tools*/
		nk_layout_row_push(gui->ctx, 4*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "pan", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UP]))){
				gui->action = VIEW_PAN_U;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DOWN]))){
				gui->action = VIEW_PAN_D;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LEFT]))){
				gui->action = VIEW_PAN_L;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RIGTH]))){
				gui->action = VIEW_PAN_R;
			}
			nk_group_end(gui->ctx);
		}
		
		/* config tools*/
		nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "config", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_INFO]))){
				gui->show_info = 1;
				//printf("Info\n");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TOOL]))){
				//printf("Tools\n");
				#ifdef OS_WIN
				DebugBreak();
				#endif
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_GEAR]))){
				//printf("Config\n");
				//gui->file_filter_types[0] = ext_types[FILE_ALL];
				//gui->file_filter_descr[0] = ext_descr[FILE_ALL];
				
				//gui->file_filter_count = 1;
				
				//gui->show_file_br = 1;
				
				gui->show_app_file = 1;
			}
			nk_group_end(gui->ctx);
		}
		
		/* config tools*/
		nk_layout_row_push(gui->ctx, 1*(ICON_SIZE + 4 + 4) + 13);
		if (nk_group_begin(gui->ctx, "help", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HELP]))){
				gui->show_app_about = 1;
				//printf("HELP\n");
			}
			nk_group_end(gui->ctx);
		}
		
		nk_layout_row_end(gui->ctx);
		/*------------ end first line --------------*/
		
		/* second line */
		nk_layout_row_begin(gui->ctx, NK_STATIC, ICON_SIZE + 4, ICON_SIZE + 4);
		
		static char text[64];
		int text_len;
		nk_layout_row_push(gui->ctx, 1000);
		if (nk_group_begin(gui->ctx, "Prop", NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 20);
			
			/*layer*/
			nk_layout_row_push(gui->ctx, 60);
			nk_label(gui->ctx, "Layer: ", NK_TEXT_RIGHT);
			nk_layout_row_push(gui->ctx, 200);
			layer_prop(gui);
			
			
			/*color picker */
			int c_idx = gui->color_idx;
			if (c_idx >255){
				c_idx = gui->drawing->layers[gui->layer_idx].color;
				if (c_idx >255){
					c_idx = 0;
				}
			}
			/* fill the tiny bitmap with selected color*/
			bmp_fill(gui->color_img, dxf_colors[c_idx]);
			
			/* print the name (number) of color */
			if (gui->color_idx == 0){
				text_len = snprintf(text, 63, "%s", "ByB");
			}
			else if (gui->color_idx < 256){
				text_len = snprintf(text, 63, "%d", gui->color_idx);
			}
			else{
				text_len = snprintf(text, 63, "%s", "ByL");
			}
			nk_layout_row_push(gui->ctx, 70);
			nk_label(gui->ctx, "Color: ", NK_TEXT_RIGHT);
			nk_layout_row_push(gui->ctx, 70);
			if (nk_combo_begin_image_label(gui->ctx, text, nk_image_ptr(gui->color_img), nk_vec2(215,320))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "By Layer")){
					gui->color_idx = 256;
					gui->action = COLOR_CHANGE;
					nk_combo_close(gui->ctx);
				}
				if (nk_button_label(gui->ctx, "By Block")){
					gui->color_idx = 0;
					gui->action = COLOR_CHANGE;
					nk_combo_close(gui->ctx);
				}
				nk_layout_row_static(gui->ctx, 15, 15, 10);
				nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
				
				for (i = 1; i < 256; i++){
					struct nk_color b_color = {
						.r = dxf_colors[i].r,
						.g = dxf_colors[i].g,
						.b = dxf_colors[i].b,
						.a = dxf_colors[i].a
					};
					if(nk_button_color(gui->ctx, b_color)){
						gui->color_idx = i;
						gui->action = COLOR_CHANGE;
						nk_combo_close(gui->ctx);
					}
				}
				
				nk_combo_end(gui->ctx);
			}
			
			/*line type*/
			nk_layout_row_push(gui->ctx, 100);
			nk_label(gui->ctx, "Line type: ", NK_TEXT_RIGHT);
			nk_layout_row_push(gui->ctx, 200);
			
			int num_ltypes = gui->drawing->num_ltypes;

			int h = num_ltypes * 25 + 5;
			h = (h < 200)? h : 200;
			
			if (nk_combo_begin_label(gui->ctx, gui->drawing->ltypes[gui->ltypes_idx].name, nk_vec2(300, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				
				for (i = 0; i < num_ltypes; i++){
					
					if (nk_button_label(gui->ctx, gui->drawing->ltypes[i].name)){
						gui->ltypes_idx = i;
						gui->action = LTYPE_CHANGE;
						nk_combo_close(gui->ctx);
					}
					nk_label(gui->ctx, gui->drawing->ltypes[i].descr, NK_TEXT_LEFT);
				}
				
				nk_combo_end(gui->ctx);
			}
			
			/* thickness 
			nk_layout_row_push(gui->ctx, 150);
			//nk_property_float(struct nk_context*, const char *name, float min, float *val, float max, float step, float inc_per_pixel);
			//nk_property_float(gui->ctx, "Thick:", 0.0, &thick, 20.0, 0.1, 0.1);
			
			//double nk_propertyd(struct nk_context*, const char *name, double min, double val, double max, double step, float inc_per_pixel);
			thick = nk_propertyd(gui->ctx, "Thickness", 0.0d, thick_prev, 20.0d, 0.1d, 0.1d);
			if (thick_prev != thick){
				gui->action = THICK_CHANGE;
				//printf ("thick change\n");
			}
			thick_prev = thick;*/
			
			nk_layout_row_push(gui->ctx, 120);
			nk_label(gui->ctx, "Line weight: ", NK_TEXT_RIGHT);
			nk_layout_row_push(gui->ctx, 120);
			
			if (nk_combo_begin_label(gui->ctx, dxf_lw_descr[gui->lw_idx], nk_vec2(200,300))){
				nk_layout_row_dynamic(gui->ctx, 25, 2);
				if (nk_button_label(gui->ctx, "By Layer")){
					gui->lw_idx = DXF_LW_LEN;
					gui->action = LW_CHANGE;
					nk_combo_close(gui->ctx);
				}
				if (nk_button_label(gui->ctx, "By Block")){
					gui->lw_idx = DXF_LW_LEN + 1;
					gui->action = LW_CHANGE;
					nk_combo_close(gui->ctx);
				}
				nk_layout_row_dynamic(gui->ctx, 17, 1);
				for (i = 0; i < DXF_LW_LEN; i++){
					if (nk_button_label(gui->ctx, dxf_lw_descr[i])){
						gui->lw_idx = i;
						gui->action = LW_CHANGE;
						nk_combo_close(gui->ctx);
					}
				}
				
				nk_combo_end(gui->ctx);
			}
			
			nk_layout_row_end(gui->ctx);
			
			nk_group_end(gui->ctx);
		}
		
		nk_layout_row_end(gui->ctx);
		/*------------ end second line --------------*/
		
		if (gui->show_app_about){
			/* file open popup */
			static struct nk_rect s = {20, 100, 400, 150};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "About", NK_WINDOW_CLOSABLE, s)){
				nk_layout_row_dynamic(gui->ctx, 50, 2);
				nk_label(gui->ctx, "CadZinho", NK_TEXT_RIGHT);
				nk_image(gui->ctx, nk_image_ptr(gui->i_cz48));
				//nk_layout_row_dynamic(gui->ctx, 165, 1);
				//nk_image(gui->ctx, i_cz);
				nk_layout_row_begin(gui->ctx, NK_DYNAMIC, 20, 2);
				nk_layout_row_push(gui->ctx, 0.7f);
				nk_label(gui->ctx, "By Ezequiel Rabelo de Aguiar", NK_TEXT_RIGHT);
				nk_layout_row_push(gui->ctx, 0.3f);
				nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_BRAZIL]));
				nk_layout_row_end(gui->ctx);
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, "CadZinho is licensed under the MIT License.",  NK_TEXT_LEFT);
				nk_popup_end(gui->ctx);
			} else gui->show_app_about = nk_false;
		}
		
		if (gui->show_app_file){
			enum files_types filters[2] = {FILE_DXF, FILE_ALL};
			char path[20];
			gui->show_app_file = file_pop (gui, filters, 2, NULL);
			if (gui->show_app_file == 2){
					//if (strlen(file_path) > 4){
						gui->path_ok = 1;
					//}
				
				gui->show_app_file = 0;
			}
		}
	}
	nk_end(gui->ctx);
	
	
	return 1;
	
}