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
    /* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
			gui->step = 0;
		}
	}
	else if (gui->step >= 1){
		/* verify if selected insert have attributes */
		if (dxf_find_obj2(gui->sel_list->next->data, "ATTRIB"))
			gui->element = gui->sel_list->next->data;
		else{
			snprintf(gui->log_msg, 63, _l("Error: No attributes found"));
			gui->step = 0;
			gui->element = NULL;
			sel_list_clear (gui);
		}
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
		}
	}
	return 1;
}

int gui_ed_attr_info (gui_obj *gui){
	if (gui->modal == ED_ATTR) {
		static int init = 0;
		static dxf_node *new_ent = NULL;
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, _l("Edit Attributes"), NK_TEXT_LEFT);
		if (gui->step == 0){ /* get insert element to edit */
			nk_label(gui->ctx, _l("Select a Insert element"), NK_TEXT_LEFT);
			init = 0;
		}
		else if (gui->step == 1){ /* init */
			init = 0;
			new_ent = NULL;
			gui->step = 2;
		}
		else { /* all inited */
			nk_label(gui->ctx, _l("Edit data"), NK_TEXT_LEFT);
			
			static dxf_node *ins_ent = NULL, *attr = NULL;
			static dxf_node *attributes[1000];
			static int num_attr = 0;
			static char tag[1000][DXF_MAX_CHARS+1];
			static char value[1000][DXF_MAX_CHARS+1];
			static int hidden[1000];
			dxf_node *tmp;
			char *new_str;
			
			int i;
			
			static char blk_name[DXF_MAX_CHARS+1];
			
			if (!new_ent){
				/* copy original insert entity */
				ins_ent = gui->element;
				new_ent = dxf_ent_copy(ins_ent, DWG_LIFE);
			}
			
			/* init the interface */
			if (!init){
				
				blk_name[0] = 0;
				if (new_ent){
					/* get block name */
					if(tmp = dxf_find_attr2(new_ent, 2))
						strncpy (blk_name,
              strpool_cstr2( &name_pool, tmp->value.str), DXF_MAX_CHARS);
					/* find attibutes */
					num_attr = 0;
					while ((attr = dxf_find_obj_i(new_ent, "ATTRIB", num_attr)) && num_attr < 999){
						/* construct tables with each attribute informations */
						attributes[num_attr] = attr;
						tag[num_attr][0] = 0;
						value[num_attr][0] = 0;
						hidden[num_attr] = 0;
						if(tmp = dxf_find_attr2(attr, 2))
							strncpy(tag[num_attr],
                strpool_cstr2( &name_pool, tmp->value.str), DXF_MAX_CHARS);
						if(tmp = dxf_find_attr2(attr, 1))
							strncpy(value[num_attr],
                strpool_cstr2( &value_pool, tmp->value.str), DXF_MAX_CHARS);
						if(tmp = dxf_find_attr2(attr, 70))
							hidden[num_attr] = tmp->value.i_data & 1;
						
						num_attr++;
					}
					
					init = 1; /* init success */
				}
				else{
					/* init fail -  no block found */
					gui->element = NULL;
					init = 0;
					gui->step = 0;
					new_ent = NULL;
					sel_list_clear (gui);
				}
			}
			
			if (init){
				/* edit attributes window */
				static struct nk_rect s = {150, -50, 420, 350};
				if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, _l("Edit Insert Attributes"), NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE, s)){
					
					/* show refered block name */
					nk_layout_row_template_begin(gui->ctx, 20);
					nk_layout_row_template_push_static(gui->ctx, 50);
					nk_layout_row_template_push_dynamic(gui->ctx);
					nk_layout_row_template_end(gui->ctx);
					nk_label(gui->ctx, _l("Block:"), NK_TEXT_RIGHT);
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
							nk_layout_row_template_push_static(gui->ctx, 20);
							nk_layout_row_template_push_static(gui->ctx, 8);
							nk_layout_row_template_end(gui->ctx);
							
							if (nk_button_label(gui->ctx, _l("Tag"))){
								
							}
							if (nk_button_label(gui->ctx, _l("Value"))){
								
							}
							if (nk_button_label(gui->ctx, _l("Hide"))){
								
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
							nk_layout_row_template_push_static(gui->ctx, 20);
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
								/* delete current attribute */
								if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->i_trash))){
									dxf_obj_subst(attributes[i], NULL);
									init = 0; /* reinit the list */
								}
								
							}
							nk_group_end(gui->ctx);
						}
						nk_group_end(gui->ctx);
					}
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					if (nk_button_label(gui->ctx, _l("OK"))){
						/* update changes in insert */
						for (i = 0; i < num_attr; i++){
							new_str = trimwhitespace(tag[i]);
							/* verify if tags contain spaces */
							if (strchr(new_str, ' ')){
								snprintf(gui->log_msg, 63, "Error: No spaces allowed in tags");
								continue; /* skip change */
							}
							attr = dxf_find_obj_i(new_ent, _l("ATTRIB"), i);
							
							/* update tag */
							dxf_attr_change(attr, 2, new_str);
							/* update value */
							new_str = trimwhitespace(value[i]);
							dxf_attr_change(attr, 1, new_str);
							/* update hide flag */
							dxf_attr_change(attr, 70, &hidden[i]);
							
						}
						if (num_attr == 0){
							dxf_attr_change(ins_ent, 66, (int[]){0});
						}
						/* update insert entity */
						new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
						dxf_obj_subst(ins_ent, new_ent);
						/* add to undo/redo list */
						do_add_entry(&gui->list_do, "Edit Insert Attributes");
						do_add_item(gui->list_do.current, ins_ent, new_ent);
						
						gui->draw = 1;
						
						
						/* close edit window */
						gui->element = NULL;
						init = 0;
						gui->step = 0;
						new_ent = NULL;
						sel_list_clear (gui);
					}
					if (nk_button_label(gui->ctx, _l("Cancel"))){
						/* close edit window */
						init = 0;
						new_ent = NULL;
						gui->step = 0;
						sel_list_clear (gui);
					}
					
				} else {
					/* close edit window */
					init = 0;
					gui->step = 0;
					new_ent = NULL;
					sel_list_clear (gui);
				}
				nk_popup_end(gui->ctx);
			}
			
			
		}
	}
	return 1;
}