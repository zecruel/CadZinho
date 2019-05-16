#include "gui_use.h"
#include <float.h>

int gui_hatch_interactive(gui_obj *gui){
	/* Initially, uses a lwpolyline (without bulge) as bondary. */
	if (gui->modal == HATCH){
		static dxf_node *new_el;
		
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				/* create a new DXF lwpolyline */
				new_el = (dxf_node *) dxf_new_lwpolyline (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, /* pt1, */
					0.0, /* bulge */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE); /* paper space */
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->bulge, DWG_LIFE);
				dxf_attr_change_i(new_el, 70, (void *) (int[]){1}, 0);
				gui->element = new_el;
				gui->step = 1;
				gui->en_distance = 1;
				gui->draw_tmp = 1;
				goto next_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				gui->step_x[gui->step - 1] = gui->step_x[gui->step];
				gui->step_y[gui->step - 1] = gui->step_y[gui->step];
				
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				//dxf_attr_change_i(new_el, 42, &gui->bulge, -1);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
				
				dxf_lwpoly_append (new_el, gui->step_x[gui->step], gui->step_y[gui->step], 0.0, gui->bulge, DWG_LIFE);
				gui->step = 2;
				goto next_step;
			}
			else if (gui->ev & EV_CANCEL){
				gui->draw_tmp = 0;
				if (gui->step == 2){
					dxf_lwpoly_remove (new_el, -1);
					//new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
					//drawing_ent_append(gui->drawing, new_el);
					
					graph_obj *bound = dxf_lwpline_parse(gui->drawing, new_el, 0 , 0);
					
					struct h_pattern *curr_h;// = &(gui->list_pattern);
					struct h_family *curr_fam = gui->hatch_fam.next;
					int i = 0;
					
					double rot = 0.0, scale = 1.0;
					
					if(gui->h_type == HATCH_USER) { /* user definied simple pattern */
						strncpy(gui->list_pattern.name, "USER_DEF", DXF_MAX_CHARS);
						curr_h = &(gui->list_pattern);
						rot = 0.0;
						scale = 1.0;
					}
					else if(gui->h_type == HATCH_SOLID) { /* solid pattern */
						strncpy(gui->list_pattern.name, "SOLID", DXF_MAX_CHARS);
						curr_h = &(gui->list_pattern);
						rot = 0.0;
						scale = 1.0;
					}
					else{ /* pattern from library */
						
						/* get current family */
						curr_h = NULL;
						i = 0;
						while (curr_fam){
							if (gui->hatch_fam_idx == i){
								curr_h = curr_fam->list->next;
								break;
							}
							
							i++;
							curr_fam = curr_fam->next;
						}
						
						/* get current hatch pattern */
						i = 0;
						while ((curr_h) && (i < gui->hatch_idx)){
							i++;
							curr_h = curr_h->next;
						}
						/* optional rotation and scale */
						rot = gui->patt_ang;
						scale = gui->patt_scale;
					}
					
					/* make DXF HATCH entity */
					dxf_node *new_hatch_el = dxf_new_hatch (curr_h, bound,
					gui->h_type == HATCH_SOLID, gui->hatch_assoc,
					0, 0, /* style, type */
					rot, scale,
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0, DWG_LIFE); /* paper space */
					
					if (new_hatch_el){
						/* parse entity */
						new_hatch_el->obj.graphics = dxf_graph_parse(gui->drawing, new_hatch_el, 0 , 0);
						/* and append to drawing */
						drawing_ent_append(gui->drawing, new_hatch_el);
						/* add to the undo/redo list*/
						do_add_entry(&gui->list_do, "HATCH");
						do_add_item(gui->list_do.current, NULL, new_hatch_el);
					}
					
					gui->step = 0;
				}
				gui->element = NULL;
				goto next_step;
			}
			if (gui->ev & EV_MOTION){
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change_i(new_el, 10, &gui->step_x[gui->step], -1);
				dxf_attr_change_i(new_el, 20, &gui->step_y[gui->step], -1);
				//dxf_attr_change_i(new_el, 42, &gui->bulge, -1);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
		
	}
	goto end_step;
	default_modal:
		gui->modal = SELECT;
	first_step:
		gui->en_distance = 0;
		gui->draw_tmp = 0;
		gui->element = NULL;
		gui->draw = 1;
		gui->step = 0;
		gui->draw_phanton = 0;
		//if (gui->phanton){
		//	gui->phanton = NULL;
		//}
	next_step:
		
		gui->lock_ax_x = 0;
		gui->lock_ax_y = 0;
		gui->user_flag_x = 0;
		gui->user_flag_y = 0;

		gui->draw = 1;
	end_step:
		return 1;
}

int gui_hatch_info (gui_obj *gui){
	if (gui->modal == HATCH) {
		static int show_pat_pp = 0, show_pat_file = 0;
		static int patt_idx = 0, last_idx = -1;
		struct h_pattern *curr_h = NULL;
		struct h_family *curr_fam = gui->hatch_fam.next;
		static double patt_scale = 1, patt_rot = 0.0;
		
		int i = 0;
		/* Tabs for select three options:
			- User definied simple hatch;
			- Hatch pattern from a library;
			- Solid fill; */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, "User", gui->h_type == HATCH_USER)) gui->h_type = HATCH_USER;
		if (gui_tab (gui, "Library", gui->h_type == HATCH_PREDEF)) gui->h_type = HATCH_PREDEF;
		if (gui_tab (gui, "Solid", gui->h_type == HATCH_SOLID)) gui->h_type = HATCH_SOLID;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		nk_layout_row_dynamic(gui->ctx, 125, 1);
		if (nk_group_begin(gui->ctx, "Patt_controls", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
		
			if (gui->h_type == HATCH_USER){/*User definied simple hatch*/
				/* the user can select only angle and spacing of continuous lines*/
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				gui->user_patt.ang = nk_propertyd(gui->ctx, "Angle", 0.0d, gui->user_patt.ang, 360.0d, 0.5d, 0.5d);
				gui->user_patt.dy = nk_propertyd(gui->ctx, "Spacing", 0.0d, gui->user_patt.dy, DBL_MAX, 0.1d, 0.1d);
			}
			else if (gui->h_type == HATCH_PREDEF){ /*Hatch pattern from a library */
				/*the library or family of pattern hatchs is a .pat file, according the
				Autodesk especification.
				
				The Standard family is embeded in program and the user can load
				other librarys from files (see the popup windows below)*/
				
				/* get current family */
				curr_fam = gui->hatch_fam.next;
				i = 0;
				gui->patt_name[0] = 0; /*clear data of current pattern */
				gui->patt_descr[0] = 0;
				curr_h = NULL;
				while (curr_fam){
					if (gui->hatch_fam_idx == i){
						strncpy(gui->h_fam_name, curr_fam->name, DXF_MAX_CHARS);
						strncpy(gui->h_fam_descr, curr_fam->descr, DXF_MAX_CHARS);
						curr_h = curr_fam->list->next;
					}
					
					i++;
					curr_fam = curr_fam->next;
				}
				
				/*get current pattern */
				i = 0;
				while (curr_h){
					if (gui->hatch_idx == i){
						strncpy(gui->patt_name, curr_h->name, DXF_MAX_CHARS);
						strncpy(gui->patt_descr, curr_h->descr, DXF_MAX_CHARS);
					}
					
					i++;
					curr_h = curr_h->next;
				}
				
				
				nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.85f, 0.15f});
				
				/* count hatch families */
				curr_fam = gui->hatch_fam.next;
				i = 0;
				while (curr_fam){
					i++;
					curr_fam = curr_fam->next;
				}
				
				int h = i * 25 + 5;
				h = (h < 300)? h : 300;
				
				/* selection of families */
				if (nk_combo_begin_label(gui->ctx, gui->h_fam_name, nk_vec2(150, h))){
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					curr_fam = gui->hatch_fam.next;
					i = 0;
					while (curr_fam){
						if (nk_button_label(gui->ctx, curr_fam->name)){
							gui->hatch_fam_idx = i;
							gui->hatch_idx = 0;
							nk_combo_close(gui->ctx);
							patt_idx = 0;
							last_idx = -1;
						}
						i++;
						curr_fam = curr_fam->next;
					}
					
					nk_combo_end(gui->ctx);
				}
				/* for load other pattern families from file */
				if (nk_button_symbol(gui->ctx, NK_SYMBOL_PLUS)) show_pat_file = 1;
				
				/* for selection of pattern*/
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				if (nk_button_label(gui->ctx, "Explore")) show_pat_pp = 1;
				
				/*show data of current pattern*/
				nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.2f, 0.8f});
				nk_label(gui->ctx, "Name:", NK_TEXT_RIGHT);
				nk_label_colored(gui->ctx, gui->patt_name, NK_TEXT_CENTERED, nk_rgb(255,255,0));
				
				/* optional rotation and scale */
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				gui->patt_scale = nk_propertyd(gui->ctx, "#Scale", 0.0d, gui->patt_scale, DBL_MAX, 0.1d, 0.1d);
				gui->patt_ang = nk_propertyd(gui->ctx, "Angle", 0.0d, gui->patt_ang, 360.0d, 0.5d, 0.5d);
			}
			nk_group_end(gui->ctx);
		}
		/* associative flag for Hatch*/
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_checkbox_label(gui->ctx, "Associative", &gui->hatch_assoc);
		
		/*messages for user in iteractive mode*/
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter next point", NK_TEXT_LEFT);
		}
		
		if (show_pat_pp){
			/* selection pattern popup window */
			
			static char patt_name[DXF_MAX_CHARS], patt_descr[DXF_MAX_CHARS];
			static struct nk_rect s = {120, -210, 420, 490};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Select Pattern", NK_WINDOW_CLOSABLE, s)){
				graph_obj *ref_graph = NULL, *curr_graph = NULL;
				list_node * pat_g = NULL;
				
				int pat_ei = 0; /*extents flag */
				/* extents and zoom parameters */
				double pat_x0, pat_y0, pat_x1, pat_y1, z, z_x, z_y, o_x, o_y;
				double cosine, sine, dx, dy, max;
				double ang, ox, oy, dash[20];
				int num_dash;
				
				/* get current family */
				curr_fam = gui->hatch_fam.next;
				i = 0;
				curr_h = NULL;
				while (curr_fam){
					if (gui->hatch_fam_idx == i){
						curr_h = curr_fam->list->next;
					}
					
					i++;
					curr_fam = curr_fam->next;
				}
				
				/* show data of current family */
				nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.15f, 0.85f});
				nk_label(gui->ctx, "Family:", NK_TEXT_RIGHT);
				nk_label_colored(gui->ctx, gui->h_fam_name, NK_TEXT_LEFT, nk_rgb(255,255,0));
				nk_layout_row_dynamic(gui->ctx, 50, 1);
				nk_label_colored_wrap(gui->ctx, gui->h_fam_descr, nk_rgb(100,115,255));
				
				/* show and allow selection of patterns in current library*/
				nk_layout_row_dynamic(gui->ctx, 360, 2);
				if (nk_group_begin(gui->ctx, "Patt_names", NK_WINDOW_BORDER)) {
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					i = 0;
					while (curr_h){
						if (nk_button_label(gui->ctx, curr_h->name)){
							patt_idx = i;
						}
						i++;
						curr_h = curr_h->next;
					}
					nk_group_end(gui->ctx);
				}
				
				/* in next, create the preview visualization of selected pattern*/
				/* get current family */
				curr_fam = gui->hatch_fam.next;
				i = 0;
				curr_h = NULL;
				while (curr_fam){
					if (gui->hatch_fam_idx == i){
						curr_h = curr_fam->list->next;
					}
					i++;
					curr_fam = curr_fam->next;
				}
				
				/* get selected hatch pattern */
				i = 0;
				while (curr_h){
					strncpy(patt_name, curr_h->name, DXF_MAX_CHARS);
					strncpy(patt_descr, curr_h->descr, DXF_MAX_CHARS);
					if (patt_idx == i) break;
					
					i++;
					curr_h = curr_h->next;
				}
				
				/* calcule the ideal scale for preview */
				struct hatch_line *curr_l = NULL;
				if (curr_h){
					max = 0.0;
					double patt_len = 0.0;
					
					if (patt_idx != last_idx){
						curr_l = curr_h->lines;
						while (curr_l){
							if (curr_l->num_dash < 2)
								max = (max > sqrt(curr_l->dx*curr_l->dx + curr_l->dy*curr_l->dy))? max : sqrt(curr_l->dx*curr_l->dx + curr_l->dy*curr_l->dy);
							else
								max = (max > sqrt(curr_l->dx*curr_l->dx + curr_l->dy*curr_l->dy)/curr_l->num_dash)? max : sqrt(curr_l->dx*curr_l->dx + curr_l->dy*curr_l->dy)/curr_l->num_dash;
							curr_l = curr_l->next;
						}
						
						if (max > 0.0) patt_scale = 1/max;
						else patt_scale = 1.0;
						
						if (curr_h->num_lines > 1) patt_scale *= sqrt(curr_h->num_lines);
						
						patt_rot = 0.0;
						
						last_idx = patt_idx;
					}
					
					pat_g = list_new(NULL, FRAME_LIFE);
					
					/*create reference graph bondary (10 x 10 units) */
					ref_graph = graph_new(FRAME_LIFE);
					line_add(ref_graph, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0);
					line_add(ref_graph, 10.0, 0.0, 0.0, 10.0, 10.0, 0.0);
					line_add(ref_graph, 10.0, 10.0, 0.0, 0.0, 10.0, 0.0);
					line_add(ref_graph, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0);
					
					curr_l = curr_h->lines;
				}
				
				/* create graph for each definition lines in pattern */
				while (curr_l){
					/* apply scale and rotation*/
					ang = fmod(curr_l->ang + patt_rot, 360.0);
					cosine = cos(ang * M_PI/180);
					sine = sin(ang * M_PI/180);
					dx = patt_scale * (cosine*curr_l->dx - sine*curr_l->dy);
					dy = patt_scale * (sine*curr_l->dx + cosine*curr_l->dy);
					cosine = cos(patt_rot * M_PI/180);
					sine = sin(patt_rot * M_PI/180);
					ox = patt_scale * (cosine*curr_l->ox - sine*curr_l->oy);
					oy = patt_scale * (sine*curr_l->ox + cosine*curr_l->oy);
					num_dash = curr_l->num_dash;
					for (i = 0; i < num_dash; i++){
						dash[i] = patt_scale * curr_l->dash[i];
					}
					if (num_dash == 0) { /* for continuous line*/
						dash[0] = 1.0;
						num_dash = 1;
					}
					/* get hatch graph of current def line*/
					curr_graph = graph_hatch(ref_graph, ang * M_PI/180,
						ox, oy,
						dx, dy,
						dash, num_dash,
						FRAME_LIFE);
					
					if ((curr_graph != NULL) && (pat_g != NULL)){
						/*change color -> white*/
						curr_graph->color.r = 255;// - gui->preview_img->bkg.r;
						curr_graph->color.g = 255;// - gui->preview_img->bkg.g;
						curr_graph->color.b = 255;// - gui->preview_img->bkg.b;
						
						list_push(pat_g, list_new((void *)curr_graph, FRAME_LIFE));
					}
					
					curr_l = curr_l->next;
				}
				
				/* calcule the zoom and offset for preview */
				graph_list_ext(pat_g, &pat_ei, &pat_x0, &pat_y0, &pat_x1, &pat_y1);
				
				z_x = fabs(pat_x1 - pat_x0)/gui->preview_img->width;
				z_y = fabs(pat_y1 - pat_y0)/gui->preview_img->height;
				z = (z_x > z_y) ? z_x : z_y;
				if (z <= 0) z =1;
				else z = 1/(1.1 * z);
				o_x = pat_x0 - (fabs((pat_x1 - pat_x0)*z - gui->preview_img->width)/2)/z;
				o_y = pat_y0 - (fabs((pat_y1 - pat_y0)*z - gui->preview_img->height)/2)/z;
				
				/* draw graphics in preview bitmap */
				bmp_fill(gui->preview_img, gui->preview_img->bkg); /* clear bitmap */
				//graph_list_draw(pat_g, gui->preview_img, o_x, o_y, z);
				struct draw_param d_param;
				
				d_param.ofs_x = o_x;
				d_param.ofs_y = o_y;
				d_param.scale = z;
				d_param.list = NULL;
				d_param.subst = NULL;
				d_param.len_subst = 0;
				d_param.inc_thick = 0;
				graph_list_draw(pat_g, gui->preview_img, d_param);
				
				
				
				if (nk_group_begin(gui->ctx, "Patt_prev", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
					/*show data of current pattern*/
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label_colored(gui->ctx, patt_name, NK_TEXT_CENTERED, nk_rgb(255,255,0));
					
					/* preview img */
					nk_layout_row_dynamic(gui->ctx, 175, 1);
					nk_button_image(gui->ctx,  nk_image_ptr(gui->preview_img));
					nk_layout_row_dynamic(gui->ctx, 50, 1);
					nk_label_colored_wrap(gui->ctx, patt_descr, nk_rgb(100,115,255));
					
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Ref: 10 x 10 units", NK_TEXT_CENTERED);
					/* optional parameters -> change the preview */
					patt_scale = nk_propertyd(gui->ctx, "#Scale", 0.001, patt_scale, DBL_MAX, 0.001, 0.001);
					patt_rot = nk_propertyd(gui->ctx, "#Rotation", 0.00, patt_rot, 360.0, 0.1, 0.1);
					
					if (nk_button_label(gui->ctx, "Select")){ /*done the selection*/
						/* update  the main parameters */
						gui->hatch_idx = patt_idx;
						gui->patt_scale = patt_scale;
						gui->patt_ang = patt_rot;
						show_pat_pp = 0; /* close */
					}
					nk_group_end(gui->ctx);
				}
				
				nk_popup_end(gui->ctx);
			}
			else show_pat_pp = 0;
		}
		if (show_pat_file){
			/* Load other pattern libraries */
			static char pat_path[DXF_MAX_CHARS];
			static int pat_path_len = 0;
			
			static struct nk_rect s = {20, 100, 400, 150};
			if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Add pattern family", NK_WINDOW_CLOSABLE, s)){
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, "File to Open:", NK_TEXT_CENTERED);
				/* get the file path */
				nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, pat_path, &pat_path_len, DXF_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "OK")) {
					pat_path[pat_path_len] = 0; /* terminate string */
					/* check if filename extension is ".pat" */
					char *ext = get_ext(pat_path);
					if (strcmp(ext, "pat") == 0){
						/*use the filename without extension for name the library */
						char *filename = get_filename(pat_path);
						strip_ext(filename);
						/* parse the file*/
						gui->end_fam->next = dxf_hatch_family_file(filename, pat_path);
						/* and append to list*/
						if(gui->end_fam->next) gui->end_fam = gui->end_fam->next;
						show_pat_file = nk_false; /*close the window*/
					}
					
					pat_path_len = 0;
				}
				nk_popup_end(gui->ctx);
			} else {
				show_pat_file = nk_false;
				pat_path_len = 0;
			}
		}
	}
	return 1;
}