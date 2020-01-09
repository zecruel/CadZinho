#include "gui_use.h"

static void angle_range(double *ang){
	/* set angle range to 0-2*pi */
	if (fabs(*ang) > 2*M_PI) *ang = fmod(*ang, 2*M_PI);
	if (*ang < 0) *ang += 2*M_PI;
}

int gui_ellip_interactive(gui_obj *gui){
	
	if (gui->modal != ELLIPSE) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	
	static dxf_node *new_el;
	if (gui->step == 0){
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (gui->el_mode == EL_ISO_CIRCLE || gui->el_mode == EL_ISO_ARC){
				gui->step = 2;
				gui->step_x[2] = gui->step_x[0];
				gui->step_y[2] = gui->step_y[0];
			} else gui->step = 1;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			/* next step */
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
	else if (gui->step == 1){
		/* define ellipse's major axis */
		major = sqrt(pow((gui->step_x[1] - gui->step_x[0]), 2) + pow((gui->step_y[1] - gui->step_y[0]), 2));
		if (major == 0.0) return 0;
		
		/* draw a line to helps user to see axis size and direction */
		gui->draw_phanton = 0;
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			/* dashed line */
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		if (gui->ev & EV_ENTER){
			/* accept axis */
			gui->step = 2;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
	}
	else if (gui->step == 2){
		/* define ellipse's minor axis */
		
		if (gui->el_mode == EL_ISO_CIRCLE || gui->el_mode == EL_ISO_ARC){
			major = sqrt(pow((gui->step_x[2] - gui->step_x[0]), 2) + pow((gui->step_y[2] - gui->step_y[0]), 2));
			/* rotation parameters of entered point,  relative to center */
			cosine = (gui->step_x[2] - gui->step_x[0])/ major;
			sine = (gui->step_y[2] - gui->step_y[0])/ major;
			
			/* for circle equivalent in isometric view,
			ellipse's major axis must be sqrt(3)/sqrt(2)*radius
			and minor axis ratio is 1/sqrt(3) */
			major *= 1.2247448713915890490986420373529;
			ratio = 0.57735026918962576450914878050196;
			
			/* elipse rotation constants (cosine and sine), according ortho view
			Top view -> angle = 0
			Front -> angle = 60 degrees
			Left  -> angle = 120 degrees */			
			double x = 1.0 , y = 0.0;
			if (gui->o_view == O_TOP){
				x = 1.0;
				y = 0.0;
			}
			else if (gui->o_view == O_FRONT){
				x = 0.5;
				y = 0.86602540378443864676372317075294;
			}
			else if (gui->o_view == O_LEFT){
				x = -0.5;
				y = 0.86602540378443864676372317075294;
			}
			if ( x * cosine + y * sine < 0){
				/* flip elipse start point, according entered point */
				x = -x;
				y = -y;
			}
			
			gui->step_x[1] = x * major + gui->step_x[0];
			gui->step_y[1] = y * major + gui->step_y[0];
		} else {
			ratio = sqrt(pow((gui->step_x[2] - gui->step_x[0]), 2) + pow((gui->step_y[2] - gui->step_y[0]), 2))/major;
			if (ratio > 1.0) ratio = 1.0; /* limits the minor axis to major axis maximum size*/
		}
		/* draw a ellipse skecth to helps user */
		gui->draw_phanton = 0;
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			/* dashed line */
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			/* major axis */
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			/* minor axis */
			double x = gui->step_x[0] - ratio * (gui->step_y[1] - gui->step_y[0]);
			double y = gui->step_y[0] + ratio * (gui->step_x[1] - gui->step_x[0]);
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				x, y, 0);
			/* ellipse */
			graph_ellipse(graph, gui->step_x[0], gui->step_y[0], 0.0,
				gui->step_x[1] - gui->step_x[0], gui->step_y[1] - gui->step_y[0], 0.0,
				ratio, 0.0, 2*M_PI);
			
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
			/* acept second axis */
			if (gui->el_mode == EL_FULL || gui->el_mode == EL_ISO_CIRCLE){
				gui->step = 5;
				start = 0.0;
				end = 2*M_PI;
			}
			else gui->step = 3;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
	}
	else if (gui->step == 3){
		/* Define the start parameter in elliptical arcs */
		
		/* rotation constants */
		rot = atan2((gui->step_y[1] - gui->step_y[0]), (gui->step_x[1] - gui->step_x[0]));
		angle_range(&rot);
		cosine = cos(rot);
		sine = sin(rot);
		/*ellipse's parameters */
		double a = major, b = major * ratio;
		/* find the angle of point, referenced to arc center */
		double ang_pt = atan2(gui->step_y[3] - gui->step_y[0], gui->step_x[3] - gui->step_x[0]);
		/* update by rotation */
		ang_pt = ang_pt - rot;
		angle_range(&ang_pt); /* set angle range to 0-2*pi */
		/* find the ellipse polar parameter (t) for  point*/
		double t = ellipse_par(ang_pt, a, b);
		
		/* draw a ellipse skecth to helps user */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			/* dashed line */
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			
			/* major axis*/
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			/* line to view start angle */
			double x = gui->step_x[0] + a*cos(t)*cosine - b*sin(t)*sine;
			double y = gui->step_y[0] + a*cos(t)*sine + b*sin(t)*cosine;
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				x, y, 0);
			/* ellipse */
			graph_ellipse(graph, gui->step_x[0], gui->step_y[0], 0.0,
				gui->step_x[1] - gui->step_x[0], gui->step_y[1] - gui->step_y[0], 0.0,
				ratio, 0.0, 2*M_PI);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		
		if (gui->ev & EV_ENTER){
			/* accept the start point */
			gui->step = 4;
			start = t;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
		
	}
	else if (gui->step == 4){
		/* Define the end parameter in elliptical arcs */
		
		/*ellipse's parameters */
		double a = major, b = major * ratio;
		/* find the angle of point, referenced to arc center */
		double ang_pt = atan2(gui->step_y[4] - gui->step_y[0], gui->step_x[4] - gui->step_x[0]);
		/* update by rotation */
		ang_pt = ang_pt - rot;
		angle_range(&ang_pt); /* set angle range to 0-2*pi */
		/* find the ellipse polar parameter (t) for  point*/
		double t = ellipse_par(ang_pt, a, b);
		
		if (t < start) t = 2*M_PI; /* limit the maximum parameter */
		
		/* draw a ellipse skecth to helps user */
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			/* dashed line */
			graph->patt_size = 2;
			graph->pattern[0] = 10 / gui->zoom;
			graph->pattern[1] = -10 / gui->zoom;
			/* major axis */
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			/* line to view end angle */
			double x = gui->step_x[0] + a*cos(t)*cosine - b*sin(t)*sine;
			double y = gui->step_y[0] + a*cos(t)*sine + b*sin(t)*cosine;
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				x, y, 0);
			/*ellipse */
			graph_ellipse(graph, gui->step_x[0], gui->step_y[0], 0.0,
				gui->step_x[1] - gui->step_x[0], gui->step_y[1] - gui->step_y[0], 0.0,
				ratio, t, start);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		/* draw a arc skecth to helps user */
		graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			graph_ellipse(graph, gui->step_x[0], gui->step_y[0], 0.0,
				gui->step_x[1] - gui->step_x[0], gui->step_y[1] - gui->step_y[0], 0.0,
				ratio, start, t);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
		if (gui->ev & EV_ENTER){
			/* accept the end point */
			gui->step = 5;
			end = t;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
			gui_next_step(gui);
			
		}
		else if (gui->ev & EV_CANCEL){
			gui_first_step(gui);
		}
	}
	
	else{
		/* create a new DXF ellipse */
		new_el = (dxf_node *) dxf_new_ellipse (gui->step_x[0], gui->step_y[0], 0.0,
			gui->step_x[1] - gui->step_x[0], gui->step_y[1] - gui->step_y[0], 0.0,
			ratio, start, end,
			gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
			gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
			0, DWG_LIFE); /* paper space */
		
		new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
		drawing_ent_append(gui->drawing, new_el);
		
		do_add_entry(&gui->list_do, "ELLIPSE");
		do_add_item(gui->list_do.current, NULL, new_el);
		
		gui->draw_phanton = 0;
		gui_first_step(gui);
	}
	
	return 1;
}

int gui_ellip_info (gui_obj *gui){
	if (gui->modal != ELLIPSE) return 0;
	
	static const char *mode[] = {"Full ellipse","Elliptical arc", "Isometric Circle", "Isometric Arc"};
	static const char *view[] = {"Top","Front", "Left"};
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Place a ellipse", NK_TEXT_LEFT);
	gui->el_mode = nk_combo(gui->ctx, mode, 4, gui->el_mode, 20, nk_vec2(150, 105));
	if (gui->el_mode == EL_ISO_CIRCLE || gui->el_mode == EL_ISO_ARC){
		gui->o_view = nk_combo(gui->ctx, view, 3, gui->o_view, 20, nk_vec2(150, 80));
	}
	
	if (gui->step == 0){
		nk_label(gui->ctx, "Enter center point", NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, "Define major axis", NK_TEXT_LEFT);
	}
	else if (gui->step == 2){
		if (gui->el_mode == EL_ISO_CIRCLE || gui->el_mode == EL_ISO_ARC)
			nk_label(gui->ctx, "Define circle radius", NK_TEXT_LEFT);
		else nk_label(gui->ctx, "Define minor axis", NK_TEXT_LEFT);
	}
	else if (gui->step == 3){
		nk_label(gui->ctx, "Enter arc start point", NK_TEXT_LEFT);
	}
	else {
		nk_label(gui->ctx, "Enter arc end point", NK_TEXT_LEFT);
	}
	
	return 1;
}