#include "gui_print.h"

extern struct page_def *fam_pages[];
extern const char *fam_pages_descr[];
extern const int fam_pages_len[];

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
	static char sel_file[PATH_MAX_CHARS] = "output.pdf";
	static char tmp_str[64];
	static int unit = PRT_MM, new_unit = PRT_MM;
	static int mono = 0, dark = 0, out_fmt = PRT_PDF;
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 580;
	gui->next_win_h = 350;
	
	/* print window */
	if (nk_begin(gui->ctx, "Print", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		static int show_app_file = 0;
		int i, ret = 0;
		static struct print_param param, prev_param;
		
		/* supported output formats */
		static const char *ext_type[] = {
			[PRT_PDF] = "PDF",
			[PRT_SVG] = "SVG",
			[PRT_PNG] = "PNG",
			[PRT_JPG] = "JPG",
			[PRT_BMP] = "BMP",
			[PRT_PS] = "PS",
			[PRT_NONE] = "*"
		};
		static const char *ext_descr[] = {
			[PRT_PDF] = "Portable Document Format (.pdf)",
			[PRT_SVG] = "Scalable Vector Graphics (.svg)",
			[PRT_PNG] = "Image PNG (.png)",
			[PRT_JPG] = "Image JPG (.jpg)",
			[PRT_BMP] = "Image Bitmap (.bmp)",
			[PRT_PS] = "Postscript (.ps)",
			[PRT_NONE] = "All files (*)"
		};
		
		/* basic colors */
		bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
		bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
		bmp_color red = { .r = 255, .g = 0, .b = 0, .a = 255 };
		bmp_color yellow = { .r = 255, .g = 255, .b = 0, .a = 255 };
		bmp_color green = { .r = 0, .g = 255, .b = 0, .a = 255 };
		bmp_color cyan = { .r = 0, .g = 255, .b = 255, .a = 255 };
		bmp_color blue = { .r = 0, .g = 0, .b = 255, .a = 255 };
		bmp_color magenta = { .r = 255, .g = 0, .b = 255, .a = 255 };
		
		/* colors for dark option substitution */
		bmp_color y_dark = { .r = 150, .g = 150, .b = 0, .a = 255 };
		bmp_color g_dark = { .r = 0, .g = 129, .b = 0, .a = 255 };
		bmp_color c_dark = { .r = 0, .g = 129, .b = 129, .a = 255 };
		
		/* default substitution list (display white -> print black) */
		bmp_color list[] = { white, };
		bmp_color subst[] = { black, };
		int len = 1;
		
		/* dark substitution list */
		bmp_color list_dark[] = { white, yellow, green, cyan};
		bmp_color subst_dark[] = { black, y_dark, g_dark, c_dark};
		int len_dark = 4;
		
		/* update structure parameters */
		param.scale = scale;
		param.ofs_x = ofs_x;
		param.ofs_y = ofs_y;
		param.w = page_w;
		param.h = page_h;
		param.mono = mono;
		param.unit = unit;
		if (!dark){ /* default substituition parameters */
			param.list = list;
			param.subst = subst;
			param.len = len;
		}
		else{ /* dark substituition parameters */
			param.list = list_dark;
			param.subst = subst_dark;
			param.len = len_dark;
		}
		if (mono) { /* monochrome substituition parameters */
			param.list = NULL; 
			param.subst = subst;
			param.len = len;
		}
		
		/* verify if any parameter was changed */
		if(prev_param.scale != param.scale ||
		prev_param.ofs_x != param.ofs_x ||
		prev_param.ofs_y != param.ofs_y ||
		prev_param.w != param.w ||
		prev_param.h != param.h ||
		prev_param.mono != param.mono ||
		prev_param.unit != param.unit ||
		prev_param.list != param.list ||
		prev_param.subst != param.subst ||
		prev_param.len != param.len ||
		prev_param.out_fmt != param.out_fmt)
		update = 1; /* then update preview */
		
		nk_layout_row(gui->ctx, NK_STATIC, 180, 3, (float[]){190, 160, 190});
		/* Paper size setup */
		if (nk_group_begin(gui->ctx, "Page setup", NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			static const char *unit_descr[] = {"in", "mm", "px"};
			static int num_fam = 6; /* number of paper families */
			static int curr_fam = -1; /* current selected paper family (-1 -> none selected) */
			static int sel_page = -1; /* current selected paper (-1 -> none selected) */
			
			/* combo to choose current paper family */
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			int h = num_fam * 22 + 5;
			h = (h < 300)? h : 300;
			gui->paper_fam = nk_combo (gui->ctx, fam_pages_descr, num_fam, gui->paper_fam, 17, nk_vec2(100, h));
			
			/* get current paper family length */
			struct page_def *page_fam = fam_pages[gui->paper_fam];
			int num_pages = fam_pages_len[gui->paper_fam];
			
			/* validate selected paper index with current length */
			if (gui->sel_paper >= num_pages) gui->sel_paper = 0;
			
			/* combo to choose current paper */
			h = num_pages * 25 + 5;
			h = (h < 300)? h : 300;
			snprintf(tmp_str, 63, "%s - %.5gx%.5g %s", /* show name and dimensions of selected paper */
				page_fam[gui->sel_paper].name,
				page_fam[gui->sel_paper].w,
				page_fam[gui->sel_paper].h,
				unit_descr[page_fam[gui->sel_paper].unit]);
			if (nk_combo_begin_label(gui->ctx, tmp_str, nk_vec2(250,h))){
				nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){100, 120});
				for (i = 0; i < num_pages; i++){ /* sweep current family list */
					/* paper name */
					if (nk_button_label(gui->ctx, page_fam[i].name)){
						gui->sel_paper = i; /* change current paper */
						
						nk_combo_close(gui->ctx);
					}
					/* paper dimensions */
					snprintf(tmp_str, 63, "%.5gx%.5g %s",
						page_fam[i].w, page_fam[i].h,
						unit_descr[page_fam[i].unit]);
					nk_label(gui->ctx, tmp_str, NK_TEXT_CENTERED);
				}
				nk_combo_end(gui->ctx);
			}
			
			/* verify if family or selected paper was changed */
			if (curr_fam != gui->paper_fam || 
				sel_page != gui->sel_paper)
			{ /* then update parameters */
				curr_fam = gui->paper_fam;
				sel_page = gui->sel_paper;
				page_w = page_fam[gui->sel_paper].w;
				snprintf(page_w_str, 63, "%.9g", page_w);
				page_h = page_fam[gui->sel_paper].h;
				snprintf(page_h_str, 63, "%.9g", page_h);
				unit = page_fam[gui->sel_paper].unit;
				update = 1;
			}
			/* below, the parameters to customize print paper sizes */
			
			/* units options */
			nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 3, (float[]){0.25, 0.4, 0.35});
			new_unit = unit;
			if (nk_option_label(gui->ctx, "mm", new_unit == PRT_MM)) {new_unit = PRT_MM;}
			if (nk_option_label(gui->ctx, "inches", new_unit == PRT_IN)) {new_unit = PRT_IN;}
			if (nk_option_label(gui->ctx, "pixels", new_unit == PRT_PX)) {new_unit = PRT_PX;}
			
			if (unit != new_unit){ /* if units was changed */
				/* convert sizes to new unit */
				double factor1 = 1.0, factor2 = 1.0;
				
				/* previous unit */
				if (unit == PRT_MM) factor1 = 25.4;
				else if (unit == PRT_IN) factor1 = 1;
				else if (unit == PRT_PX) factor1 = 96;
				
				/* current unit */
				if (new_unit == PRT_MM) factor2 = 25.4;
				else if (new_unit == PRT_IN) factor2 = 1;
				else if (new_unit == PRT_PX) factor2 = 96;
				
				/* convert, based in relationship between old and new units*/
				page_w = page_w * factor2/factor1;
				snprintf(page_w_str, 63, "%.9g", page_w);
				page_h = page_h * factor2/factor1;
				snprintf(page_h_str, 63, "%.9g", page_h);
				scale = scale * factor2/factor1;
				snprintf(scale_str, 63, "%.9g", scale);
				
				unit = new_unit;
			}
			
			/* width customization */
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 70});
			nk_label(gui->ctx, "Width:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_w_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_w_str)) /* update parameter value */
					page_w = atof(page_w_str);
				snprintf(page_w_str, 63, "%.9g", page_w);
			}
			
			/* height customization */
			nk_label(gui->ctx, "Height:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, page_h_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(page_h_str)) /* update parameter value */
					page_h = atof(page_h_str);
				snprintf(page_h_str, 63, "%.9g", page_h);
			}
			
			/* rotate page */
			if (nk_button_label(gui->ctx, "Rotate")){
				/* simply swap dimentions */
				double swap = page_w;
				page_w = page_h;
				snprintf(page_w_str, 63, "%.9g", page_w);
				page_h = swap;
				snprintf(page_h_str, 63, "%.9g", page_h);
			}
			
			nk_group_end(gui->ctx);
		}
		/* adjust drawing position and scale on print area */
		if (nk_group_begin(gui->ctx, "Scale & Position", NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 70});
			
			/* customize x axis origin of print area (left lower corner) in drawing */
			nk_label(gui->ctx, "Origin X:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_x_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_x_str)) /* update parameter value */
					ofs_x = atof(ofs_x_str);
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
			}
			
			/* customize x axis origin of print area (left lower corner) in drawing */
			nk_label(gui->ctx, "Origin Y:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_y_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_y_str)) /* update parameter value */
					ofs_y = atof(ofs_y_str);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
			}
			
			/* customize drawing scale in print area ( [drawing unit]/[print unit] factor) */
			nk_label(gui->ctx, "Scale:", NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, scale_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(scale_str)) /* update parameter value */
					scale = atof(scale_str);
				snprintf(scale_str, 63, "%.9g", scale);
			}
			
			/* adjust print parameters to fit the current drawing view in screen */
			if (nk_button_label(gui->ctx, "View")){
				double zoom_x, zoom_y;
				
				/* get origin */
				ofs_x = gui->ofs_x;
				ofs_y = gui->ofs_y;
				
				/* get the better scale to fit in width or height */
				zoom_x = gui->win_w/(page_w * gui->zoom);
				zoom_y = gui->win_h/(page_h * gui->zoom);
				scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
				scale = 1/scale;
				
				/* update parameters */
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				snprintf(scale_str, 63, "%.9g", scale);
				update = 1;
			}
			
			/* adjust print parameters to fit the drawing extends */
			if (nk_button_label(gui->ctx, "Fit all")){
				double min_x, min_y, max_x, max_y;
				double zoom_x, zoom_y;
				
				/* get drawing extents */
				dxf_ents_ext(gui->drawing, &min_x, &min_y, &max_x, &max_y);
				
				/* get the better scale to fit in width or height */
				zoom_x = fabs(max_x - min_x)/page_w;
				zoom_y = fabs(max_y - min_y)/page_h;
				scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
				scale = 1/scale;
				
				/* get origin */
				ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
				ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
				
				/* update parameters */
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				snprintf(scale_str, 63, "%.9g", scale);
				update = 1;
			}
			/* adjust print parameters to centralize small figure in page*/
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			if (nk_button_label(gui->ctx, "Centralize")){
				double min_x, min_y, max_x, max_y;
				
				/* get drawing extents */
				dxf_ents_ext(gui->drawing, &min_x, &min_y, &max_x, &max_y);
				
				scale = atof(scale_str);
				
				/* get origin */
				ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
				ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
				
				/* update parameters */
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
				//snprintf(scale_str, 63, "%.9g", scale);
				update = 1;
			}
			
			nk_group_end(gui->ctx);
		}
		
		/* show print preview image */
		if (nk_group_begin(gui->ctx, "Preview", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(gui->ctx, 172, 1);
			nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
			
			nk_group_end(gui->ctx);
		}
		
		/* choose color options (color substitution tables)*/
		nk_layout_row(gui->ctx, NK_STATIC, 20, 3, (float[]){120, 120, 70});
		nk_label(gui->ctx, "Color options:", NK_TEXT_RIGHT);
		nk_checkbox_label(gui->ctx, "Monochrome", &mono);
		nk_checkbox_label(gui->ctx, "Dark", &dark);
		
		/* choose output file format */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){120, 270});
		nk_label(gui->ctx, "Output format:", NK_TEXT_RIGHT);
		/* combo for file extension filter */
		int h = (PRT_SIZE - 1) * 22 + 5;
		h = (h < 300)? h : 300;
		int fmt = nk_combo (gui->ctx, ext_descr, PRT_SIZE - 1, out_fmt, 17, nk_vec2(270, h));
		/* if current format type was changed */
		if (fmt != out_fmt){
			out_fmt = fmt;
			if (strlen(sel_file) > 0){
				/* change the extension in output path string */
				char *end_fname = strrchr(sel_file, '.'); /* try to find old extension */
				if (!end_fname) end_fname = sel_file + strlen(sel_file); /* or consider the string end */
				/* write to string */
				end_fname[0] = '.';
				i = 1;
				while (end_fname[i] = tolower(ext_type[out_fmt][i -1])) i++;
			}			
		}
		/* update output parameter*/
		param.out_fmt = out_fmt;
		
		/* choose output file path */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Destination:", NK_TEXT_LEFT);
		nk_layout_row_template_begin(gui->ctx, 20);
		nk_layout_row_template_push_static(gui->ctx, 80);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_end(gui->ctx);
		if (nk_button_label(gui->ctx, "Browse")){/* call file browser */
			show_app_file = 1;
			/* set filter for suported output formats */
			for (i = 0; i < PRT_SIZE; i++){
				gui->file_filter_types[i] = ext_type[i];
				gui->file_filter_descr[i] = ext_descr[i];
			}
			gui->file_filter_count = PRT_SIZE;
			gui->filter_idx = out_fmt;
			
			gui->show_file_br = 1;
			gui->curr_path[0] = 0;
		}
		if (show_app_file){ /* running file browser */
			if (gui->show_file_br == 2){ /* return file OK */
				/* close browser window*/
				gui->show_file_br = 0;
				show_app_file = 0;
				/* update output path */
				strncpy(sel_file, gui->curr_path, PATH_MAX_CHARS);
				/* update output format*/
				out_fmt = gui->filter_idx;
				if (out_fmt >= PRT_SIZE - 1) out_fmt = 0;
			}
		} /* manual entry to output path */
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, PATH_MAX_CHARS, nk_filter_default);
		
		/* Print command */
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Print")){
			snprintf(gui->log_msg, 63, " ");
			/* call corresponding  function, based on output format */
			if (out_fmt == PRT_PDF)
				ret = print_pdf(gui->drawing, param, sel_file);
			else if (out_fmt == PRT_SVG)
				ret = print_svg(gui->drawing, param, sel_file);
			else if (out_fmt == PRT_PNG ||
			out_fmt == PRT_JPG ||
			out_fmt == PRT_BMP)
				ret = print_img(gui->drawing, param, sel_file);
			else if (out_fmt == PRT_PS)
				ret = print_ps(gui->drawing, param, sel_file);
			/* verify success or fail*/
			if (ret)
				snprintf(gui->log_msg, 63, "Print: Created print output succesfully");
			else
				snprintf(gui->log_msg, 63, "Print Error");
		}
		/* draw print preview image */
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
			
			/* clear entire image with its background color*/
			bmp_fill(gui->preview_img, gui->preview_img->bkg);
			
			/* set image clip for paper area */
			gui->preview_img->clip_x = (unsigned int)x;
			gui->preview_img->clip_y = (unsigned int)y;
			gui->preview_img->clip_w = (unsigned int)w;
			gui->preview_img->clip_h = (unsigned int)h;
			/* clear image on page bounds with white color */
			bmp_fill_clip(gui->preview_img, white);
			
			/* draw image */
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
		
		/* update parameters for next iteration */
		prev_param.scale = param.scale;
		prev_param.ofs_x = param.ofs_x;
		prev_param.ofs_y = param.ofs_y;
		prev_param.w = param.w;
		prev_param.h = param.h;
		prev_param.mono = param.mono;
		prev_param.unit = param.unit;
		prev_param.list = param.list;
		prev_param.subst = param.subst;
		prev_param.len = param.len;
		prev_param.out_fmt = param.out_fmt;
		
	}
	else{
		show_print = 0; /* close window*/
		/* clear preview image */
		bmp_fill(gui->preview_img, gui->preview_img->bkg);
	}
	nk_end(gui->ctx);
	
	return show_print;
}