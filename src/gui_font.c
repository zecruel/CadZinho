#include "gui.h"

int fonts_mng (gui_obj *gui){
	int i, show_font_mng = 1;
	static int show_color_pick = 0, show_font_name = 0;
	static int sel_t_sty, t_sty_idx;
	
	static struct sort_by_idx sort_t_sty[DXF_MAX_LAYERS];
	dxf_tfont *t_sty = gui->drawing->text_fonts;
	
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 810;
	gui->next_win_h = 310;
	
	enum font_op {
		FONT_OP_NONE,
		FONT_OP_CREATE,
		FONT_OP_RENAME,
		FONT_OP_UPDATE
	};
	static int font_change = FONT_OP_UPDATE;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Info", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, "Font Manager", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		t_sty = gui->drawing->text_fonts;
		
		nk_layout_row_dynamic(gui->ctx, 32, 1);
		if (nk_group_begin(gui->ctx, "font_head", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row(gui->ctx, NK_STATIC, 22, 8, (float[]){175, 175, 175, 25, 25, 50, 50, 50});
			if (nk_button_label(gui->ctx, "Name")){
				
			}
			if (nk_button_label(gui->ctx, "File")){
				
			}
			if (nk_button_label(gui->ctx, "Subst")){
				
			}
			if (nk_button_label(gui->ctx, "F1")){
				
			}
			if (nk_button_label(gui->ctx, "F2")){
				
			}
			if (nk_button_label(gui->ctx, "W")){
				
			}
			if (nk_button_label(gui->ctx, "FH")){
				
			}
			if (nk_button_label(gui->ctx, "A")){
				
			}
			
			nk_group_end(gui->ctx);
		}
		nk_layout_row_dynamic(gui->ctx, 200, 1);
		if (nk_group_begin(gui->ctx, "font_prop", NK_WINDOW_BORDER)) {
			
			nk_layout_row(gui->ctx, NK_STATIC, 22, 8, (float[]){175, 175, 175, 25, 25, 50, 50, 50});
			int num_fonts = gui->drawing->num_fonts;
			char txt[DXF_MAX_CHARS];
				
			for (i = 0; i < num_fonts; i++){
				
				//t_sty_idx = sort_t_sty[i].idx;
				t_sty_idx = i;
				if (sel_t_sty == t_sty_idx){
					if (nk_button_label_styled(gui->ctx, &gui->b_icon_sel, t_sty[t_sty_idx].name)){
						sel_t_sty = -1;
					}
				}
				else {
					if (nk_button_label_styled(gui->ctx,&gui->b_icon_unsel, t_sty[t_sty_idx].name)){
						sel_t_sty = t_sty_idx;
					}
				}
				
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, t_sty[t_sty_idx].file);
				
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, t_sty[t_sty_idx].subst_file);
				
				snprintf(txt, DXF_MAX_CHARS, "%d", t_sty[t_sty_idx].flags1);
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, txt);
				
				snprintf(txt, DXF_MAX_CHARS, "%d", t_sty[t_sty_idx].flags2);
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, txt);
				
				snprintf(txt, DXF_MAX_CHARS, "%0.2f", t_sty[t_sty_idx].width_f);
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, txt);
				
				snprintf(txt, DXF_MAX_CHARS, "%0.2f", t_sty[t_sty_idx].fixed_h);
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, txt);
				
				snprintf(txt, DXF_MAX_CHARS, "%0.2f", t_sty[t_sty_idx].oblique);
				nk_button_label_styled(gui->ctx, &gui->b_icon_unsel, txt);
				
				
			}
			nk_group_end(gui->ctx);
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 3);
		if (nk_button_label(gui->ctx, "Create")){
			/*
			show_lay_name = 1;
			lay_name[0] = 0;
			lay_change = LAY_OP_CREATE;*/
		}
		if ((nk_button_label(gui->ctx, "Rename")) && (sel_t_sty >= 0)){
			/*show_lay_name = 1;
			strncpy(lay_name, layers[sel_lay].name, DXF_MAX_CHARS);
			lay_change = LAY_OP_RENAME;*/
			
		}
		if ((nk_button_label(gui->ctx, "Remove")) && (sel_t_sty >= 0)){
			/*if (layers[sel_lay].num_el){
				snprintf(gui->log_msg, 63, "Error: Don't remove Layer in use");
			}
			else{
				
				layer_use(gui->drawing);
				dxf_obj_subst(layers[sel_lay].obj, NULL);
				sel_lay = -1;
				dxf_layer_assemb (gui->drawing);
				lay_change = LAY_OP_UPDATE;
			}*/
		}
		
	} else {
		show_font_mng = 0;
		font_change = FONT_OP_UPDATE;
	}
	nk_end(gui->ctx);
	
	return show_font_mng;
}