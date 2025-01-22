#include "gui_use.h"

int gui_view_info (gui_obj *gui){
	if (gui->modal != VIEW_ROTATE) return 0;
  
  static float alpha = 0, beta = 0, gamma = 0; 
  
  if (alpha != gui->alpha || beta != gui->beta || gamma != gui->gamma) {
    alpha = gui->alpha; beta = gui->beta; gamma = gui->gamma;
    gui->draw = 1;
  }
  
	nk_layout_row_dynamic(gui->ctx, 20, 1);
  nk_label(gui->ctx, _l("Rotate View"), NK_TEXT_LEFT);
  nk_property_float(gui->ctx, _l("#Alpha"), -180.0, &gui->alpha, 180.0f, 1.0f, 1.0);
  nk_property_float(gui->ctx, _l("#Beta"), -180.0, &gui->beta, 180.0f, 1.0f, 1.0);
  nk_property_float(gui->ctx, _l("#Gamma"), -180.0, &gui->gamma, 180.0f, 1.0f, 1.0);
  
  
  nk_layout_row_dynamic(gui->ctx, 20, 2);
  if (nk_button_label(gui->ctx, _l("Top"))){
    gui->alpha = 0.0;
    gui->beta = 0.0;
    gui->gamma = 0.0;
  }
  if (nk_button_label(gui->ctx, _l("Front"))){
    gui->alpha = 0.0;
    gui->beta = 0.0;
    gui->gamma = 90.0;
  }
  if (nk_button_label(gui->ctx, _l("Right"))){
    gui->alpha = 90.0;
    gui->beta = 0.0;
    gui->gamma = 90.0;
  }
  nk_layout_row_dynamic(gui->ctx, 20, 2);
  if (nk_button_label(gui->ctx, _l("Bottom"))){
    gui->alpha = 0.0;
    gui->beta = 0.0;
    gui->gamma = 180.0;
  }
  if (nk_button_label(gui->ctx, _l("Rear"))){
    gui->alpha = 180.0;
    gui->beta = 0.0;
    gui->gamma = 90.0;
  }
  if (nk_button_label(gui->ctx, _l("Left"))){
    gui->alpha = -90.0;
    gui->beta = 0.0;
    gui->gamma = 90.0;
  }
  nk_layout_row_dynamic(gui->ctx, 20, 1);
  if (nk_button_label(gui->ctx, _l("Iso"))){
    gui->alpha = 45.0;
    gui->beta = 0.0;
    gui->gamma = 90.0 - 35.264;
  }
  
  if (alpha != gui->alpha || beta != gui->beta || gamma != gui->gamma) {
    alpha = gui->alpha; beta = gui->beta; gamma = gui->gamma;
    gui->draw = 1;
    gui_calc_view_rot (gui);
    gui->action = REDRAW;
  }
  
  if (gui->ev & EV_CANCEL){
			
    gui->draw = 1;
    gui->draw_phanton = 0;
    
    gui_default_modal(gui);
  }
  

	return 1;
}