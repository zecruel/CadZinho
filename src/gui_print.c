#include "gui.h"
#include "gui_file.h"

int print_win (gui_obj *gui){
	int show_print = 1;
	int i = 0;
	static double page_w = 297.0, page_h = 210.0;
	static double ofs_x = 0.0, ofs_y = 0.0, scale = 1.0;
	static char page_w_str[64] = "297.00";
	static char page_h_str[64] = "210.00";
	static char ofs_x_str[64] = "0.00";
	static char ofs_y_str[64] = "0.00";
	static char scale_str[64] = "1.00";
	static char sel_file[MAX_PATH_LEN];
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 420;
	gui->next_win_h = 500;
	
	if (nk_begin(gui->ctx, "Print", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		static int show_app_file = 0;
		nk_layout_row_static(gui->ctx, 180, 190, 2);
		if (nk_group_begin(gui->ctx, "Page setup", NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
			nk_layout_row_static(gui->ctx, 20, 70, 2);
			nk_label(gui->ctx, "Width:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_w_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_w_str)) /* update parameter value */
					page_w = atof(page_w_str);
				snprintf(page_w_str, 63, "%0.2f", page_w);
			}
			
			nk_label(gui->ctx, "Height:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_h_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_h_str)) /* update parameter value */
					page_h = atof(page_h_str);
				snprintf(page_h_str, 63, "%0.2f", page_h);
			}
			
			nk_label(gui->ctx, "Offset X:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_x_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_x_str)) /* update parameter value */
					ofs_x = atof(ofs_x_str);
				snprintf(ofs_x_str, 63, "%0.2f", ofs_x);
			}
			
			nk_label(gui->ctx, "Offset Y:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_y_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_y_str)) /* update parameter value */
					ofs_y = atof(ofs_y_str);
				snprintf(ofs_y_str, 63, "%0.2f", ofs_y);
			}
			
			nk_label(gui->ctx, "Scale:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, scale_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){
				nk_edit_unfocus(gui->ctx);
				if (strlen(scale_str)) /* update parameter value */
					scale = atof(scale_str);
				snprintf(scale_str, 63, "%0.2f", scale);
			}
			nk_group_end(gui->ctx);
		}
		if (nk_group_begin(gui->ctx, "Preview", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* preview img */
			nk_layout_row_dynamic(gui->ctx, 172, 1);
			nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
			
			nk_group_end(gui->ctx);
		}
		nk_layout_row_static(gui->ctx, 20, 80, 2);
		if (nk_button_label(gui->ctx, "View")){
			
			ofs_x = gui->ofs_x;
			ofs_y = gui->ofs_y + (gui->main_h - gui->win_h) / gui->zoom ;
			scale = (page_w * gui->zoom)/gui->win_w;
			
			snprintf(ofs_x_str, 63, "%0.2f", ofs_x);
			snprintf(ofs_y_str, 63, "%0.2f", ofs_y);
			snprintf(scale_str, 63, "%0.2f", scale);
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
			
			snprintf(ofs_x_str, 63, "%0.2f", ofs_x);
			snprintf(ofs_y_str, 63, "%0.2f", ofs_y);
			snprintf(scale_str, 63, "%0.2f", scale);
		}
		
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
			
			static const char *ext_type[] = { "PDF", "*"};
			static const char *ext_descr[] = { "PDF (.pdf)", "All files (*)"};
			gui->file_filter_types[0] = ext_type[0];
			gui->file_filter_types[1] = ext_type[1];
			gui->file_filter_descr[0] = ext_descr[0];
			gui->file_filter_descr[1] = ext_descr[1];
			
			gui->file_filter_count = 2;
			
			gui->show_file_br = 1;
			gui->curr_path[0] = 0;
		}
		
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, MAX_PATH_LEN, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Print")){
			print_pdf(gui->drawing, scale, ofs_x, ofs_y, page_w, page_h);
		}
		
		
		
		if (show_app_file){
			if (gui->show_file_br == 2){
				gui->show_file_br = 0;
				show_app_file = 0;
				strncpy(sel_file, gui->curr_path, MAX_PATH_LEN);
			}
			
			
			
			
			
			#if(0)
			/* popup to open other font files */
			enum files_types filters[5] = {FILE_PDF, FILE_ALL};
			
			show_app_file = file_pop (gui, filters, 2, NULL);
			if (show_app_file == 2){
				//gui->curr_path
				show_app_file = 0;
			}
			#endif
		}
		
	} else show_print = 0;
	nk_end(gui->ctx);
	
	return show_print;
}