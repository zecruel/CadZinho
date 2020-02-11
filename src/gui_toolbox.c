#include "gui_use.h"

int gui_tools_win (gui_obj *gui){

	if (nk_begin(gui->ctx, "Toolbox", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)){
			
		//if (nk_tree_push(gui->ctx, NK_TREE_TAB, "Place", NK_MAXIMIZED)) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Modify:", NK_TEXT_LEFT);
		
		nk_layout_row_static(gui->ctx, 28, 28, 6);
		
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CURSOR]))){
			gui->modal = SELECT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT_E]))){
			gui->modal = ED_TEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MOVE]))){
			gui->modal = MOVE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DUPLI]))){
			gui->modal = DUPLI;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SCALE]))){
			gui->modal = SCALE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ROT]))){
			gui->modal = ROTATE;
			gui->step = 0;
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MIRROR]))){
			gui->modal = MIRROR;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BLOCK]))){
			gui->modal = NEW_BLK;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPLODE]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EDIT]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG_E]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FIND]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RULER]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_STYLE]))){
			
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TRASH]))){
			gui->action = DELETE;
			gui->step = 0;
		}
			
		
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place:", NK_TEXT_LEFT);
			
		nk_layout_row_static(gui->ctx, 28, 28, 6);
		
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LINE]))){
			gui->modal = LINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PLINE]))){
			gui->modal = POLYLINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RECT]))){
			gui->modal = RECT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT]))){
			gui->modal = TEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CIRCLE]))){
			gui->modal = CIRCLE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ELIPSE]))){
			gui->modal = ELLIPSE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ARC]))){
			gui->modal = ARC;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SPLINE]))){
			gui->modal = SPLINE;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HATCH]))){
			gui->modal = HATCH;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_I_TEXT]))){
			gui->modal = MTEXT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BOOK]))){
			gui->modal = INSERT;
			gui->step = 0;
		}
		if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_IMAGE]))){
			gui->modal = IMAGE;
			gui->step = 0;
		}
		
		nk_layout_row_dynamic(gui->ctx, 20, 1); /*space*/
		
		struct nk_vec2 wid_pos = nk_widget_position(gui->ctx);
		struct nk_vec2 win_pos = nk_window_get_position(gui->ctx);
		struct nk_rect win_cont = nk_window_get_content_region(gui->ctx);
	
		nk_layout_row_dynamic(gui->ctx, win_cont.h - (wid_pos.y - win_pos.y), 1);
		if (nk_group_begin(gui->ctx, "especific", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			gui_select_info (gui);
			gui_line_info (gui);
			gui_pline_info (gui);
			gui_circle_info (gui);
			gui_rect_info (gui);
			gui_text_info (gui);
			gui_mtext_info (gui);
			gui_move_info (gui);
			gui_dupli_info (gui);
			gui_scale_info (gui);
			gui_rotate_info (gui);
			gui_mirror_info (gui);
			gui_insert_info (gui);
			gui_block_info (gui);
			gui_hatch_info (gui);
			gui_paste_info (gui);
			gui_ed_text_info (gui);
			gui_spline_info (gui);
			gui_arc_info (gui);
			gui_ellip_info (gui);
			gui_image_info (gui);
			
			if (gui->modal == SCRIPT) {
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, "Runing Script", NK_TEXT_LEFT);
			}
			
			nk_group_end(gui->ctx);
		}
	}
	nk_end(gui->ctx);
	return 1;
}