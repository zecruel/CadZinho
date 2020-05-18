#include "gui_use.h"

int gui_find_interactive(gui_obj *gui){
	if (gui->modal != FIND) return 0;
	
	if (gui->ev & EV_CANCEL){
		gui->step = 0;
		gui_default_modal(gui);
	}
	
	return 1;
}

int gui_find_info (gui_obj *gui){
	if (gui->modal != FIND) return 0;
	
	static char search[DXF_MAX_CHARS+1] = "";
	static char repl[DXF_MAX_CHARS+1] = "";
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Find/Replace text", NK_TEXT_LEFT);
	
	nk_label(gui->ctx, "Search:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, search, DXF_MAX_CHARS, nk_filter_default);
	
	nk_label(gui->ctx, "Replace:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, repl, DXF_MAX_CHARS, nk_filter_default);
	
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	if (nk_button_label(gui->ctx, "Next")){
		
	}
	if (nk_button_label(gui->ctx, "Replace")){
		
	}
	if (nk_button_label(gui->ctx, "Selection")){
		
	}
	
	return 1;
}