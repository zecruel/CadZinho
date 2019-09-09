#include "gui.h"

int script_win (gui_obj *gui){
	int show_script = 1;
	int i = 0;
	
	static int init = 0;
	
	if (!init){
		nk_str_clear(&gui->debug_edit.string);
		init = 1;
	}
	
	if (nk_begin(gui->ctx, "Script", nk_rect(gui->win_w - 404, gui->win_h - 350, 400, 250),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Script file:", NK_TEXT_LEFT);
		
		/* user can type the file name/path, or paste text, or drop from system navigator */
		//nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_script, MAX_PATH_LEN, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Run")){
			nk_str_append_str_char(&gui->debug_edit.string, "click\n");
		}
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_label(gui->ctx, "Output:", NK_TEXT_LEFT);
		if (nk_button_label(gui->ctx, "Clear")){
			nk_str_clear(&gui->debug_edit.string);
		}
		nk_layout_row_dynamic(gui->ctx, 100, 1);
		nk_edit_buffer_wrap(gui->ctx, NK_EDIT_EDITOR, &(gui->debug_edit), nk_filter_default);
		
	} else {
		show_script = 0;
		//init = 0;
	}
	nk_end(gui->ctx);
	
	return show_script;
}