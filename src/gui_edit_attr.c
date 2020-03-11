#include "gui_use.h"

int gui_ed_attr_interactive(gui_obj *gui){
	if (gui->modal != ED_ATTR) return 0;
	
	static dxf_node *attr_el = NULL, *x = NULL;
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
		gui->sel_ent_filter = DXF_INSERT;
		gui_simple_select(gui);
	}
	else if (gui->step == 1){
		gui->element = gui->sel_list->next->data;
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
		}
	}
	return 1;
}

int gui_ed_attr_info (gui_obj *gui){
	if (gui->modal == ED_ATTR) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Edit Attributes", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Select a Insert element", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Edit data and hit OK", NK_TEXT_LEFT);
			
			static dxf_node *ins_ent = NULL, *attr = NULL;
			static int init = 0;
			static int num_attr = 0;
			static char tag[1000][DXF_MAX_CHARS+1];
			static char value[1000][DXF_MAX_CHARS+1];
			static int hidden[1000];
			dxf_node *tmp, *new_ent = NULL;
			char *new_str;
			
			int i;
			
			static char blk_name[DXF_MAX_CHARS+1];
			
			/* init the interface */
			if (!init){
				ins_ent = gui->element;
				blk_name[0] = 0;
				if (ins_ent){
					/* get block name */
					if(tmp = dxf_find_attr2(ins_ent, 2))
						strncpy (blk_name, tmp->value.s_data, DXF_MAX_CHARS);
					/* find attibutes */
					num_attr = 0;
					while ((attr = dxf_find_obj_i(ins_ent, "ATTRIB", num_attr)) && num_attr < 999){
						/* construct tables with each attribute informations */
						tag[num_attr][0] = 0;
						value[num_attr][0] = 0;
						hidden[num_attr] = 0;
						if(tmp = dxf_find_attr2(attr, 2))
							strncpy(tag[num_attr], tmp->value.s_data, DXF_MAX_CHARS);
						if(tmp = dxf_find_attr2(attr, 1))
							strncpy(value[num_attr], tmp->value.s_data, DXF_MAX_CHARS);
						if(tmp = dxf_find_attr2(attr, 70))
							hidden[num_attr] = tmp->value.i_data & 1;
						
						num_attr++;
					}
					
					if(num_attr) init = 1; /* init success */
					else { /* init fail -  no attributes found */
						gui->step = 0;
						snprintf(gui->log_msg, 63, "Error: No attributes found");
						sel_list_clear (gui);
					}
				}
				else{
					gui->step = 0; /* init fail -  no block found */
					sel_list_clear (gui);
				}
			}
			
			if (init){
				/* edit attributes window */
				static struct nk_rect s = {150, 10, 420, 350};
				if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Edit Attributes", NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE, s)){
					
					nk_layout_row_template_begin(gui->ctx, 20);
					nk_layout_row_template_push_static(gui->ctx, 50);
					nk_layout_row_template_push_dynamic(gui->ctx);
					nk_layout_row_template_end(gui->ctx);
					
					nk_label(gui->ctx, "Block:", NK_TEXT_RIGHT);
					nk_label_colored(gui->ctx, blk_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
					
					nk_layout_row_dynamic(gui->ctx, 250, 1);
					if (nk_group_begin(gui->ctx, "Attr_list", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
						/* show header */
						nk_layout_row_dynamic(gui->ctx, 32, 1);
						if (nk_group_begin(gui->ctx, "Attr_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
							/* dynamic width for Attribute tag and value. Fixed width for hide flags */
							nk_layout_row_template_begin(gui->ctx, 22);
							nk_layout_row_template_push_dynamic(gui->ctx);
							nk_layout_row_template_push_dynamic(gui->ctx);
							nk_layout_row_template_push_static(gui->ctx, 50);
							nk_layout_row_template_push_static(gui->ctx, 8);
							nk_layout_row_template_end(gui->ctx);
							
							if (nk_button_label(gui->ctx, "Tag")){
								
							}
							if (nk_button_label(gui->ctx, "Value")){
								
							}
							if (nk_button_label(gui->ctx, "Hide")){
								
							}
							nk_group_end(gui->ctx);
						}
						/* show attributes list */
						nk_layout_row_dynamic(gui->ctx, 200, 1);
						if (nk_group_begin(gui->ctx, "Attr_names", NK_WINDOW_BORDER)) {
							/* dynamic width for Attribute tag and value. Fixed width for hide flags */
							nk_layout_row_template_begin(gui->ctx, 20);
							nk_layout_row_template_push_dynamic(gui->ctx);
							nk_layout_row_template_push_dynamic(gui->ctx);
							nk_layout_row_template_push_static(gui->ctx, 50);
							nk_layout_row_template_end(gui->ctx);
							
							for (i = 0; i < num_attr; i++){
								/* tag */
								nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, tag[i], DXF_MAX_CHARS, nk_filter_default);
								/* value */
								nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, value[i], DXF_MAX_CHARS, nk_filter_default);
								/* hide flag */
								if (hidden[i]){
									if (nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, "X"))
										hidden[i] = 0;
								} else{
									if (nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, " "))
										hidden[i] = 1;
								}
							}
							nk_group_end(gui->ctx);
						}
						nk_group_end(gui->ctx);
					}
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					if (nk_button_label(gui->ctx, "OK")){
						/* update changes in attributes */
						int init_do = 0;
						for (i = 0; i < num_attr; i++){
							new_str = trimwhitespace(tag[i]);
							/* verify if tags contain spaces */
							if (strchr(new_str, ' ')){
								snprintf(gui->log_msg, 63, "Error: No spaces allowed in tags");
								continue; /* skip change */
							}
							
							if (!init_do){
								new_ent = dxf_ent_copy(ins_ent, DWG_LIFE); /* copy original insert entity */
								init_do = 1;
							}
							
							attr = dxf_find_obj_i(new_ent, "ATTRIB", i);
							
							/* update tag */
							dxf_attr_change(attr, 2, new_str);
							/* update value */
							new_str = trimwhitespace(value[i]);
							dxf_attr_change(attr, 1, new_str);
							/* update hide flag */
							dxf_attr_change(attr, 70, &hidden[i]);
							
						}
						if (init_do){
							new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
							dxf_obj_subst(ins_ent, new_ent);
							
							/* update undo/redo list */
							do_add_entry(&gui->list_do, "Edit Insert Attributes");
							do_add_item(gui->list_do.current, ins_ent, new_ent);
						}
						
						//blk_idx = 1; /* update block preview */
						/* close edit window */
						init = 0;
						gui->step = 0;
						sel_list_clear (gui);
					}
					if (nk_button_label(gui->ctx, "Cancel")){
						/* close edit window */
						init = 0;
						gui->step = 0;
						sel_list_clear (gui);
					}
					
				} else {
					/* close edit window */
					init = 0;
					gui->step = 0;
					sel_list_clear (gui);
				}
				nk_popup_end(gui->ctx);
			}
			
			
		}
	}
	return 1;
}