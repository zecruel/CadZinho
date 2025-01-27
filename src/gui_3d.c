#include "gui_use.h"

int gui_sphere_interactive(gui_obj *gui){
	
	if (gui->modal != SPHERE) return 0;
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

int gui_sphere_info (gui_obj *gui){
	if (gui->modal != SPHERE) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a sphere"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
	return 1;
}

int gui_cylinder_interactive(gui_obj *gui){
	
	if (gui->modal != CYLINDER) return 0;
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

int gui_cylinder_info (gui_obj *gui){
	if (gui->modal != CYLINDER) return 0;
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Place a cylinder"), NK_TEXT_LEFT);
	
	if (gui->step == 0){
		nk_label(gui->ctx, _l("Enter center point"), NK_TEXT_LEFT);
	} else if (gui->step == 1){
		nk_label(gui->ctx, _l("Define radius"), NK_TEXT_LEFT);
	}
	
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