#include "gui_use.h"

int gui_ed_text_interactive(gui_obj *gui){
	if (gui->modal != ED_TEXT) return 0;
	static dxf_node *text_el = NULL, *x = NULL;
	enum dxf_graph ent_type = DXF_NONE;
	dxf_node *new_ent = NULL;
	
	int i = 0;
	
	if (gui->step == 0) {
		/* try to go to next step */
		gui->step = 1;
		gui->free_sel = 1;
	}
	/* verify if elements in selection list */
	if (gui->step == 1 && (!gui->sel_list->next)){
		/* if selection list is empty, back to first step */
		gui->step = 0;
		gui->free_sel = 1;
	}
	if (gui->step == 0){
		/* in first step, select the elements to proccess*/
		gui->en_distance = 0;
		gui->sel_ent_filter = DXF_TEXT | DXF_MTEXT;
		gui_simple_select(gui);
    
    /* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
			gui->step = 0;
		}
	}
	else if (gui->step == 1){
		/* get selected element */
		text_el = gui->sel_list->next->data;
		gui->element = text_el;
		gui->draw = 1;
		
		/* init the edit string with DXF entity text */
		nk_str_clear(&gui->text_edit.string);
		for (i = 0; x = dxf_find_attr_i(text_el, 3, i); i++){
			/* first, get the additional text (MTEXT ent) */
			nk_str_append_str_char(&gui->text_edit.string, x->value.s_data);
		}
		for (i = 0; x = dxf_find_attr_i(text_el, 1, i); i++){
			/* finally, get main text */
			nk_str_append_str_char(&gui->text_edit.string, x->value.s_data);
		}
		
		gui->step = 2;
		gui_next_step(gui);
	}
	else{
		if (gui->ev & EV_ENTER){
			new_ent = dxf_ent_copy(text_el, DWG_LIFE); /* copy original entity */
			if (new_ent){
				/* get edited text */
				char *blank = "";
				char *text = nk_str_get(&(gui->text_edit.string));
				int len = nk_str_len_char(&(gui->text_edit.string));
				if (!text) text = blank;
				
				ent_type = dxf_ident_ent_type (new_ent);
				
				/* replace the text */
				if (ent_type == DXF_MTEXT)
					mtext_change_text (new_ent, text, len, DWG_LIFE);
				else if (ent_type == DXF_TEXT){
					for (i = 0; x = dxf_find_attr_i(new_ent, 1, i); i++){
						/* limit of TEXT entity length */
						len = (len < DXF_MAX_CHARS - 1)? len : DXF_MAX_CHARS - 1;
						strncpy(x->value.s_data, text, len);
						x->value.s_data[len] = 0; /* terminate string */
					}
				}
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
				dxf_obj_subst(text_el, new_ent);
				
				/* update undo/redo list */
				do_add_entry(&gui->list_do, _l("EDIT TEXT"));
				do_add_item(gui->list_do.current, text_el, new_ent);
			}
			gui->element = NULL;
			gui->step = 0;
			sel_list_clear (gui);
			gui_first_step(gui);
		}
		/* user cancel operation */
		else if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui->step = 0;
			sel_list_clear (gui);
			gui_first_step(gui);
		}
	}
	return 1;
}

int gui_ed_text_info (gui_obj *gui){
	if (gui->modal == ED_TEXT) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Edit a text"), NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, _l("Select a text element"), NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, _l("Enter a new text"), NK_TEXT_LEFT);
				
			static struct nk_rect s = {150, 10, 420, 330};
			/* edit text window */
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, _l("Edit text"), NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE, s)){
				nk_layout_row_dynamic(gui->ctx, 270, 1);
				/* big edit box */
				//nk_edit_buffer(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
				nk_edit_buffer_wrap(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
				nk_popup_end(gui->ctx);
			}
			else { /* user close window */
				gui->step = 0;
				sel_list_clear (gui);
				gui_first_step(gui);
			}
			
		}
	}
	return 1;
}