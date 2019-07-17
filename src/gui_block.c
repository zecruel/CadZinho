#include "gui_use.h"

int gui_block_interactive(gui_obj *gui){
	if (gui->modal == NEW_BLK){
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				/* verify if text is valid */
				if (strlen(gui->txt) > 0){
					if(!gui->text2tag) dxf_new_block(gui->drawing, gui->txt, "0", gui->sel_list, &gui->list_do, DWG_LIFE);
					
					else dxf_new_block2(gui->drawing, gui->txt, gui->tag_mark, "0", gui->sel_list, &gui->list_do, DWG_LIFE);
				}
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
		}
	}
	
	return 1;
}

int gui_block_info (gui_obj *gui){
	if (gui->modal == NEW_BLK) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Create a new block from selection", NK_TEXT_LEFT);
		//nk_label(gui->ctx, "Enter base point", NK_TEXT_LEFT);
		nk_label(gui->ctx, "Block Name:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->txt, DXF_MAX_CHARS, nk_filter_default);
		nk_checkbox_label(gui->ctx, "Text to Tags", &gui->text2tag);
		nk_label(gui->ctx, "Tag mark:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, gui->tag_mark, DXF_MAX_CHARS, nk_filter_default);
		
	}
	return 1;
}