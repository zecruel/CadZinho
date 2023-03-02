#include "gui_xy.h"

int gui_update_pos(gui_obj *gui){
	double rect_pt1[2], rect_pt2[2];
	double cursor_x = 0.0, cursor_y = 0.0;
	double ref_x = 0.0, ref_y = 0.0;
	
	/* if user hit the enter key during a drawing operation, toggle axis lock */
	if ((gui->modal != SELECT) && (gui->step > 0) && (gui->ev & EV_LOCK_AX)
	//&& (!gui->user_flag_x) && (!gui->user_flag_y)
	){
		if ((gui->lock_ax_x != 0) || (gui->lock_ax_y != 0)){
			/* release the lock, if previously active */
			gui->lock_ax_x = 0;
			gui->lock_ax_y = 0;
			if (gui->rect_polar){
				gui->user_flag_y = 0;
			}
		}
		else{
			if (!gui->rect_polar){
				/* activate the lock according coordinate is predominant */
				gui->lock_ax_x = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) >= fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
				gui->lock_ax_y = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) < fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
			}
			else {
				double dx = (double) gui->mouse_x/gui->zoom + gui->ofs_x - gui->step_x[gui->step - 1];
				double dy = (double) gui->mouse_y/gui->zoom + gui->ofs_y - gui->step_y[gui->step - 1];
				double angle = atan2(dy, dx) * 6.0 / M_PI;
				angle = round(angle) * M_PI / 6.0;
				gui->user_y = angle * 180.0 / M_PI;
				
				gui->lock_ax_y = 1;
				gui->user_flag_y = 1;
			}
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
		
		gui->near_list = list_new(NULL, FRAME_LIFE);
		gui->near_count = dxf_ents_isect2(gui->near_list, gui->drawing, rect_pt1, rect_pt2);
		
		
		if ((gui->step >= 0) && (gui->step < 1000)){
			/* update current position by the mouse */
			gui->step_x[gui->step] = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
			gui->step_y[gui->step] = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
			gui->near_attr = ATRC_NONE;
			
			if (gui->step > 0) {
				ref_x = gui->step_x[gui->step - 1];
				ref_y = gui->step_y[gui->step - 1];
			} else {
				ref_x = gui->step_x[gui->step];
				ref_y = gui->step_y[gui->step];
			}
			
			if (!gui->free_sel){
				/* update current position by the attractor of near element */
				if (gui->near_attr = dxf_ent_attract(gui->drawing, gui->near_el, gui->curr_attr_t,
				gui->step_x[gui->step], gui->step_y[gui->step], ref_x, ref_y,
				(double) 20/gui->zoom, &gui->near_x , &gui->near_y)){
					gui->step_x[gui->step] = gui->near_x;
					gui->step_y[gui->step] = gui->near_y;
				}
			}
		}
		
		/* compute the next point coordinates by axis distances entry */
		if ((gui->en_distance) && (gui->step > 0) && (gui->step < 1000)){
			if (!gui->rect_polar){
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
			else {
				double dx = (double) gui->mouse_x/gui->zoom + gui->ofs_x - gui->step_x[gui->step - 1];
				double dy = (double) gui->mouse_y/gui->zoom + gui->ofs_y - gui->step_y[gui->step - 1];
				double dist = sqrt( dx * dx + dy * dy);
				double angle = atan2(dy, dx);
				/* check the user entry */
				if (gui->user_flag_x){
					dist = gui->user_x; /* distance in x param */
				}
				if (gui->user_flag_y){
					angle = gui->user_y * M_PI / 180.0; /* angle in y param  - degrees*/
					if (!gui->user_flag_x){
						dist = fabs(cos(angle) * (gui->step_x[gui->step] - gui->step_x[gui->step - 1]) +
							sin(angle) * (gui->step_y[gui->step] - gui->step_y[gui->step - 1]));
					}
				}
				if (gui->user_flag_x || gui->user_flag_y){
					/* calcule next coordinate pair from distance and/or angle entered */
					dx = dist * cos(angle);
					dy = dist * sin(angle);
					gui->step_x[gui->step] = gui->step_x[gui->step - 1] + dx;
					gui->step_y[gui->step] = gui->step_y[gui->step - 1] + dy;
				}
			}
		}
		if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 1000)){
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
	static int flag_x = 0;
	static int flag_y = 0;
	
	if (gui->step == 0){
		flag_x = 1;
		flag_y = 0;
	}
	else if (!gui->rect_polar){
		flag_x = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) >= fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
		flag_y = fabs(gui->step_x[gui->step] - gui->step_x[gui->step - 1]) < fabs(gui->step_y[gui->step] - gui->step_y[gui->step - 1]);
	}
	
	
	
	int space = 120;
	
	double dist = 0.0, angle = 0.0, dx = 0.0, dy = 0.0;
	if ((gui->en_distance) && (gui->step > 0) && (gui->step < 1000)){
		dx = gui->step_x[gui->step] - gui->step_x[gui->step - 1];
		dy = gui->step_y[gui->step] - gui->step_y[gui->step - 1];
		dist = sqrt( dx * dx + dy * dy);
		angle = atan2(dy, dx) * 180.0 / M_PI;
	}
	
	if (nk_group_begin(gui->ctx, "coord", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 10);
		if (!gui->rect_polar){
			nk_layout_row_push(gui->ctx, 20);
			/* X distance */
			/* hilite coordinate, if coord is predominant during a drawing operation*/
			if ((gui->en_distance) && (gui->step > 0) && (gui->step < 1000) && (flag_x)){
				nk_label_colored(gui->ctx, _l("X="), NK_TEXT_RIGHT, nk_rgb(255,255,0));
			}
			else {
				nk_label(gui->ctx, _l("X="), NK_TEXT_RIGHT);
			}
			space = 120;
		} else {
			nk_layout_row_push(gui->ctx, 40);
			nk_label(gui->ctx, _l("len="), NK_TEXT_RIGHT);
			space = 100;
		}
		/* verify if the user initiate a number entry during a drawing operation */
		if (((gui->en_distance)||(!gui->entry_relative)) && (gui->user_number) && (gui->step >= 0) && (gui->step < 1000) &&
		(!gui->user_flag_x) && (flag_x)){
			gui->user_number = 0; /* clear user flag */
			user_str_x[0] = 0; /* clear edit string */
			/* set focus to edit */
			nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		}
		
		nk_layout_row_push(gui->ctx, space);
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
			if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 1000)){
				snprintf(user_str_x, 63, "%f", gui->step_x[gui->step]);
			}
			else if (gui->en_distance){
				if (!gui->rect_polar){
					/* rectangular mode - delta x */
					snprintf(user_str_x, 63, "%f", dx);
				}
				else{
					/* polar mode - distance */
					snprintf(user_str_x, 63, "%f", dist);
				}
			}
			else {
				snprintf(user_str_x, 63, "%f", 0.0);
			}
		}
		if (res & NK_EDIT_COMMITED){ /* finish the enter mode */
			nk_edit_unfocus(gui->ctx);
			gui->keyEnter = 0;
			flag_x = 0;
			flag_y = 1;
		}
		if (!gui->rect_polar){
			nk_layout_row_push(gui->ctx, 20);
			/* Y distance */
			/* hilite coordinate, if coord is predominant during a drawing operation*/
			if ((gui->en_distance) && (gui->step > 0) && (gui->step < 1000) && (flag_y)){
				nk_label_colored(gui->ctx, _l("Y="), NK_TEXT_RIGHT, nk_rgb(255,255,0));
			}
			else {
				nk_label(gui->ctx, _l("Y="), NK_TEXT_RIGHT);
			}
		} else {
			nk_layout_row_push(gui->ctx, 40);
			nk_label(gui->ctx, _l("ang="), NK_TEXT_RIGHT);
		}
		/* verify if the user initiate a number entry during a drawing operation */
		if (((gui->en_distance)||(!gui->entry_relative)) && (gui->user_number) && (gui->step >= 0) && (gui->step < 1000) &&
		(!gui->user_flag_y) && (flag_y)){
			gui->user_number = 0; /* clear user flag */
			user_str_y[0] = 0; /* clear edit string */
			/* set focus to edit */
			nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		}
		
		nk_layout_row_push(gui->ctx, space);
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
			if ((!gui->entry_relative) && (gui->step >= 0) && (gui->step < 1000)){
				snprintf(user_str_y, 63, "%f", gui->step_y[gui->step]);
			}
			else if (gui->en_distance){
				if (!gui->rect_polar){
					/* rectangular mode - delta y */
					snprintf(user_str_y, 63, "%f", dy);
				}
				else{
					/* polar mode - angle in degrees */
					snprintf(user_str_y, 63, "%f", angle);
				}
			}
			else {
				snprintf(user_str_y, 63, "%f", 0.0);
			}
		}
		if (res & NK_EDIT_COMMITED){ /* finish the enter mode */
			nk_edit_unfocus(gui->ctx);
			gui->keyEnter = 0;
			flag_x = 1;
			flag_y = 0;
		}
		
		/* select if entry mode is in rectangular (X,Y) or polar (dist, angle)*/
		nk_layout_row_push(gui->ctx, 100);
		if (gui->rect_polar){
			gui->en_distance = 1; /* in polar mode, always enable distance */
			gui->entry_relative = 1;
			//nk_selectable_label(gui->ctx, "Polar", NK_TEXT_CENTERED, &gui->rect_polar);
			if(gui_selectable (gui, _l("Polar"), gui->rect_polar)){
				gui->rect_polar = 0;
				gui->user_flag_x = 0;
				gui->user_flag_y = 0;
			}
		}
		else if(gui_selectable (gui, _l("Rectangular"), gui->rect_polar)){
			gui->rect_polar = 1;
			gui->user_flag_x = 0;
			gui->user_flag_y = 0;
			flag_x = 1;
			flag_y = 0;
		}
		nk_layout_row_end(gui->ctx);
		
		/* view coordinates of mouse in drawing units */
		int text_len;
		char text[64];
		double pos_x = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
		double pos_y = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
		double pos_z = 0.0;
		
		nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny*/
	
		//nk_layout_row_dynamic(gui->ctx, 17, 1);
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 3);
		nk_layout_row_push(gui->ctx, 292);
		text_len = snprintf(text, 63, "(%f,  %f)", pos_x, pos_y);
		nk_label(gui->ctx, text, NK_TEXT_CENTERED);
		
		#if(0)
		/* get ray  from mouse point in screen*/
		
		double ray_o[3], ray_dir[3], plane[4], point[3];
		
		ray_o[0] = (double) gui->mouse_x * gui->drwg_view_i[0][0] +
			(double) gui->mouse_y * gui->drwg_view_i[1][0] +
			gui->drwg_view_i[3][0];
		ray_o[1] = (double) gui->mouse_x * gui->drwg_view_i[0][1] +
			(double) gui->mouse_y * gui->drwg_view_i[1][1] +
			gui->drwg_view_i[3][1];
		ray_o[2] = (double) gui->mouse_x * gui->drwg_view_i[0][2] +
			(double) gui->mouse_y * gui->drwg_view_i[1][2] +
			gui->drwg_view_i[3][2];
		
		ray_dir[0] = -gui->drwg_view_i[2][0];
		ray_dir[1] = -gui->drwg_view_i[2][1];
		ray_dir[2] = -gui->drwg_view_i[2][2];
		
		/* try xy plane*/
		plane[0] = 0.0; plane[1] = 0.0; plane[2] = 1.0; plane[3] = 0.0;
		if( ray_plane(ray_o, ray_dir, plane, point)){
			pos_x = point[0];
			pos_y = point[1];
			pos_z = point[2];
		}
		else{
			/* try xz plane*/
			plane[0] = 0.0; plane[1] = 1.0; plane[2] = 0.0; plane[3] = 0.0;
			if( ray_plane(ray_o, ray_dir, plane, point)){
				pos_x = point[0];
				pos_y = point[1];
				pos_z = point[2];
			}
			else{
				/* try yz plane*/
				plane[0] = 1.0; plane[1] = 0.0; plane[2] = 0.0; plane[3] = 0.0;
				if( ray_plane(ray_o, ray_dir, plane, point)){
					pos_x = point[0];
					pos_y = point[1];
					pos_z = point[2];
				}
				else{
					pos_x = ray_o[0];
					pos_y = ray_o[1];
					pos_z = ray_o[2];
				}
			}
		}
		
		pos_x = pos_x/gui->zoom + gui->ofs_x;
		pos_y = pos_y/gui->zoom + gui->ofs_y;
		pos_z = pos_z/gui->zoom + gui->ofs_z;
		
		text_len = snprintf(text, 63, "(%0.2f,  %0.2f,  %0.2f)", pos_x, pos_y, pos_z);
		nk_label(gui->ctx, text, NK_TEXT_CENTERED);
		#endif
		
		nk_style_pop_font(gui->ctx); /* return to the default font*/
		
		/* select if entry mode is in absolute coordinates or relative distance*/
		if (!gui->rect_polar){
			nk_layout_row_push(gui->ctx, 100);
			if (gui->entry_relative){
				if(gui_selectable (gui, _l("Relative"), gui->entry_relative)){
					gui->entry_relative = 0;
				}
			}
			else if(gui_selectable (gui, _l("Absolute"), gui->entry_relative)){
				gui->entry_relative = 1;
			}
		}
		
		nk_layout_row_end(gui->ctx);
		
		nk_group_end(gui->ctx);
	}
	
	return 1;
}