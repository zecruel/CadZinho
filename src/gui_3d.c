#include "gui_use.h"
#include "dxf_3d.h"

int gui_sphere_interactive(gui_obj *gui){
	
	if (gui->modal != SPHERE) return 0;
  static char cmd[DXF_MAX_CHARS + 1] = "";
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define sphere center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
			gui->step_x[gui->step] = gui->step_x[gui->step - 1];
			gui->step_y[gui->step] = gui->step_y[gui->step - 1];
      gui->step_z[gui->step] = gui->step_z[gui->step - 1];
			/* next step */
			gui->en_distance = 1;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
	else{
    if(!(gui->user_flag & 1)){
    gui->radius1 = sqrt( pow(gui->step_x[gui->step - 1] - gui->step_x[gui->step], 2) +
      pow(gui->step_y[gui->step - 1] - gui->step_y[gui->step], 2) +
      pow(gui->step_z[gui->step - 1] - gui->step_z[gui->step], 2) );
    }
    
    snprintf(cmd, DXF_MAX_CHARS, "manifold[1] = sphere(%.9g)\n"
     // "manifold[1]:translate(%.9g,%.9g,%.9g)",
    "manifold[1]:transform({{1,0,0},{0,1,0},{0,0,1},{%.9g,%.9g,%.9g}})", 
	gui->radius1,
      gui->step_x[gui->step - 1], gui->step_y[gui->step - 1], 
      gui->step_z[gui->step - 1]);
    
	
		if (gui->ev & EV_ENTER){
			/* accept point */
      
      new_el = (dxf_node *) dxf_new_mesh ( cmd, //"manifold = sphere('2')", 
        gui->color_idx, /* color, layer */
        (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
        DWG_LIFE);
      
      
      new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
      drawing_ent_append(gui->drawing, new_el);
      
      do_add_entry(&gui->list_do, _l("SPHERE"));
      do_add_item(gui->list_do.current, NULL, new_el);
      
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
      gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
    }
    else{
      new_el = (dxf_node *) dxf_new_mesh (cmd, //"manifold = sphere('2')", 
        gui->color_idx, /* color, layer */
        (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
        FRAME_LIFE);
      list_node *vec_graph = dxf_graph_parse(gui->drawing, new_el, 0, FRAME_LIFE);
      if (vec_graph){
        gui->phanton = vec_graph;
        gui->draw_phanton = 1;
      }
    }
	}
	
	return 1;
}

int gui_sphere_info (gui_obj *gui){
	if (gui->modal != SPHERE) return 0;
  static char user_str_r[64] = "0.000000";
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a sphere"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius:"), NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->radius1 = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		}
		else if (!(gui->user_flag & 1)) { /* visualize mode */
			
      snprintf(user_str_r, 63, "%.9g", gui->radius1);
			
		}
	}
	
	return 1;
}

int gui_cylinder_interactive(gui_obj *gui){
	
	if (gui->modal != CYLINDER) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define cylinder center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
			gui->step_x[1] = gui->step_x[0];
			gui->step_y[1] = gui->step_y[0];
      gui->step_z[1] = gui->step_z[0];
      
      gui->step_x[2] = gui->step_x[0];
			gui->step_y[2] = gui->step_y[0];
      gui->step_z[2] = gui->step_z[0];
      
      gui->step_x[3] = gui->step_x[0];
			gui->step_y[3] = gui->step_y[0];
      gui->step_z[3] = gui->step_z[0];
			/* next step */
			gui->en_distance = 1;
      
      gui->user_flag |= 4;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
	else{
    double dx = gui->step_x[1] - gui->step_x[0];
    double dy = gui->step_y[1] - gui->step_y[0];
    double dz = gui->step_z[1] - gui->step_z[0];
    double len = sqrt(dx*dx + dy*dy + dz*dz);
    
    if (len > 1e-9) {
      dx /= len; dy /= len; dz /= len;
    }
    
    /* double alpha = atan2(dy, dx); /* angle over z axis */
    double gamma = atan2(dz, fabs(dx)); /* angle over y axis */
    double beta = atan2(dz, fabs(dy)); /* angle over x axis */
    
    double dir[3]; /* height direction - normal to base circle */
    if (fabs(gamma) < fabs(beta)){
      dir[0] = cos(beta) * sin(gamma);
      dir[1] = -sin(beta);
      dir[2] = cos(beta) * cos(gamma);
    } else {
      dir[0] = sin(gamma);
      dir[1] = -sin(beta) * cos(gamma);
      dir[2] = cos(beta) * cos(gamma);
    }
    
    /*alpha *= 180.0/M_PI;*/
    gamma *= 180.0/M_PI;
    beta *= 180.0/M_PI;
    
    if(!(gui->user_flag & 1)){ /* base radius */
      gui->radius1 = sqrt( pow(gui->step_x[1] - gui->step_x[0], 2) +
        pow(gui->step_y[1] - gui->step_y[0], 2) +
        pow(gui->step_z[1] - gui->step_z[0], 2) );
    }
    
    if(!(gui->user_flag & 2)){ /* heigtht */
      dx = gui->step_x[2] - gui->step_x[0];
      dy = gui->step_y[2] - gui->step_y[0];
      dz = gui->step_z[2] - gui->step_z[0];
      
      gui->heigth1 = dir[0]*dx + dir[1]*dy + dir[2]*dz;
      
      gui->step_x[2] = gui->step_x[0] + gui->heigth1 * dir[0];
      gui->step_y[2] = gui->step_y[0] + gui->heigth1 * dir[1];
      gui->step_z[2] = gui->step_z[0] + gui->heigth1 * dir[2];
      
      if (gui->heigth1 < 0.0){
        if (fabs(gamma) > fabs(beta)) gamma += 180.0;
        else beta += 180.0;
        gui->heigth1 *= -1.0;
      }
    }
    /* top radius */
    if (gui->step < 3){
      gui->radius2 = gui->radius1;
    }
    else if(!(gui->user_flag & 4)){
      gui->radius2 = sqrt( pow(gui->step_x[3] - gui->step_x[2], 2) +
        pow(gui->step_y[3] - gui->step_y[2], 2) +
        pow(gui->step_z[3] - gui->step_z[2], 2) );
    }
    
    /* create manifold */
    
    if (fabs(gamma) > fabs(beta))
      snprintf(cmd, 1000, "manifold[1] = cylinder(%.9g,%.9g,%.9g)\n"
        "manifold[1]:rotate(0,%.9g,0)\n"
        "manifold[1]:rotate(%.9g,0,0)\n"
        "manifold[1]:translate(%.9g,%.9g,%.9g)",
        gui->heigth1, gui->radius1, gui->radius2,
        gamma, beta,
        gui->step_x[0], gui->step_y[0], gui->step_z[0]);
    else
      snprintf(cmd, 1000, "manifold[1] = cylinder(%.9g,%.9g,%.9g)\n"
        "manifold[1]:rotate(%.9g,0,0)\n"
        "manifold[1]:rotate(0,%.9g,0)\n"
        "manifold[1]:translate(%.9g,%.9g,%.9g)",
        gui->heigth1, gui->radius1, gui->radius2,
        beta, gamma,
        gui->step_x[0], gui->step_y[0], gui->step_z[0]);
    
    new_el = (dxf_node *) dxf_new_mesh (cmd,
      gui->color_idx, /* color, layer */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
      FRAME_LIFE);
    list_node *vec_graph = dxf_graph_parse(gui->drawing, new_el, 0, FRAME_LIFE);
    if (vec_graph){
      gui->phanton = vec_graph;
      gui->draw_phanton = 1;
    }
    
		if (gui->ev & EV_ENTER){
			/* accept point */
      if (gui->step < 3){
        gui->step++;
      }
      else{
        /* add cylinder to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("CYLINDER"));
        do_add_item(gui->list_do.current, NULL, new_el);
        
        gui->draw_phanton = 0;
        gui->phanton = NULL;
        gui_first_step(gui);
      }
		}
		else if (gui->ev & EV_CANCEL){
      gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
    }
    else{
      
    }
	}
	
	return 1;
}

int gui_cylinder_info (gui_obj *gui){
	if (gui->modal != CYLINDER) return 0;
  static char user_str_r[64] = "0.000000";
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a cylinder"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define base radius:"), NK_TEXT_LEFT);
    snprintf(user_str_r, 63, "%.9g", gui->radius1);
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->radius1 = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    snprintf(user_str_r, 63, _l("Radius: %.9g"), gui->radius1);
    nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
    nk_label(gui->ctx, _l("Define heigth:"), NK_TEXT_LEFT);
    snprintf(user_str_r, 63, "%.9g", gui->heigth1);
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->heigth1 = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 3){
    snprintf(user_str_r, 63, _l("Radius: %.9g"), gui->radius1);
    nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
    snprintf(user_str_r, 63, _l("Heigth: %.9g"),  gui->heigth1);
    nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
    nk_label(gui->ctx, _l("Define top radius:"), NK_TEXT_LEFT);
    
    snprintf(user_str_r, 63, "%.9g", gui->radius2);
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->radius2 = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  }
  
  
  #if(0)
  double dx = gui->step_x[1] - gui->step_x[0];
  double dy = gui->step_y[1] - gui->step_y[0];
  double dz = gui->step_z[1] - gui->step_z[0];
  double len = sqrt(dx*dx + dy*dy + dz*dz);
  
  if (len > 1e-9) {
    dx /= len; dy /= len; dz /= len;
  }
  double alpha = atan2(dy, dx); /* angle over z axis */
  double gamma = atan2(dz, fabs(dx)); /* angle over y axis */
  double beta = atan2(dz, fabs(dy)); /* angle over x axis */
  
  double dir[3];
  dir[0] = cos(beta) * sin(gamma);
  dir[1] = -sin(beta);
  dir[2] = cos(beta) * cos(gamma);
  
  alpha *= 180.0/M_PI;
  gamma *= 180.0/M_PI;
  beta *= 180.0/M_PI;
  
  snprintf(user_str_r, 63, "alpha = %.2f", alpha);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  
  snprintf(user_str_r, 63, "beta = %.2f", beta);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  
  snprintf(user_str_r, 63, "gamma = %.2f", gamma);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  
  snprintf(user_str_r, 63, "dir=[%.2f , %.2f , %.2f]", dir[0], dir[1], dir[2]);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  #endif
	
	
	return 1;
}

int gui_pyramid_interactive(gui_obj *gui){
	
	if (gui->modal != PYRAMID) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define pyramid ref point */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
			gui->step_x[1] = gui->step_x[0];
			gui->step_y[1] = gui->step_y[0];
      gui->step_z[1] = gui->step_z[0];
      
      gui->step_x[2] = gui->step_x[0];
			gui->step_y[2] = gui->step_y[0];
      gui->step_z[2] = gui->step_z[0];
      
      gui->step_x[3] = gui->step_x[0];
			gui->step_y[3] = gui->step_y[0];
      gui->step_z[3] = gui->step_z[0];
      
      gui->step_x[4] = gui->step_x[0];
			gui->step_y[4] = gui->step_y[0];
      gui->step_z[4] = gui->step_z[0];
			/* next step */
			gui->en_distance = 1;
      
      gui->user_flag |= 8;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	} 
  else if (gui->step == 1){
    double x_dir_x = gui->step_x[1] - gui->step_x[0];
    double x_dir_y = gui->step_y[1] - gui->step_y[0];
    double x_dir_z = gui->step_z[1] - gui->step_z[0];
    double width = sqrt(x_dir_x*x_dir_x + x_dir_y*x_dir_y + x_dir_z*x_dir_z);
    
    if (width > 1e-9) {
      x_dir_x /= width; x_dir_y /= width; x_dir_z /= width;
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->radius1 = width;
    }
    
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
			
			line_add(graph, gui->step_x[0], gui->step_y[0], gui->step_z[0],
				gui->step_x[0] + gui->radius1 * x_dir_x,
        gui->step_y[0] + gui->radius1 * x_dir_y,
        gui->step_z[0] + gui->radius1 * x_dir_z);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		/* define pyramid ref point */
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (width > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->radius1 * x_dir_x;
			gui->step_y[1] = gui->step_y[0] + gui->radius1 * x_dir_y;
      gui->step_z[1] = gui->step_z[0] + gui->radius1 * x_dir_z;
      
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
	else{
    double x_dir_x = gui->step_x[1] - gui->step_x[0];
    double x_dir_y = gui->step_y[1] - gui->step_y[0];
    double x_dir_z = gui->step_z[1] - gui->step_z[0];
    double width = sqrt(x_dir_x*x_dir_x + x_dir_y*x_dir_y + x_dir_z*x_dir_z);
    
    if (width > 1e-9) {
      x_dir_x /= width; x_dir_y /= width; x_dir_z /= width;
    }
    /* get next point in plane perpenticular to base width */
    double d = -x_dir_x * gui->step_x[1] - x_dir_y * gui->step_y[1] - x_dir_z * gui->step_z[1];
    double nq = x_dir_x * gui->step_x[2] + x_dir_y * gui->step_y[2] + x_dir_z * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * x_dir_x;
    double py = gui->step_y[2] - (nq + d) * x_dir_y;
    double pz = gui->step_z[2] - (nq + d) * x_dir_z;
    
    double y_dir_x = px - gui->step_x[1];
    double y_dir_y = py - gui->step_y[1];
    double y_dir_z = pz - gui->step_z[1];
    double height = sqrt(y_dir_x*y_dir_x + y_dir_y*y_dir_y + y_dir_z*y_dir_z);
    
    if (height > 1e-9) {
      y_dir_x /= height; y_dir_y /= height; y_dir_z /= height;
    }
    /* cross product to get z direction */
    double z_dir_x = x_dir_y*y_dir_z - x_dir_z*y_dir_y;
    double z_dir_y = x_dir_z*y_dir_x - x_dir_x*y_dir_z;
    double z_dir_z = x_dir_x*y_dir_y - x_dir_y*y_dir_x;
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->radius1 = 2.0 * width;
    }
    
    if(!(gui->user_flag & 2)){ /* base heigtht */
      gui->heigth1 = 2.0 * height;
    }
    
    if(!(gui->user_flag & 4)){ /* pyramid height */
      double dx = gui->step_x[3] - gui->step_x[0];
      double dy = gui->step_y[3] - gui->step_y[0];
      double dz = gui->step_z[3] - gui->step_z[0];
      
      gui->heigth2 = z_dir_x*dx + z_dir_y*dy + z_dir_z*dz;
    }
    
    if (gui->heigth2 < 0.0){
      gui->heigth2 *= -1.0;
      /* rotate 180 degrees in x axis */
      y_dir_x *= -1; y_dir_y *= -1; y_dir_z *= -1;
      z_dir_x *= -1; z_dir_y *= -1; z_dir_z *= -1;
    }
    
    gui->step_x[3] = gui->step_x[0] + gui->heigth2 * z_dir_x;
    gui->step_y[3] = gui->step_y[0] + gui->heigth2 * z_dir_y;
    gui->step_z[3] = gui->step_z[0] + gui->heigth2 * z_dir_z;
    
    /* top scale factor */
    if (gui->step < 4){
      gui->radius2 = 0.0;
    }
    else if(!(gui->user_flag & 8)){
      double w = (gui->radius1 > 1e-9) ? gui->radius1 : 1.0;
      gui->radius2 = sqrt( pow(gui->step_x[4] - gui->step_x[3], 2) +
        pow(gui->step_y[4] - gui->step_y[3], 2) +
        pow(gui->step_z[4] - gui->step_z[3], 2) ) / w;
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = pyramid(%.9g,%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      gui->radius1,gui->heigth1, gui->heigth2, gui->radius2,
      x_dir_x, x_dir_y, x_dir_z, y_dir_x, y_dir_y, y_dir_z,
      z_dir_x, z_dir_y, z_dir_z,
      gui->step_x[0], gui->step_y[0], gui->step_z[0]);
   
    
    new_el = (dxf_node *) dxf_new_mesh (cmd,
      gui->color_idx, /* color, layer */
      (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
      FRAME_LIFE);
    list_node *vec_graph = dxf_graph_parse(gui->drawing, new_el, 0, FRAME_LIFE);
    if (vec_graph){
      gui->phanton = vec_graph;
      gui->draw_phanton = 1;
    }
    
		if (gui->ev & EV_ENTER){
			/* accept point */
      if (gui->step < 4 && width > 1e-9 && height > 1e-9) {
        gui->step++;
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
      }
      else{
        /* add pyramid to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("PYRAMID"));
        do_add_item(gui->list_do.current, NULL, new_el);
        
        gui->draw_phanton = 0;
        gui->phanton = NULL;
        gui_first_step(gui);
      }
		}
		else if (gui->ev & EV_CANCEL){
      gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
    }
	}
	
	return 1;
}

int gui_pyramid_info (gui_obj *gui){
	if (gui->modal != PYRAMID) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("Base"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("Top"), DXF_MAX_CHARS);
  
  char *mode_addr[] = {mode[0], mode[1]};
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a pyramid"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->el_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->radius1);
		nk_label(gui->ctx, _l("Define base width:"), NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->radius1 = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->radius1);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->heigth1);
    snprintf(tmp_str, 63, _l("Base width: %.9g"), gui->radius1);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, _l("Define base heigth:"), NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->heigth1 = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->heigth1);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 3){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->heigth2);
    snprintf(tmp_str, 63, _l("Base width: %.9g"), gui->radius1);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, _l("Base heigth: %.9g"),  gui->heigth1);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, _l("Define top heigth:"), NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->heigth2 = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->heigth2);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->radius2);
    snprintf(tmp_str, 63, _l("Base width: %.9g"), gui->radius1);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, _l("Base heigth: %.9g"),  gui->heigth1);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, _l("Top heigth: %.9g"),  gui->heigth2);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    
    nk_label(gui->ctx, _l("Top scale factor:"), NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->radius2 = atof(user_str_r);
				gui->user_flag |= 8;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~8;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 8)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->radius2);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
	}
  
  prev_step = gui->step;
  
	return 1;
}

int gui_slab_interactive(gui_obj *gui){
	
	if (gui->modal != SLAB) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_slab_info (gui_obj *gui){
	if (gui->modal != SLAB) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a slab"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_extrude_interactive(gui_obj *gui){
	
	if (gui->modal != EXTRUDE) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_extrude_info (gui_obj *gui){
	if (gui->modal != EXTRUDE) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a extrude"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_extrude_path_interactive(gui_obj *gui){
	
	if (gui->modal != EXTRUDE_PATH) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_extrude_path_info (gui_obj *gui){
	if (gui->modal != EXTRUDE_PATH) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a extrude_path"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_revolve_interactive(gui_obj *gui){
	
	if (gui->modal != REVOLVE) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_revolve_info (gui_obj *gui){
	if (gui->modal != REVOLVE) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a revolve"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_union_interactive(gui_obj *gui){
	
	if (gui->modal != UNION) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_union_info (gui_obj *gui){
	if (gui->modal != UNION) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a union"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_subtract_interactive(gui_obj *gui){
	
	if (gui->modal != SUBTRACT) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_subtract_info (gui_obj *gui){
	if (gui->modal != SUBTRACT) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a subtract"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_intersection_interactive(gui_obj *gui){
	
	if (gui->modal != INTERSECTION) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_intersection_info (gui_obj *gui){
	if (gui->modal != INTERSECTION) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a intersection"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_slice_interactive(gui_obj *gui){
	
	if (gui->modal != SLICE) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_slice_info (gui_obj *gui){
	if (gui->modal != SLICE) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a slice"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_rotate_3d_interactive(gui_obj *gui){
	
	if (gui->modal != ROTATE_3D) return 0;
	static double start = 0.0, end = 0.0;
	static double ratio = 1.0, major = 0.0;
	static double rot = 0.0, sine = 0.0, cosine = 1.0;
	static double step1_x, step1_y;
	
	static dxf_node *new_el;
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define ellipse center */
		if (gui->ev & EV_ENTER){
			/* accept point */
			gui->step = 1;
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
	else{
		
		gui->draw_phanton = 0;
		//gui_first_step(gui);
	}
	
	return 1;
}

int gui_rotate_3d_info (gui_obj *gui){
	if (gui->modal != ROTATE_3D) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a rotate_3d"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}