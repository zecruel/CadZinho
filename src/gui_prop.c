#include "gui_use.h"

int gui_prop_interactive(gui_obj *gui){
	if (gui->modal != PROP) return 0;
	
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
		gui->sel_ent_filter = ~DXF_NONE;
		gui_simple_select(gui);
	}
	else if (gui->step >= 1){
		
		/* user cancel operation */
		if (gui->ev & EV_CANCEL){
			gui->element = NULL;
			gui_default_modal(gui);
		}
	}
	return 1;
}

int gui_prop_info (gui_obj *gui){
	if (gui->modal != PROP) return 0;
	static int init = 0;
	static dxf_node *new_ent = NULL;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Edit Properties", NK_TEXT_LEFT);
	if (gui->step == 0){ /* get insert element to edit */
		nk_label(gui->ctx, "Select a element", NK_TEXT_LEFT);
		init = 0;
	}
	else if (gui->step == 1){ /* init */
		init = 0;
		new_ent = NULL;
		gui->step = 2;
	}
	else { /* all inited */
		nk_label(gui->ctx, "Edit data", NK_TEXT_LEFT);
		
		static dxf_node *ent = NULL;
		static char layer[DXF_MAX_CHARS+1];
		static char ltype[DXF_MAX_CHARS+1];
		static int color;
		dxf_node *tmp;
		
		
		int i, j, h;
		static int show_color_pick = 0, sel_color = -1;
		
		/* init the interface */
		if (!init){
			ent = gui->sel_list->next->data;
			
			layer[0] = 0;
			ltype[0] = 0;
			color = 256;
			
			if(tmp = dxf_find_attr2(ent, 8))
				strncpy (layer, tmp->value.s_data, DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(ent, 6))
				strncpy (ltype, tmp->value.s_data, DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(ent, 62))
				color = tmp->value.i_data;
			
				
			init = 1; /* init success */
			
		}
		else{
			nk_layout_row_dynamic(gui->ctx, 20, 2);
			nk_label(gui->ctx, "Layer:", NK_TEXT_LEFT);
			
			int sel_layer = -1;
			/* line pattern */
			dxf_layer *layers = gui->drawing->layers;
			int num_layers = gui->drawing->num_layers;
			h = num_layers * 25 + 5;
			h = (h < 200)? h : 200;
			
			if (nk_combo_begin_label(gui->ctx, layer, nk_vec2(200, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				
				/* show available line patterns */
				for (j = 0; j < num_layers; j++){
					
					/* skip unauthorized layers */
					if (strlen(layers[j].name) == 0) continue;
					
					/* line type name*/
					if (nk_button_label(gui->ctx, layers[j].name)){ /* change layer*/
						sel_layer = j;
						nk_combo_close(gui->ctx);
					}
				}
				nk_combo_end(gui->ctx);
			}
			
			
			nk_label(gui->ctx, "Ltype:", NK_TEXT_LEFT);
			
			int sel_ltype = -1;
			/* line pattern */
			dxf_ltype *ltypes = gui->drawing->ltypes;
			int num_ltypes = gui->drawing->num_ltypes;
			h = num_ltypes * 25 + 5;
			h = (h < 200)? h : 200;
			
			if (nk_combo_begin_label(gui->ctx, ltype, nk_vec2(300, h))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				
				/* show available line patterns */
				for (j = 0; j < num_ltypes; j++){
					
					/* skip unauthorized line types*/
					if (strlen(ltypes[j].name) == 0) continue;
					
					/* line type name*/
					if (nk_button_label(gui->ctx, ltypes[j].name)){ /* change line type*/
						sel_ltype = j;
						nk_combo_close(gui->ctx);
					}
					/* show line type short description */
					nk_label(gui->ctx, ltypes[j].descr, NK_TEXT_LEFT);
				}
				nk_combo_end(gui->ctx);
			}
			
			nk_label(gui->ctx, "Color:", NK_TEXT_LEFT);
			
			int curr_color = abs(color);
			
			if (curr_color > 256) curr_color = 0;
			
			struct nk_color b_color = {
				.r = dxf_colors[curr_color].r,
				.g = dxf_colors[curr_color].g,
				.b = dxf_colors[curr_color].b,
				.a = dxf_colors[curr_color].a
			};
			if(nk_button_color(gui->ctx, b_color)){
				/* to change color, open a popup */
				show_color_pick = 1;
				
			}
			
		}
		
		/* popup to change color */
		if (show_color_pick){
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Choose Color", NK_WINDOW_CLOSABLE, nk_rect(220, 10, 220, 300))){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "By Layer")){
					sel_color = 256;
					nk_popup_close(gui->ctx);
					show_color_pick = 0;
				}
				if (nk_button_label(gui->ctx, "By Block")){
					sel_color = 0;
					nk_popup_close(gui->ctx);
					show_color_pick = 0;
				}
				nk_layout_row_static(gui->ctx, 15, 15, 10);
				struct nk_color b_color;
				nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
				/* show available colors */
				for (j = 1; j < 256; j++){
					
					b_color.r = dxf_colors[j].r;
					b_color.g = dxf_colors[j].g;
					b_color.b = dxf_colors[j].b;
					b_color.a = dxf_colors[j].a;
					
					if(nk_button_color(gui->ctx, b_color)){ /* pick color */
						/* change layer color */
						sel_color = j;
						nk_popup_close(gui->ctx);
						show_color_pick = 0;
						break;
					}
				}
				nk_popup_end(gui->ctx);
			} else show_color_pick = 0;
		}
		
		
	}

	return 1;
}