#include "gui_xy.h"

int gui_update_pos(gui_obj *gui){
	double rect_pt1[2], rect_pt2[2];
	
	/* if user hit the enter key during a drawing operation, toggle axis lock */
	if ((gui->modal != SELECT) && (gui->step > 0) && (gui->ev & EV_LOCK_AX)
	//&& (!gui->user_flag_x) && (!gui->user_flag_y)
	){
		if ((gui->lock_ax_x != 0) || (gui->lock_ax_y != 0)){
			/* release the lock, if previously active */
			gui->lock_ax_x = 0;
			gui->lock_ax_y = 0;
		}
		else{
			/* activate the lock according coordinate is predominant */
			gui->lock_ax_x = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) >= fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
			gui->lock_ax_y = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) < fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
		}
	}
	
	/* events to update current coordinates according the mouse position,
	axis locks, user entry, or DXf element attractor */
	if ((gui->ev & EV_ENTER) || (gui->ev & EV_CANCEL) || (gui->ev & EV_MOTION)){
		/* aproximation rectangle in mouse position (10 pixels wide) */
		rect_pt1[0] = (double) (gui->mouse_x - 5)/gui->zoom + gui->ofs_x;
		rect_pt1[1] = (double) (gui->mouse_y - 5)/gui->zoom + gui->ofs_y;
		rect_pt2[0] = (double) (gui->mouse_x + 5)/gui->zoom + gui->ofs_x;
		rect_pt2[1] = (double) (gui->mouse_y + 5)/gui->zoom + gui->ofs_y;
		/* get the drawing element near the mouse */
		gui->near_el = NULL;
		gui->near_el = (dxf_node *)dxf_ents_isect(gui->drawing, rect_pt1, rect_pt2);
		
		if ((gui->step >= 0) && (gui->step < 10)){
			/* update current position by the mouse */
			gui->step_x[gui->step] = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
			gui->step_y[gui->step] = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
			gui->near_attr = ATRC_NONE;
			
			if ((gui->modal != SELECT)&& (gui->step >= 1)){
				/* update current position by the attractor of near element */
				if (gui->near_attr = dxf_ent_attract(gui->drawing, gui->near_el, gui->curr_attr_t,
				gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step-1], gui->step_y[gui->step-1],
				(double) 20/gui->zoom, &gui->near_x , &gui->near_y)){
					gui->step_x[gui->step] = gui->near_x;
					gui->step_y[gui->step] = gui->near_y;
				}
			}
			else if (gui->modal != SELECT){
				/* update current position by the attractor of near element */
				if (gui->near_attr = dxf_ent_attract(gui->drawing, gui->near_el, gui->curr_attr_t,
				gui->step_x[gui->step], gui->step_y[gui->step], gui->step_x[gui->step], gui->step_y[gui->step],
				(double) 20/gui->zoom, &gui->near_x , &gui->near_y)){
					gui->step_x[gui->step] = gui->near_x;
					gui->step_y[gui->step] = gui->near_y;
				}
			}
		}
		
		/* compute the next point coordinates by axis distances entry */
		if ((gui->en_distance) && (gui->step > 0) && (gui->step < 10)){
			/* verify if an axis is locked during a drawing operation */
			if (gui->lock_ax_y != 0){
				gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			}
			if (gui->lock_ax_x != 0){
				gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			}
			/* check the user entry */
			if (gui->user_flag_x){
				gui->step_x[gui->step] = gui->step_x[gui->step - 1] + gui->user_x;
			}
			if (gui->user_flag_y){
				gui->step_y[gui->step] = gui->step_y[gui->step - 1] + gui->user_y;
			}
		}
		if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 10)){
			/* check the user entry */
			if (gui->user_flag_x){
				gui->step_x[gui->step] = gui->user_x;
			}
			if (gui->user_flag_y){
				gui->step_y[gui->step] = gui->user_y;
			}
		}
		
	}
}

int gui_xy(gui_obj *gui){
	static char user_str_x[64] = "0.000000", user_str_y[64] = "0.000000";
	
	/* interface to the user visualize and enter coordinates and distances*/
	nk_flags res;
	/* flags to verify which coordinate is predominant */
	int flag_x = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) >= fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
	int flag_y = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) < fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
	
	if (nk_group_begin(gui->ctx, "coord", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 10);
		nk_layout_row_push(gui->ctx, 20);
		/* X distance */
		/* hilite coordinate, if coord is predominant during a drawing operation*/
		if ((gui->en_distance) && (gui->step > 0) && (gui->step < 10) && (flag_x)){
			nk_label_colored(gui->ctx, "X=", NK_TEXT_RIGHT, nk_rgb(255,255,0));
		}
		else {
			nk_label(gui->ctx, "X=", NK_TEXT_RIGHT);
		}
		/* verify if the user initiate a number entry during a drawing operation */
		if (((gui->en_distance)||(!gui->entry_relative)) && (gui->user_number) && (gui->step >= 0) && (gui->step < 10) &&
		(!gui->user_flag_x) && (flag_x)){
			gui->user_number = 0; /* clear user flag */
			user_str_x[0] = 0; /* clear edit string */
			/* set focus to edit */
			nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		}
		
		nk_layout_row_push(gui->ctx, 120);
		/* edit to visualize or enter distance */
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_str_x, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_x)){
				/* sinalize the distance of user entry */
				gui->user_x = atof(user_str_x);
				gui->user_flag_x = 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag_x = 0;
				nk_edit_unfocus(gui->ctx);
			}
		}
		else { /* visualize mode */
			if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 10)){
				snprintf(user_str_x, 63, "%f", gui->step_x[gui->step]);
			}
			else if ((gui->en_distance) && (gui->step > 0) && (gui->step < 10)){
				snprintf(user_str_x, 63, "%f", gui->step_x[gui->step] - gui->step_x[gui->step - 1]);
			}
			else {
				snprintf(user_str_x, 63, "%f", 0.0);
			}
		}
		if (res & NK_EDIT_COMMITED){ /* finish the enter mode */
			nk_edit_unfocus(gui->ctx);
			gui->keyEnter = 0;
		}
		
		nk_layout_row_push(gui->ctx, 20);
		/* Y distance */
		/* hilite coordinate, if coord is predominant during a drawing operation*/
		if ((gui->en_distance) && (gui->step > 0) && (gui->step < 10) && (flag_y)){
			nk_label_colored(gui->ctx, "Y=", NK_TEXT_RIGHT, nk_rgb(255,255,0));
		}
		else {
			nk_label(gui->ctx, "Y=", NK_TEXT_RIGHT);
		}
		/* verify if the user initiate a number entry during a drawing operation */
		if (((gui->en_distance)||(!gui->entry_relative)) && (gui->user_number) && (gui->step >= 0) && (gui->step < 10) &&
		(!gui->user_flag_y) && (flag_y)){
			gui->user_number = 0; /* clear user flag */
			user_str_y[0] = 0; /* clear edit string */
			/* set focus to edit */
			nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		}
		
		nk_layout_row_push(gui->ctx, 120);
		/* edit to visualize or enter distance */
		res = nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT, user_str_y, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_y)){
				/* sinalize the distance of user entry */
				gui->user_y = atof(user_str_y);
				gui->user_flag_y = 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag_y = 0;
				nk_edit_unfocus(gui->ctx);
			}
		}
		else { /* visualize mode */
			if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 10)){
				snprintf(user_str_y, 63, "%f", gui->step_y[gui->step]);
			}
			else if ((gui->en_distance) && (gui->step > 0) && (gui->step < 10)){
				snprintf(user_str_y, 63, "%f", gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
			}
			else {
				snprintf(user_str_y, 63, "%f", 0.0);
			}
		}
		if (res & NK_EDIT_COMMITED){ /* finish the enter mode */
			nk_edit_unfocus(gui->ctx);
			gui->keyEnter = 0;
		}
		
		/* select if entry mode is in absolute coordinates or relative distance*/
		nk_layout_row_push(gui->ctx, 70);
		if (gui->entry_relative){
			nk_selectable_label(gui->ctx, "Relative", NK_TEXT_CENTERED, &gui->entry_relative);
			flag_x = 1;
		}
		else nk_selectable_label(gui->ctx, "Absolute", NK_TEXT_CENTERED, &gui->entry_relative);
		nk_layout_row_end(gui->ctx);
		
		/* view coordinates of mouse in drawing units */
		int text_len;
		char text[64];
		double pos_x = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
		double pos_y = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
		
		nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny*/
	
		nk_layout_row_dynamic(gui->ctx, 17, 1);
		text_len = snprintf(text, 63, "(%f,  %f)", pos_x, pos_y);
		nk_label(gui->ctx, text, NK_TEXT_CENTERED);
		
		nk_style_pop_font(gui->ctx); /* return to the default font*/
		
		nk_group_end(gui->ctx);
	}
	
	return 1;
}