#include "gui_export.h"

int export_win (gui_obj *gui){
	int show_export = 1;
	static double ofs_x = 0.0, ofs_y = 0.0, scale = 1.0;
	static char ofs_x_str[64] = "0.00";
	static char ofs_y_str[64] = "0.00";
	static char scale_str[64] = "1.00";
	static char sel_file[PATH_MAX_CHARS] = "output.plt";
	static char tmp_str[64];
	static int out_fmt = EXPORT_HPGL;
	
	static double gcode_feed = 1.0;
	static char gcode_init[DXF_MAX_CHARS+1] = "G21 G90 S1600 M03 G00 z1.0";
	static char gcode_end[DXF_MAX_CHARS+1] = "G00 z1.0 M05";
	static char gcode_move[DXF_MAX_CHARS+1] = "G00 z1.0";
	static char gcode_stroke[DXF_MAX_CHARS+1] = "G00 z-1.0";
	static char gcode_feed_str[64] = "1.00";
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 500;
	gui->next_win_h = 340;
	
	/* export window */
	if (nk_begin(gui->ctx, "Export", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_flags res;
		static int show_app_file = 0;
		int i, ret = 0;
		static struct export_param param;
		
		/* supported output formats */
		static const char *ext_type[] = {
			[EXPORT_HPGL] = "PLT",
			[EXPORT_GCODE] = "NC",
			[EXPORT_NONE] = "*"
		};
    
    static char ext_descr[3][DXF_MAX_CHARS + 1];
    strncpy(ext_descr[0], _l("HP-GL files (.plt)"), DXF_MAX_CHARS);
    strncpy(ext_descr[1], _l("G-Code files (.nc)"), DXF_MAX_CHARS);
    strncpy(ext_descr[2], _l("All files (*)"), DXF_MAX_CHARS);
		char *ext_descr_addr[] = {
			[EXPORT_HPGL] = ext_descr[0],
			[EXPORT_GCODE] = ext_descr[1],
			[EXPORT_NONE] = ext_descr[2]
		};
				
		/* update structure parameters */
		param.scale = scale;
		param.ofs_x = ofs_x;
		param.ofs_y = ofs_y;
		
		nk_layout_row(gui->ctx, NK_STATIC, 180, 2, (float[]){160, 310});
		if (nk_group_begin(gui->ctx, _l("Position and scale"), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			
			/* adjust drawing position and scale factor */
			nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){60, 70});
			
			/* customize x axis origin of export area (left lower corner) in drawing */
			nk_label(gui->ctx, _l("Origin X:"), NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_x_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_x_str)) /* update parameter value */
					ofs_x = atof(ofs_x_str);
				snprintf(ofs_x_str, 63, "%.9g", ofs_x);
			}
			
			/* customize x axis origin of export area (left lower corner) in drawing */
			nk_label(gui->ctx, _l("Origin Y:"), NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, ofs_y_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(ofs_y_str)) /* update parameter value */
					ofs_y = atof(ofs_y_str);
				snprintf(ofs_y_str, 63, "%.9g", ofs_y);
			}
			
			/* customize drawing scale in export area ( [drawing unit]/[export unit] factor) */
			nk_label(gui->ctx, _l("Scale:"), NK_TEXT_RIGHT);
			res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, scale_str, 63, nk_filter_float);
			if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
				nk_edit_unfocus(gui->ctx);
				if (strlen(scale_str)) /* update parameter value */
					scale = atof(scale_str);
				snprintf(scale_str, 63, "%.9g", scale);
			}
			nk_group_end(gui->ctx);
		}
		if (nk_group_begin(gui->ctx, _l("Driver specific"), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
			if (out_fmt == EXPORT_GCODE){
				nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){50, 220});
				/* feed rate */
				nk_label(gui->ctx, _l("Feed:"), NK_TEXT_RIGHT);
				res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, gcode_feed_str, 63, nk_filter_float);
				if ((res & NK_EDIT_DEACTIVATED) || (res & NK_EDIT_COMMITED)){ /* probably, user change parameter string */
					nk_edit_unfocus(gui->ctx);
					if (strlen(gcode_feed_str)) /* update parameter value */
						gcode_feed = atof(gcode_feed_str);
					snprintf(gcode_feed_str, 63, "%.9g", gcode_feed);
				}
				
				/* init  command*/
				nk_label(gui->ctx, _l("Init:"), NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, gcode_init, DXF_MAX_CHARS, nk_filter_default);
				
				/* end command */
				nk_label(gui->ctx, _l("End:"), NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, gcode_end, DXF_MAX_CHARS, nk_filter_default);
				
				/* command before move */
				nk_label(gui->ctx, _l("Move:"), NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, gcode_move, DXF_MAX_CHARS, nk_filter_default);
				
				/* command before stroke */
				nk_label(gui->ctx, _l("Stroke:"), NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, gcode_stroke, DXF_MAX_CHARS, nk_filter_default);
			}
			nk_group_end(gui->ctx);
		}
		
		nk_layout_row_dynamic(gui->ctx, 10, 1);
		
		/* choose output file format */
		nk_layout_row(gui->ctx, NK_STATIC, 20, 2, (float[]){120, 270});
		nk_label(gui->ctx, _l("Output format:"), NK_TEXT_RIGHT);
		/* combo for file extension filter */
		int h = (EXPORT_SIZE - 1) * 22 + 5;
		h = (h < 300)? h : 300;
		int fmt = nk_combo (gui->ctx, (const char **)ext_descr_addr, EXPORT_SIZE - 1, out_fmt, 17, nk_vec2(270, h));
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
		nk_label(gui->ctx, _l("Destination:"), NK_TEXT_LEFT);
		nk_layout_row_template_begin(gui->ctx, 20);
		nk_layout_row_template_push_static(gui->ctx, 80);
		nk_layout_row_template_push_dynamic(gui->ctx);
		nk_layout_row_template_end(gui->ctx);
		if (nk_button_label(gui->ctx, _l("Browse"))){/* call file browser */
			show_app_file = 1;
			/* set filter for suported output formats */
			for (i = 0; i < EXPORT_SIZE; i++){
				gui->file_filter_types[i] = ext_type[i];
				gui->file_filter_descr[i] = ext_descr[i];
			}
			gui->file_filter_count = EXPORT_SIZE;
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
				if (out_fmt >= EXPORT_SIZE - 1) out_fmt = 0;
			}
		} /* manual entry to output path */
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, sel_file, PATH_MAX_CHARS, nk_filter_default);
		
		/* export command */
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, _l("Export"))){
			snprintf(gui->log_msg, 63, " ");
			/* call corresponding  function, based on output format */
			if (out_fmt == EXPORT_HPGL)
				ret = export_hpgl(gui->drawing, param, sel_file);
			else if (out_fmt == EXPORT_GCODE){
				param.feed = gcode_feed;
				strncpy(param.init, gcode_init, DXF_MAX_CHARS);
				strncpy(param.move, gcode_move, DXF_MAX_CHARS);
				strncpy(param.stroke, gcode_stroke, DXF_MAX_CHARS);
				strncpy(param.end, gcode_end, DXF_MAX_CHARS);
				ret = export_gcode(gui->drawing, param, sel_file);
			}
			/* verify success or fail*/
			if (ret)
				snprintf(gui->log_msg, 63, _l("Export: Created export output succesfully"));
			else
				snprintf(gui->log_msg, 63, _l("Export Error"));
		}
		
	}
	else{
		show_export = 0; /* close window*/
	}
	nk_end(gui->ctx);
	
	return show_export;
}