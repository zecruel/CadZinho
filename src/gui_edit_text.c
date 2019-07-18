#include "gui_use.h"

int gui_ed_text_interactive(gui_obj *gui){
	if (gui->modal == ED_TEXT){
		static dxf_node *text_el = NULL, *x = NULL;
		enum dxf_graph ent_type = DXF_NONE;
		//static char text[DXF_MAX_CHARS];
		
		int i = 0;
		
		gui->element = text_el;
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				ent_type = dxf_ident_ent_type (gui->near_el);
				if (ent_type == DXF_TEXT || ent_type == DXF_MTEXT){
					text_el = gui->near_el;
					gui->draw = 1;
					//gui->draw_tmp = 1;
					gui->show_edit_text = 1;
					
					nk_str_clear(&gui->text_edit.string);
					for (i = 0; x = dxf_find_attr_i(text_el, 3, i); i++){
						//text[0] = 0;
						//strncpy(text, "teste", DXF_MAX_CHARS - 1);
						nk_str_append_str_char(&gui->text_edit.string, x->value.s_data);
					}
					
					for (i = 0; x = dxf_find_attr_i(text_el, 1, i); i++){
						//text[0] = 0;
						//strncpy(text, "teste", DXF_MAX_CHARS - 1);
						nk_str_append_str_char(&gui->text_edit.string, x->value.s_data);
					}
					
					
					
					
					gui->step_x[gui->step + 1] = gui->step_x[gui->step];
					gui->step_y[gui->step + 1] = gui->step_y[gui->step];
					gui->step = 1;
					gui->step_x[gui->step + 1] = gui->step_x[gui->step];
					gui->step_y[gui->step + 1] = gui->step_y[gui->step];
					gui_next_step(gui);
				}
			}
			else if (gui->ev & EV_CANCEL){
				gui_default_modal(gui);
			}
			else if (gui->ev & EV_MOTION){
				ent_type = dxf_ident_ent_type (gui->near_el);
				if (ent_type == DXF_TEXT || ent_type == DXF_MTEXT){
					gui->element = gui->near_el;
					gui->draw = 1;
				}
				else gui->element = NULL;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				char *blank = "";
				char *text = nk_str_get(&(gui->text_edit.string));
				int len = nk_str_len_char(&(gui->text_edit.string));
				if (!text) text = blank;
				mtext_change_text (text_el, text, len, DWG_LIFE);
				text_el->obj.graphics = dxf_graph_parse(gui->drawing, text_el, 0 , 0);
				gui_first_step(gui);
			}
			else if (gui->ev & EV_CANCEL){
				gui_first_step(gui);
			}
			
		}
	}
	return 1;
}

int gui_ed_text_info (gui_obj *gui){
	if (gui->modal == ED_TEXT) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Edit a text", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Select a text element", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter a new text", NK_TEXT_LEFT);
				if (gui->show_edit_text){
				
				static struct nk_rect s = {150, 10, 420, 330};
				if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Edit text", NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE, s)){
					
					nk_layout_row_dynamic(gui->ctx, 270, 1);
					
					//nk_edit_buffer(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
					nk_edit_buffer_wrap(gui->ctx, NK_EDIT_BOX, &(gui->text_edit), nk_filter_default);
					
					nk_popup_end(gui->ctx);
				}
				else {
					gui->show_edit_text = 0;
					gui_first_step(gui);
				}
			}
		}
	}
	return 1;
}