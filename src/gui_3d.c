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
      "manifold[1]:translate(%.9g,%.9g,%.9g)", gui->radius1,
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
		
		/* define sphere center */
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
    
    //double alpha = atan2(dy, dx) * 180.0/M_PI;
    double gamma = atan2(dz, fabs(dx)) * 180.0/M_PI;
    double beta = atan2(dz, fabs(dy)) * 180.0/M_PI;
    
    
    if(!(gui->user_flag & 1)){
      gui->radius1 = sqrt( pow(gui->step_x[1] - gui->step_x[0], 2) +
        pow(gui->step_y[1] - gui->step_y[0], 2) +
        pow(gui->step_z[1] - gui->step_z[0], 2) );
    }
    
    if(!(gui->user_flag & 2)){
      gui->heigth1 = sqrt( pow(gui->step_x[2] - gui->step_x[0], 2) +
        pow(gui->step_y[2] - gui->step_y[0], 2) +
        pow(gui->step_z[2] - gui->step_z[0], 2) );
    }
    if (gui->step < 3){
      gui->radius2 = gui->radius1;
    }
    else if(!(gui->user_flag & 4)){
      gui->radius2 = sqrt( pow(gui->step_x[3] - gui->step_x[2], 2) +
        pow(gui->step_y[3] - gui->step_y[2], 2) +
        pow(gui->step_z[3] - gui->step_z[2], 2) );
    }
    
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
  
  
  /*
  double dx = gui->step_x[1] - gui->step_x[0];
  double dy = gui->step_y[1] - gui->step_y[0];
  double dz = gui->step_z[1] - gui->step_z[0];
  double len = sqrt(dx*dx + dy*dy + dz*dz);
  
  if (len > 1e-9) {
    dx /= len; dy /= len; dz /= len;
  }
  double alpha = atan2(dy, dx) * 180.0/M_PI;
  double gamma = atan2(dz, fabs(dx)) * 180.0/M_PI;
  double beta = atan2(dz, fabs(dy)) * 180.0/M_PI;
  
  snprintf(user_str_r, 63, "alpha = %.2f", alpha);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  
  snprintf(user_str_r, 63, "beta = %.2f", beta);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  
  snprintf(user_str_r, 63, "gamma = %.2f", gamma);
  nk_label(gui->ctx, user_str_r, NK_TEXT_LEFT);
  */
	
	
	return 1;
}

int gui_pyramid_interactive(gui_obj *gui){
	
	if (gui->modal != PYRAMID) return 0;
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

int gui_pyramid_info (gui_obj *gui){
	if (gui->modal != PYRAMID) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a pyramid"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
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