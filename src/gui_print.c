#include "gui_print.h"

extern struct page_def pages_iso_a[];

int print_win (gui_obj *gui){
	int show_print = 1;
	int i = 0, update = 0;
	static double page_w = 297.0, page_h = 210.0;
	static double ofs_x = 0.0, ofs_y = 0.0, scale = 1.0;
	static char page_w_str[64] = "297";
	static char page_h_str[64] = "210";
	static char ofs_x_str[64] = "0.00";
	static char ofs_y_str[64] = "0.00";
	static char scale_str[64] = "1.00";
	static char sel_file[MAX_PATH_LEN] = "output.pdf";
	static char tmp_str[64];
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 620;
	gui->next_win_h = 500;
	
	if (nk_begin(gui->ctx, "Print", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		static int show_app_file = 0, mono = 0, dark = 0, out_fmt = PRT_PDF;
		static const char *ext_type[] = {
			[PRT_PDF] = "PDF",
			[PRT_SVG] = "SVG",
			[PRT_PNG] = "PNG",
			[PRT_JPG] = "JPG",
			[PRT_BMP] = "BMP",
			[PRT_NONE] = "*"};
		static const char *ext_descr[] = {
			[PRT_PDF] = "Portable Document Format (.pdf)",
			[PRT_SVG] = "Scalable Vector Graphics (.svg)",
			[PRT_PNG] = "Image PNG (.png)",
			[PRT_JPG] = "Image JPG (.jpg)",
			[PRT_BMP] = "Image BMP (.bmp)",
			[PRT_NONE] = "All files (*)"};
		int i, ret = 0;
		
		bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
		bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
		bmp_color red = { .r = 255, .g = 0, .b = 0, .a = 255 };
		bmp_color yellow = { .r = 255, .g = 255, .b = 0, .a = 255 };
		bmp_color green = { .r = 0, .g = 255, .b = 0, .a = 255 };
		bmp_color cyan = { .r = 0, .g = 255, .b = 255, .a = 255 };
		bmp_color blue = { .r = 0, .g = 0, .b = 255, .a = 255 };
		bmp_color magenta = { .r = 255, .g = 0, .b = 255, .a = 255 };
		
		bmp_color y_dark = { .r = 150, .g = 150, .b = 0, .a = 255 };
		bmp_color g_dark = { .r = 0, .g = 129, .b = 0, .a = 255 };
		bmp_color c_dark = { .r = 0, .g = 129, .b = 129, .a = 255 };
		
		bmp_color list[] = { white, };
		bmp_color subst[] = { black, };
		
		static struct print_param param, prev_param;
			
		int len = 1;
		
		bmp_color list_dark[] = { white, yellow, green, cyan};
		bmp_color subst_dark[] = { black, y_dark, g_dark, c_dark};
		int len_dark = 4;
		
		param.scale = scale;
		param.ofs_x = ofs_x;
		param.ofs_y = ofs_y;
		param.w = page_w;
		param.h = page_h;
		param.mono = mono;
		param.inch = 0;
		
		if (!dark){
			param.list = list;
			param.subst = subst;
			param.len = len;
		}
		else{
			param.list = list_dark;
			param.subst = subst_dark;
			param.len = len_dark;
		}
		if (mono) param.list = NULL;
		
		
		if(prev_param.scale != param.scale ||
		prev_param.ofs_x != param.ofs_x ||
		prev_param.ofs_y != param.ofs_y ||
		prev_param.w != param.w ||
		prev_param.h != param.h ||
		prev_param.mono != param.mono ||
		prev_param.inch != param.inch ||
		prev_param.list != param.list ||
		prev_param.subst != param.subst ||
		prev_param.len != param.len ||
		prev_param.out_fmt != param.out_fmt)
		update = 1;
		
		nk_layout_row(gui->ctx, NK_STATIC, 180, 3, (float[]){160, 160, 190});
		if (nk_group_begin(gui->ctx, "Page setup", NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			enum paper_fam {
				ISO_A,
				ISO_B,
				US,
				ARCH
			};
			
			static const char *p_fam_descr[] = {"ISO A", "ISO B", "US", "ARCH"};
			static const char *unit_descr[] = {"in", "mm", "px"};
			
			
			static enum paper_fam tab_state = ISO_A;
			struct page_def *page_fam = pages_iso_a;
			int num_pages = 9;
			static int sel_page = 0;
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			int h = 4 * 22 + 5;
			h = (h < 300)? h : 300;
			tab_state = nk_combo (gui->ctx, p_fam_descr, 4, tab_state, 17, nk_vec2(100, h));
			/*
			nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
			nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
			if (gui_tab (gui, "ISO A", tab_state == ISO_A)) tab_state = ISO_A;
			if (gui_tab (gui, "ISO B", tab_state == ISO_B)) tab_state = ISO_B;
			if (gui_tab (gui, "US", tab_state == US)) tab_state = US;
			if (gui_tab (gui, "ARCH", tab_state == ARCH)) tab_state = ARCH;
			nk_style_pop_vec2(gui->ctx);
			nk_layout_row_end(gui->ctx);
			*/
			h = num_pages * 25 + 5;
			h = (h < 300)? h : 300;
			snprintf(tmp_str, 63, "%s - %.5gx%.5g%s",
				page_fam[sel_page].name,
				page_fam[sel_page].w,
				page_fam[sel_page].h,
				unit_descr[page_fam[sel_page].unit]);
			if (nk_combo_begin_label(gui->ctx, tmp_str, nk_vec2(200,h))){
				nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){70, 110});
				for (i = 0; i < num_pages; i++){
					if (nk_button_label(gui->ctx, page_fam[i].name)){
						sel_page = i;
						page_w = page_fam[i].w;
						snprintf(page_w_str, 63, "%.9g", page_w);
						page_h = page_fam[i].h;
						snprintf(page_h_str, 63, "%.9g", page_h);
						
						nk_combo_close(gui->ctx);
					}
					snprintf(tmp_str, 63, "%.5gx%.5g%s",
						page_fam[i].w, page_fam[i].h,
						unit_descr[page_fam[i].unit]);
					nk_label(gui->ctx, tmp_str, NK_TEXT_CENTERED);
				}
				
				nk_combo_end(gui->ctx);
			}
			/*
			nk_layout_row_dynamic(gui->ctx, 120, 1);
			if (nk_group_begin(gui->ctx, "Paper sizes", NK_WINDOW_BORDER)) {
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				for (i = 0; i < num_pages; i++){
					if (nk_button_label(gui->ctx, page_fam[i].name)){
						page_w = page_fam[i].w;
						snprintf(page_w_str, 63, "%.9g", page_w);
						page_h = page_fam[i].h;
						snprintf(page_h_str, 63, "%.9g", page_h);
					}
				}
				
				nk_group_end(gui->ctx);
			}
			*/
			
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 70});
			nk_label(gui->ctx, "Width:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_w_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_w_str)) /* update parameter value */
					page_w = atof(page_w_str);
				snprintf(page_w_str, 63, "%.9g", page_w);
				//update = 1;
			}
			
			nk_label(gui->ctx, "Height:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_h_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_h_str)) /* update parameter value */
					page_h = atof(page_h_str);
				snprintf(page_h_str, 63, "%.9g", page_h);
				//update = 1;
			}
			
			if (nk_button_label(gui->ctx, "Rotate")){
				double swap = page_w;
				page_w = page_h;
				snprintf(page_w_str, 63, "%.9g", page_w);
				page_h = swap;
				snprintf(page_h_str, 63, "%.9g", page_h);
			}
			nk_group_end(gui->ctx);
		}
		if (nk_group_begin(gui->ctx, "Scale & Position", NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 70});
				
			nk_label(gui->ctx, "Origin X:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_x_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_x_str)) /* update parameter value */
					ofs_x = atof(ofs_x_str);
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				//update = 1;
			}
			
			nk_label(gui->ctx, "Origin Y:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_y_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_y_str)) /* update parameter value */
					ofs_y = atof(ofs_y_str);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				//update = 1;
			}
			
			nk_label(gui->ctx, "Scale:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, scale_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(scale_str)) /* update parameter value */
					scale = atof(scale_str);
				snprintf(scale_str, 63, "%.9g", scale);
				//update = 1;
			}
			
			if (nk_button_label(gui->ctx, "View")){
				double zoom_x, zoom_y;
				ofs_x = gui->ofs_x;
				ofs_y = gui->ofs_y + (gui->main_h - gui->win_h) / gui->zoom ;
				//scale = (page_w * gui->zoom)/gui->win_w;
				
				zoom_x = gui->win_w/(page_w * gui->zoom);
				zoom_y = gui->win_h/(page_h * gui->zoom);
				scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
				scale = 1/scale;
				
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				snprintf(scale_str, 63, "%.9g", scale);
				update = 1;
			}
			if (nk_button_label(gui->ctx, "Fit all")){
				double min_x, min_y, max_x, max_y;
				double zoom_x, zoom_y;
				dxf_ents_ext(gui->drawing, &min_x, &min_y, &max_x, &max_y);
				zoom_x = fabs(max_x - min_x)/page_w;
				zoom_y = fabs(max_y - min_y)/page_h;
				scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
				scale = 1/scale;
				
				ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
				ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
				
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				snprintf(scale_str, 63, "%.9g", scale);
				update = 1;
			}
			
			nk_group_end(gui->ctx);
		}
		if (nk_group_begin(gui->ctx, "Preview", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* preview img */
			nk_layout_row_dynamic(gui->ctx, 172, 1);
			nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
			
			nk_group_end(gui->ctx);
		}
		
		
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_checkbox_label(gui->ctx, "Monochrome", &mono);
		nk_checkbox_label(gui->ctx, "Dark", &dark);
		
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){120, 270});
		nk_label(gui->ctx, "Output format:", NK_TEXT_RIGHT);
		/* file extension filter option */
		int h = (PRT_SIZE - 1) * 22 + 5;
		h = (h < 300)? h : 300;
		int fmt = nk_combo (gui->ctx, ext_descr, PRT_SIZE - 1, out_fmt, 17, nk_vec2(270, h));
		if (fmt != out_fmt){
			out_fmt = fmt;
			char *ext = get_ext(sel_file);
			char *end_fname = NULL;
			str_upp(ext);
			int ok = 0;
			for (i = 0; i < PRT_SIZE - 1; i++){
				if (strcmp(ext, ext_type[i]) == 0){
					ok =1;
					break;
				}
			}
			
			if (ok){
				end_fname = strrchr(sel_file, '.');
			}
			else if (strlen(sel_file) > 0){
				end_fname = sel_file + strlen(sel_file) - 1;
			}
			
			if (end_fname){
				end_fname[0] = '.';
				i = 1;
				while (end_fname[i] = tolower(ext_type[out_fmt][i -1])) i++;
				//sprintf(end_fname, ".%s", ext_type[out_fmt]);
			}
		}
		
		param.out_fmt = out_fmt;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Destination:", NK_TEXT_LEFT);
		
		/* dynamic width for dir/file name and fixed width for other informations */
		nk_layout_row_template_begin(gui->ctx, 20);
		nk_layout_row_template_push_static(gui->ctx, 80);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_end(gui->ctx);
		
		if (nk_button_label(gui->ctx, "Browse")){
			/* load more other fonts */
			show_app_file = 1;
			
			for (i = 0; i < PRT_SIZE; i++){
				gui->file_filter_types[i] = ext_type[i];
				gui->file_filter_descr[i] = ext_descr[i];
			}
			
			gui->file_filter_count = PRT_SIZE;
			
			gui->show_file_br = 1;
			gui->curr_path[0] = 0;
		}
		
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, MAX_PATH_LEN, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Print")){
			snprintf(gui->log_msg, 63, " ");
			
			if (out_fmt == PRT_PDF)
				ret = print_pdf(gui->drawing, param, sel_file);
			else if (out_fmt == PRT_SVG)
				ret = print_svg(gui->drawing, param, sel_file);
			else if (out_fmt == PRT_PNG ||
			out_fmt == PRT_JPG ||
			out_fmt == PRT_BMP)
				ret = print_img(gui->drawing, param, sel_file);
			
			if (ret)
				snprintf(gui->log_msg, 63, "Print: Created print output succesfully");
			else
				snprintf(gui->log_msg, 63, "Print Error");
		}
		
		if (update){
			update = 0;
			/* calcule the zoom and offset for preview */
			double z_x = page_w/gui->preview_img->width;
			double z_y = page_h/gui->preview_img->height;
			double z = (z_x > z_y) ? z_x : z_y;
			if (z <= 0.0) z =1.0;
			else z = 1/z;
				
			int w = (int)(page_w * z);
			int h = (int)(page_h * z);
			int x = -(int)(w - gui->preview_img->width) / 2;
			int y = -(int)(h - gui->preview_img->height) / 2;
			
			double o_x = ofs_x - (double) x / (z * scale);
			double o_y = ofs_y - (double) y / (z * scale);
			
			bmp_fill(gui->preview_img, gui->preview_img->bkg); /* clear bitmap */
			
			gui->preview_img->clip_x = (unsigned int)x;
			gui->preview_img->clip_y = (unsigned int)y;
			gui->preview_img->clip_w = (unsigned int)w;
			gui->preview_img->clip_h = (unsigned int)h;
			
			bmp_fill_clip(gui->preview_img, white); /* clear bitmap on page bounds */
			
			//dxf_ents_draw(gui->drawing, gui->preview_img, o_x, o_y, z * scale);
			struct draw_param d_param;
			
			d_param.ofs_x = o_x;
			d_param.ofs_y = o_y;
			d_param.scale = z * scale;
			d_param.list = param.list;
			d_param.subst = param.subst;
			d_param.len_subst = param.len;
			d_param.inc_thick = 0;
			
			dxf_ents_draw(gui->drawing, gui->preview_img, d_param);
			
			/* draw page rectangle */
			bmp_color blue = { .r = 0, .g = 0, .b = 255, .a = 255 };
			gui->preview_img->frg = blue;
			gui->preview_img->tick = 0;
			
			bmp_line(gui->preview_img, x, y, x + w - 1, y);
			bmp_line(gui->preview_img, x + w - 1, y, x + w - 1, y + h - 1);
			bmp_line(gui->preview_img, x + w - 1, y + h - 1, x, y + h - 1);
			bmp_line(gui->preview_img, x, y + h - 1, x, y);
		}
		if (show_app_file){
			if (gui->show_file_br == 2){
				gui->show_file_br = 0;
				show_app_file = 0;
				strncpy(sel_file, gui->curr_path, MAX_PATH_LEN);
			}
		}
		
		prev_param.scale = param.scale;
		prev_param.ofs_x = param.ofs_x;
		prev_param.ofs_y = param.ofs_y;
		prev_param.w = param.w;
		prev_param.h = param.h;
		prev_param.mono = param.mono;
		prev_param.inch = param.inch;
		prev_param.list = param.list;
		prev_param.subst = param.subst;
		prev_param.len = param.len;
		prev_param.out_fmt = param.out_fmt;
		
	}
	else{
		show_print = 0;
		bmp_fill(gui->preview_img, gui->preview_img->bkg); /* clear bitmap */
	}
	nk_end(gui->ctx);
	
	return show_print;
}