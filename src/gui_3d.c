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
  
  int x = 0, y = 1, z = 2;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of cylinder */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2;
  } else {
    x = 2; y = 0; z = 1;
  }
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define base center point */
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
      
      gui->user_flag |= 4;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
  else if (gui->step == 1){
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
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
				gui->step_x[0] + gui->param_3d[x] * matrix[x][0],
        gui->step_y[0] + gui->param_3d[x] * matrix[x][1],
        gui->step_z[0] + gui->param_3d[x] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[x] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
      
      gui->step_x[3] = gui->step_x[1];
			gui->step_y[3] = gui->step_y[1];
      gui->step_z[3] = gui->step_z[1];
      
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
    }
    
    if(!(gui->user_flag & 2)){ /* base height */
      gui->param_3d[y] = size[y];
    }
    
    double dx = gui->step_x[3] - gui->step_x[0];
    double dy = gui->step_y[3] - gui->step_y[0];
    double dz = gui->step_z[3] - gui->step_z[0];
    
    size[z] = matrix[z][0]*dx + matrix[z][1]*dy + matrix[z][2]*dz;
    
    if(!(gui->user_flag & 2)){ /* cylinder height */
      gui->param_3d[z] = size[z];
    }
    
    if (gui->param_3d[z] < 0.0){
      gui->param_3d[z] *= -1.0;
      /* rotate 180 degrees in x axis */
      matrix[y][0] *= -1; matrix[y][1] *= -1; matrix[y][2] *= -1;
      matrix[z][0] *= -1; matrix[z][1] *= -1; matrix[z][2] *= -1;
    }
    
    gui->step_x[3] = gui->step_x[0] + gui->param_3d[z] * matrix[z][0];
    gui->step_y[3] = gui->step_y[0] + gui->param_3d[z] * matrix[z][1];
    gui->step_z[3] = gui->step_z[0] + gui->param_3d[z] * matrix[z][2];
    
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = cylinder(%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      gui->param_3d[2], gui->param_3d[0],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 3 && size[x] > 1e-9 ) {
        if (gui->extr_mode == E3D_TOP){
          gui->step = 3;
          gui->step_x[gui->step] = gui->step_x[gui->step - 1];
          gui->step_y[gui->step] = gui->step_y[gui->step - 1];
          gui->step_z[gui->step] = gui->step_z[gui->step - 1];
          gui->step = 4;
        } else {
          gui->step++;
          
        }
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
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
	}
	
	return 1;
}

int gui_cylinder_info (gui_obj *gui){
	if (gui->modal != CYLINDER) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by top direction"), DXF_MAX_CHARS);
  
  const char *text_define[3];
  text_define[0] = _l("Define base radius:");
  text_define[1] = _l("Define base plane:");
  text_define[2] = _l("Define height:");
  
  const char *text_info[3];
  text_info[0] = _l("Base radius: %.9g");
  text_info[1] = _l("Base plane: %.9g");
  text_info[2] = _l("Height: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2;
  } else {
    x = 2; y = 0; z = 1;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a cylinder"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    if (gui->extr_mode == E3D_TOP){
      /* edit to visualize or enter heigth */
      nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
        NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
        user_str_r, 63, nk_filter_float);
      if (res & NK_EDIT_ACTIVE){ /* enter mode */
        if (strlen(user_str_r)){
          /* sinalize the radius of user entry */
          gui->param_3d[y] = atof(user_str_r);
          gui->user_flag |= 2;
        }
        else{ /* if the user clear the string */
          /* cancel the enter mode*/
          gui->user_flag &= ~2;
          nk_edit_unfocus(gui->ctx);
        }
      } else if (!(gui->user_flag & 2)) { /* visualize mode */
        snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
      }
      if (res & NK_EDIT_COMMITED){
        nk_edit_unfocus(gui->ctx);
      }
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  }
  
  prev_step = gui->step;
  
	return 1;
}

int gui_cone_interactive(gui_obj *gui){
	if (gui->modal != CONE) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
  
  int x = 0, y = 1, z = 2, s = 3;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of cone */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define base center point */
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
      
      gui->user_flag |= 4;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
  else if (gui->step == 1){
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
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
				gui->step_x[0] + gui->param_3d[x] * matrix[x][0],
        gui->step_y[0] + gui->param_3d[x] * matrix[x][1],
        gui->step_z[0] + gui->param_3d[x] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[x] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
      
      gui->step_x[3] = gui->step_x[1];
			gui->step_y[3] = gui->step_y[1];
      gui->step_z[3] = gui->step_z[1];
      
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
    }
    
    if(!(gui->user_flag & 2)){ /* base height */
      gui->param_3d[y] = size[y];
    }
    
    double dx = gui->step_x[3] - gui->step_x[0];
    double dy = gui->step_y[3] - gui->step_y[0];
    double dz = gui->step_z[3] - gui->step_z[0];
    
    size[z] = matrix[z][0]*dx + matrix[z][1]*dy + matrix[z][2]*dz;
    
    if(!(gui->user_flag & 2)){ /* cone height */
      gui->param_3d[z] = size[z];
    }
    
    if (gui->param_3d[z] < 0.0){
      gui->param_3d[z] *= -1.0;
      /* rotate 180 degrees in x axis */
      matrix[y][0] *= -1; matrix[y][1] *= -1; matrix[y][2] *= -1;
      matrix[z][0] *= -1; matrix[z][1] *= -1; matrix[z][2] *= -1;
    }
    
    /* top scale factor */
    if (gui->step < 4){
      //gui->param_3d[s] = gui->param_3d[0];
      gui->param_3d[s] = 0.0;
    }
    else if(!(gui->user_flag & 4)){
      
      gui->step_x[3] = gui->step_x[0] + gui->param_3d[2] * matrix[2][0];
      gui->step_y[3] = gui->step_y[0] + gui->param_3d[2] * matrix[2][1];
      gui->step_z[3] = gui->step_z[0] + gui->param_3d[2] * matrix[2][2];
      
      /* get next point in plane paralel to base */
      dx = gui->step_x[4] - gui->step_x[3];
      dy = gui->step_y[4] - gui->step_y[3];
      dz = gui->step_z[4] - gui->step_z[3];
      
      px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
      py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
      gui->param_3d[s] = sqrt(px*px + py*py);
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = cone(%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      gui->param_3d[2], gui->param_3d[0], gui->param_3d[3],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 4 && size[x] > 1e-9 ) {
        if (gui->extr_mode == E3D_TOP){
          gui->step = 3;
          gui->step_x[gui->step] = gui->step_x[gui->step - 1];
          gui->step_y[gui->step] = gui->step_y[gui->step - 1];
          gui->step_z[gui->step] = gui->step_z[gui->step - 1];
          gui->step = 4;
        } else {
          gui->step++;
          
        }
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
      }
      else{
        /* add cone to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("CONE"));
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

int gui_cone_info (gui_obj *gui){
	if (gui->modal != CONE) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by top direction"), DXF_MAX_CHARS);
  
  const char *text_define[4];
  text_define[0] = _l("Define base radius:");
  text_define[1] = _l("Define base plane:");
  text_define[2] = _l("Define height:");
  text_define[3] = _l("Define top radius:");
  
  const char *text_info[4];
  text_info[0] = _l("Base radius: %.9g");
  text_info[1] = _l("Base plane: %.9g");
  text_info[2] = _l("Height: %.9g");
  text_info[3] = _l("Top radius: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2, s = 3;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a cone"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    if (gui->extr_mode == E3D_TOP){
      /* edit to visualize or enter heigth */
      nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
        NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
        user_str_r, 63, nk_filter_float);
      if (res & NK_EDIT_ACTIVE){ /* enter mode */
        if (strlen(user_str_r)){
          /* sinalize the radius of user entry */
          gui->param_3d[y] = atof(user_str_r);
          gui->user_flag |= 2;
        }
        else{ /* if the user clear the string */
          /* cancel the enter mode*/
          gui->user_flag &= ~2;
          nk_edit_unfocus(gui->ctx);
        }
      } else if (!(gui->user_flag & 2)) { /* visualize mode */
        snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
      }
      if (res & NK_EDIT_COMMITED){
        nk_edit_unfocus(gui->ctx);
      }
    }
  } else if (gui->step == 3){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    if (gui->extr_mode == E3D_TOP)
      snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    else
      snprintf(tmp_str, 63, text_info[z],  gui->param_3d[z]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    
    nk_label(gui->ctx, text_define[s], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[s] = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
	}
  
  prev_step = gui->step;
  
	return 1;
}

int gui_pyramid_interactive(gui_obj *gui){
	if (gui->modal != PYRAMID) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
  
  int x = 0, y = 1, z = 2, s = 3;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of pyramid */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
	
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
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
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
				gui->step_x[0] + gui->param_3d[x] * matrix[x][0],
        gui->step_y[0] + gui->param_3d[x] * matrix[x][1],
        gui->step_z[0] + gui->param_3d[x] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[x] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
    }
    
    if(!(gui->user_flag & 2)){ /* base height */
      gui->param_3d[y] = size[y];
    }
    
    double dx = gui->step_x[3] - gui->step_x[0];
    double dy = gui->step_y[3] - gui->step_y[0];
    double dz = gui->step_z[3] - gui->step_z[0];
    
    size[z] = matrix[z][0]*dx + matrix[z][1]*dy + matrix[z][2]*dz;
    
    if(!(gui->user_flag & 4)){ /* pyramid height */
      gui->param_3d[z] = size[z];
    }
    
    if (gui->param_3d[z] < 0.0){
      gui->param_3d[z] *= -1.0;
      /* rotate 180 degrees in x axis */
      matrix[y][0] *= -1; matrix[y][1] *= -1; matrix[y][2] *= -1;
      matrix[z][0] *= -1; matrix[z][1] *= -1; matrix[z][2] *= -1;
    }
    
    /* top scale factor */
    if (gui->step < 4){
      gui->param_3d[s] = 0.0;
    }
    else if(!(gui->user_flag & 8)){
      double w = (gui->param_3d[0] > 1e-9) ? gui->param_3d[0] : 1.0;
      double h = (gui->param_3d[1] > 1e-9) ? gui->param_3d[1] : 1.0;
      gui->step_x[3] = gui->step_x[0] + gui->param_3d[2] * matrix[2][0];
      gui->step_y[3] = gui->step_y[0] + gui->param_3d[2] * matrix[2][1];
      gui->step_z[3] = gui->step_z[0] + gui->param_3d[2] * matrix[2][2];
      
      /* get next point in plane paralel to base */
      dx = gui->step_x[4] - gui->step_x[3];
      dy = gui->step_y[4] - gui->step_y[3];
      dz = gui->step_z[4] - gui->step_z[3];
      
      px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
      py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
      
      double sw = fabs(px / w);
      double sh = fabs(py / h);
      
      gui->param_3d[s] = (sw > sh) ? sw : sh;
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = pyramid(%.9g,%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      2.0 * gui->param_3d[0],2.0 * gui->param_3d[1],
      gui->param_3d[2], gui->param_3d[3],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 4 && size[x] > 1e-9 && size[y] > 1e-9) {
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
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by top direction"), DXF_MAX_CHARS);
  
  const char *text_define[4];
  text_define[0] = _l("Define base width:");
  text_define[1] = _l("Define base height:");
  text_define[2] = _l("Define pyramid height:");
  text_define[3] = _l("Define top factor:");
  
  const char *text_info[4];
  text_info[0] = _l("Base width: %.9g");
  text_info[1] = _l("Base height: %.9g");
  text_info[2] = _l("Pyramid height: %.9g");
  text_info[3] = _l("Top factor: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2, s = 3;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a pyramid"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[y] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 3){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[z],  gui->param_3d[z]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    
    nk_label(gui->ctx, text_define[s], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[s] = atof(user_str_r);
				gui->user_flag |= 8;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~8;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 8)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
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
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
  
  int x = 0, y = 1, z = 2, s = 3;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of slab */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define slab ref point */
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
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
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
				gui->step_x[0] + gui->param_3d[x] * matrix[x][0],
        gui->step_y[0] + gui->param_3d[x] * matrix[x][1],
        gui->step_z[0] + gui->param_3d[x] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[x] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
    }
    
    if(!(gui->user_flag & 2)){ /* base height */
      gui->param_3d[y] = size[y];
    }
    
    double dx = gui->step_x[3] - gui->step_x[0];
    double dy = gui->step_y[3] - gui->step_y[0];
    double dz = gui->step_z[3] - gui->step_z[0];
    
    size[z] = matrix[z][0]*dx + matrix[z][1]*dy + matrix[z][2]*dz;
    
    if(!(gui->user_flag & 4)){ /* slab height */
      gui->param_3d[z] = size[z];
    }
    
    if (gui->param_3d[z] < 0.0){
      gui->param_3d[z] *= -1.0;
      /* rotate 180 degrees in x axis */
      matrix[y][0] *= -1; matrix[y][1] *= -1; matrix[y][2] *= -1;
      matrix[z][0] *= -1; matrix[z][1] *= -1; matrix[z][2] *= -1;
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = slab(%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      2.0 * gui->param_3d[0], 2.0 * gui->param_3d[1], gui->param_3d[2],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 3 && size[x] > 1e-9 && size[y] > 1e-9) {
        gui->step++;
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
      }
      else{
        /* add slab to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("SLAB"));
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

int gui_slab_info (gui_obj *gui){
	if (gui->modal != SLAB) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by top direction"), DXF_MAX_CHARS);
  
  const char *text_define[3];
  text_define[0] = _l("Define base width:");
  text_define[1] = _l("Define base height:");
  text_define[2] = _l("Define slab height:");
  
  const char *text_info[3];
  text_info[0] = _l("Base width: %.9g");
  text_info[1] = _l("Base height: %.9g");
  text_info[2] = _l("Slab height: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2, s = 3;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a slab"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[y] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  }
  
  prev_step = gui->step;
  
	return 1;
}

int gui_wedge_interactive(gui_obj *gui){
	if (gui->modal != WEDGE) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
  
  int x = 0, y = 1, z = 2, s = 3;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of wedge */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define wedge ref point */
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
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
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
				gui->step_x[0] + gui->param_3d[x] * matrix[x][0],
        gui->step_y[0] + gui->param_3d[x] * matrix[x][1],
        gui->step_z[0] + gui->param_3d[x] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[x] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[x] = size[x];
    }
    
    if(!(gui->user_flag & 2)){ /* base height */
      gui->param_3d[y] = size[y];
    }
    
    double dx = gui->step_x[3] - gui->step_x[0];
    double dy = gui->step_y[3] - gui->step_y[0];
    double dz = gui->step_z[3] - gui->step_z[0];
    
    size[z] = matrix[z][0]*dx + matrix[z][1]*dy + matrix[z][2]*dz;
    
    if(!(gui->user_flag & 4)){ /* wedge height */
      gui->param_3d[z] = size[z];
    }
    
    if (gui->param_3d[z] < 0.0){
      gui->param_3d[z] *= -1.0;
      /* rotate 180 degrees in x axis */
      matrix[y][0] *= -1; matrix[y][1] *= -1; matrix[y][2] *= -1;
      matrix[z][0] *= -1; matrix[z][1] *= -1; matrix[z][2] *= -1;
    }
    
    /* top scale factor */
    if (gui->step < 4){
      gui->param_3d[3] = 1.0;
      gui->param_3d[4] = 0.0;
    }
    else if(!(gui->user_flag & 8)){
      double w = (gui->param_3d[0] > 1e-9) ? gui->param_3d[0] : 1.0;
      double h = (gui->param_3d[1] > 1e-9) ? gui->param_3d[1] : 1.0;
      gui->step_x[3] = gui->step_x[0] + gui->param_3d[2] * matrix[2][0];
      gui->step_y[3] = gui->step_y[0] + gui->param_3d[2] * matrix[2][1];
      gui->step_z[3] = gui->step_z[0] + gui->param_3d[2] * matrix[2][2];
      
      /* get next point in plane paralel to base */
      dx = gui->step_x[4] - gui->step_x[3];
      dy = gui->step_y[4] - gui->step_y[3];
      dz = gui->step_z[4] - gui->step_z[3];
      
      px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
      py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
      
      gui->param_3d[3] = fabs(px / w);
      gui->param_3d[4] = fabs(py / h);
      
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = wedge(%.9g,%.9g,%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      2.0 * gui->param_3d[0],2.0 * gui->param_3d[1],
      gui->param_3d[2], gui->param_3d[3], gui->param_3d[4],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 4 && size[x] > 1e-9 && size[y] > 1e-9) {
        gui->step++;
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
      }
      else{
        /* add wedge to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("WEDGE"));
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

int gui_wedge_info (gui_obj *gui){
	if (gui->modal != WEDGE) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static char user_str_s[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by top direction"), DXF_MAX_CHARS);
  
  const char *text_define[4];
  text_define[0] = _l("Define base width:");
  text_define[1] = _l("Define base height:");
  text_define[2] = _l("Define wedge height:");
  text_define[3] = _l("Define top factors:");
  
  const char *text_info[4];
  text_info[0] = _l("Base width: %.9g");
  text_info[1] = _l("Base height: %.9g");
  text_info[2] = _l("Wedge height: %.9g");
  text_info[3] = _l("Top factor: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2, s = 3;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a wedge"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[y] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 3){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    snprintf(tmp_str, 63, text_info[z],  gui->param_3d[z]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    
    nk_label(gui->ctx, text_define[s], NK_TEXT_LEFT);
    
    /* edits to visualize or enter top scale factors */
    nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.2, 0.8});
    nk_label(gui->ctx, _l("X:"), NK_TEXT_LEFT);
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[3] = atof(user_str_r);
				gui->user_flag |= 8;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~8;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 8)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[3]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
    
    nk_label(gui->ctx, _l("Y:"), NK_TEXT_LEFT);
		res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_s, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_s)){
				/* sinalize the radius of user entry */
				gui->param_3d[4] = atof(user_str_s);
				gui->user_flag |= 8;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~8;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 8)) { /* visualize mode */
      snprintf(user_str_s, 63, "%.9g", gui->param_3d[4]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
	}
  
  prev_step = gui->step;
  
	return 1;
}

int gui_torus_interactive(gui_obj *gui){
	if (gui->modal != TORUS) return 0;
	static char cmd[1001] = "";
	
	static dxf_node *new_el;
  
  int x = 0, y = 1, z = 2, s = 3;
  double matrix[3][3] = {{1.0,0.0,0.0},
    {0.0,1.0,0.0},{0.0,0.0,1.0}};
  double size[3] = {1,1,1}; /* sizes of torus */
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 1; z = 0; s = 3;
  }
	
	gui->draw_phanton = 0;
  gui->phanton = NULL;
	if (gui->step == 0){
		gui->free_sel = 0;
		
		/* define base center point */
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
      
      gui->user_flag |= 4;
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui_default_modal(gui);
		}
	}
  else if (gui->step == 1){
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[1] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[1] > 1e-9) {
      matrix[x][0] /= size[1];
      matrix[x][1] /= size[1];
      matrix[x][2] /= size[1];
    }
    if (gui->extr_mode == E3D_BASE){
      if(!(gui->user_flag & 1)){ /* base width */
        gui->param_3d[0] = size[1];
      } else size[1] = gui->param_3d[0];
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
				gui->step_x[0] + size[1] * matrix[x][0],
        gui->step_y[0] + size[1] * matrix[x][1],
        gui->step_z[0] + size[1] * matrix[x][2]);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
    
		if (gui->ev & EV_ENTER){
			/* accept point */
			if (size[1] > 1e-9) gui->step = 2;
      
      gui->step_x[1] = gui->step_x[0] + gui->param_3d[x] * matrix[x][0];
			gui->step_y[1] = gui->step_y[0] + gui->param_3d[x] * matrix[x][1];
      gui->step_z[1] = gui->step_z[0] + gui->param_3d[x] * matrix[x][2];
      
      gui->step_x[2] = gui->step_x[1];
			gui->step_y[2] = gui->step_y[1];
      gui->step_z[2] = gui->step_z[1];
      
      gui->step_x[3] = gui->step_x[1];
			gui->step_y[3] = gui->step_y[1];
      gui->step_z[3] = gui->step_z[1];
      
			gui_next_step(gui);
		}
		else if (gui->ev & EV_CANCEL){
			gui->draw_phanton = 0;
      gui->phanton = NULL;
      gui_first_step(gui);
		}
	}
	else{
    matrix[x][0] = gui->step_x[1] - gui->step_x[0];
    matrix[x][1] = gui->step_y[1] - gui->step_y[0];
    matrix[x][2] = gui->step_z[1] - gui->step_z[0];
    size[x] = sqrt(matrix[x][0]*matrix[x][0] + 
      matrix[x][1]*matrix[x][1] + matrix[x][2]*matrix[x][2]);
    
    if (size[x] > 1e-9) {
      matrix[x][0] /= size[x];
      matrix[x][1] /= size[x];
      matrix[x][2] /= size[x];
    }
    /* get next point in plane perpenticular to direction */
    double d = -matrix[x][0] * gui->step_x[1] 
      - matrix[x][1] * gui->step_y[1]
      - matrix[x][2] * gui->step_z[1];
    double nq = matrix[x][0] * gui->step_x[2] + 
      matrix[x][1] * gui->step_y[2] + 
      matrix[x][2] * gui->step_z[2];
    
    double px = gui->step_x[2] - (nq + d) * matrix[x][0];
    double py = gui->step_y[2] - (nq + d) * matrix[x][1];
    double pz = gui->step_z[2] - (nq + d) * matrix[x][2];
    
    matrix[y][0] = px - gui->step_x[1];
    matrix[y][1] = py - gui->step_y[1];
    matrix[y][2] = pz - gui->step_z[1];
    size[y] = sqrt(matrix[y][0]*matrix[y][0] +
      matrix[y][1]*matrix[y][1] + matrix[y][2]*matrix[y][2]);
    
    if (size[y] > 1e-9) {
      matrix[y][0] /= size[y];
      matrix[y][1] /= size[y];
      matrix[y][2] /= size[y];
    }
    /* cross product to get z direction */
    matrix[z][0] = matrix[x][1]*matrix[y][2] - matrix[x][2]*matrix[y][1];
    matrix[z][1] = matrix[x][2]*matrix[y][0] - matrix[x][0]*matrix[y][2];
    matrix[z][2] = matrix[x][0]*matrix[y][1] - matrix[x][1]*matrix[y][0];
    
    double dx = gui->step_x[1] - gui->step_x[0];
    double dy = gui->step_y[1] - gui->step_y[0];
    double dz = gui->step_z[1] - gui->step_z[0];
    if (gui->extr_mode != E3D_BASE){
      dx = gui->step_x[2] - gui->step_x[0];
      dy = gui->step_y[2] - gui->step_y[0];
      dz = gui->step_z[2] - gui->step_z[0];
    }
    px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
    py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
    size[1] = sqrt(px*px + py*py);
    
    if(!(gui->user_flag & 1)){ /* base width */
      gui->param_3d[0] = size[1];
    }
    
    dx = gui->step_x[3] - gui->step_x[0];
    dy = gui->step_y[3] - gui->step_y[0];
    dz = gui->step_z[3] - gui->step_z[0];
    
    px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
    py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
    size[1] = sqrt(px*px + py*py);
    
    
    if(!(gui->user_flag & 2)){ /* torus height */
      gui->param_3d[2] = fabs(size[1] - gui->param_3d[0]);
    }
    
    
    
    /* top scale factor */
    if (gui->step < 4){
      //gui->param_3d[s] = gui->param_3d[0];
      gui->param_3d[s] = 0.0;
    }
    else if(!(gui->user_flag & 4)){
      
      gui->step_x[3] = gui->step_x[0] + gui->param_3d[2] * matrix[2][0];
      gui->step_y[3] = gui->step_y[0] + gui->param_3d[2] * matrix[2][1];
      gui->step_z[3] = gui->step_z[0] + gui->param_3d[2] * matrix[2][2];
      
      /* get next point in plane paralel to base */
      dx = gui->step_x[4] - gui->step_x[3];
      dy = gui->step_y[4] - gui->step_y[3];
      dz = gui->step_z[4] - gui->step_z[3];
      
      px = matrix[0][0]*dx + matrix[0][1]*dy + matrix[0][2]*dz;
      py = matrix[1][0]*dx + matrix[1][1]*dy + matrix[1][2]*dz;
      gui->param_3d[s] = sqrt(px*px + py*py);
    }
    
    /* create manifold */
    snprintf(cmd, 1000, "manifold[1] = torus(%.9g,%.9g,%.9g)\n"
      "manifold[1]:transform({{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g},"
      "{%.9g,%.9g,%.9g},{%.9g,%.9g,%.9g}})", 
      gui->param_3d[0], gui->param_3d[2], gui->param_3d[3],
      matrix[0][0], matrix[0][1], matrix[0][2],
      matrix[1][0], matrix[1][1], matrix[1][2],
      matrix[2][0], matrix[2][1], matrix[2][2],
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
      if (gui->step < 4 && size[x] > 1e-9 ) {
        
          gui->step++;
          
        
        gui->step_x[gui->step] = gui->step_x[gui->step - 1];
        gui->step_y[gui->step] = gui->step_y[gui->step - 1];
        gui->step_z[gui->step] = gui->step_z[gui->step - 1];
      }
      else{
        /* add torus to drawing */
        new_el = (dxf_node *) dxf_new_mesh ( cmd,
          gui->color_idx, /* color, layer */
          (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
          DWG_LIFE);
        
        
        new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
        drawing_ent_append(gui->drawing, new_el);
        
        do_add_entry(&gui->list_do, _l("TORUS"));
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

int gui_torus_info (gui_obj *gui){
	if (gui->modal != TORUS) return 0;
  char tmp_str[64];
	static char user_str_r[64] = "0.000000";
  static int prev_step = 0;
  
  static char mode[2][DXF_MAX_CHARS + 1];
  strncpy(mode[0], _l("by base plane"), DXF_MAX_CHARS);
  strncpy(mode[1], _l("by axis direction"), DXF_MAX_CHARS);
  
  const char *text_define[4];
  text_define[0] = _l("Define base radius:");
  text_define[1] = _l("Define base plane:");
  text_define[2] = _l("Define height:");
  text_define[3] = _l("Define top radius:");
  
  const char *text_info[4];
  text_info[0] = _l("Base radius: %.9g");
  text_info[1] = _l("Base plane: %.9g");
  text_info[2] = _l("Height: %.9g");
  text_info[3] = _l("Top radius: %.9g");
  
  char *mode_addr[] = {mode[0], mode[1]};
  
  int x = 0, y = 1, z = 2, s = 3;
  
  if (gui->extr_mode == E3D_BASE){
    x = 0; y = 1; z = 2; s = 3;
  } else {
    x = 2; y = 0; z = 1; s = 3;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a torus"), NK_TEXT_LEFT);
  
  int h = 2 * 25 + 5;
	gui->extr_mode = nk_combo(gui->ctx, (const char **) mode_addr, 2, gui->extr_mode, 20, nk_vec2(150, h));
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter base center"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		nk_label(gui->ctx, text_define[x], NK_TEXT_LEFT);
    
    /* edit to visualize or enter radius */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[x] = atof(user_str_r);
				gui->user_flag |= 1;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~1;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 1)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[x]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else if (gui->step == 2){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[y], NK_TEXT_LEFT);
    if (gui->extr_mode == E3D_TOP){
      /* edit to visualize or enter heigth */
      nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
        NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
        user_str_r, 63, nk_filter_float);
      if (res & NK_EDIT_ACTIVE){ /* enter mode */
        if (strlen(user_str_r)){
          /* sinalize the radius of user entry */
          gui->param_3d[y] = atof(user_str_r);
          gui->user_flag |= 2;
        }
        else{ /* if the user clear the string */
          /* cancel the enter mode*/
          gui->user_flag &= ~2;
          nk_edit_unfocus(gui->ctx);
        }
      } else if (!(gui->user_flag & 2)) { /* visualize mode */
        snprintf(user_str_r, 63, "%.9g", gui->param_3d[y]);
      }
      if (res & NK_EDIT_COMMITED){
        nk_edit_unfocus(gui->ctx);
      }
    }
  } else if (gui->step == 3){
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    nk_label(gui->ctx, text_define[z], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[z] = atof(user_str_r);
				gui->user_flag |= 2;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~2;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 2)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[z]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
  } else {
    if (prev_step != gui->step) snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
    snprintf(tmp_str, 63, text_info[x], gui->param_3d[x]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    if (gui->extr_mode == E3D_TOP)
      snprintf(tmp_str, 63, text_info[y],  gui->param_3d[y]);
    else
      snprintf(tmp_str, 63, text_info[z],  gui->param_3d[z]);
    nk_label(gui->ctx, tmp_str, NK_TEXT_LEFT);
    
    nk_label(gui->ctx, text_define[s], NK_TEXT_LEFT);
    
    /* edit to visualize or enter heigth */
		nk_flags res = nk_edit_string_zero_terminated(gui->ctx,
      NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT,
      user_str_r, 63, nk_filter_float);
		if (res & NK_EDIT_ACTIVE){ /* enter mode */
			if (strlen(user_str_r)){
				/* sinalize the radius of user entry */
				gui->param_3d[s] = atof(user_str_r);
				gui->user_flag |= 4;
			}
			else{ /* if the user clear the string */
				/* cancel the enter mode*/
				gui->user_flag &= ~4;
				nk_edit_unfocus(gui->ctx);
			}
		} else if (!(gui->user_flag & 4)) { /* visualize mode */
      snprintf(user_str_r, 63, "%.9g", gui->param_3d[s]);
		}
    if (res & NK_EDIT_COMMITED){
      nk_edit_unfocus(gui->ctx);
    }
	}
  
  prev_step = gui->step;
  
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