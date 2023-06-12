
#include "gui_lay.h"
#include "gui_ltype.h"
#include "gui_tstyle.h"
#include "gui_info.h"
#include "gui_xy.h"
#include "gui_use.h"
#include "gui_file.h"
#include "gui_print.h"
#include "gui_export.h"
#include "gui_config.h"
#include "gui_script.h"

static void zoom_ext(dxf_drawing *drawing, int x, int y, int width, int height, double *zoom, double *ofs_x, double *ofs_y){
	double min_x = 0.0, min_y = 0.0, min_z, max_x = 20.0, max_y = 20.0, max_z;
	double zoom_x = 1.0, zoom_y = 1.0;
	
	dxf_ents_ext(drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
	zoom_x = fabs(max_x - min_x)/width;
	zoom_y = fabs(max_y - min_y)/height;
	
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	*zoom = 1/(1.1 * (*zoom));
	
	*ofs_x = min_x - ((fabs((max_x - min_x)*(*zoom) - width)/2)+x)/(*zoom);
	*ofs_y = min_y - ((fabs((max_y - min_y)*(*zoom) - height)/2)+y)/(*zoom);
}

int gui_main_loop (gui_obj *gui) {
  int running = 1;
  int ev_type;
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int leftMouseButtonClick = 0;
	int rightMouseButtonClick = 0;
	int MouseMotion = 0;
	int ctrlDown = 0;
	
	SDL_Event event;
	gui->low_proc = 1;
  int i, ok, key_space = 0, key_esc = 0;
  
  static char macro[64] = "";
	static int macro_len = 0;
	static int macro_timer = 0;
	
	static int refresh_timer = 0;
	
	static char function_key[20] = "";
  
  static int update_title = 0, changed = 0;
  
  static int open_prg = 0;
	static int progr_win = 0;
	static int progr_end = 0;
	//static unsigned int wait_open = 0;
  
  struct draw_param d_param;
	char file_path[DXF_MAX_CHARS];
	int file_path_len = 0;
  
  /* Colors in use */
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	bmp_color green = {.r = 0, .g = 255, .b =0, .a = 255};
	bmp_color yellow = {.r = 255, .g = 255, .b =0, .a = 255};
	bmp_color grey = {.r = 100, .g = 100, .b = 100, .a = 255};
  
  /* default substitution list (display white -> print black) */
  bmp_color list[] = { white, };
  bmp_color subst[] = { black, };
  
  //SDL_ShowCursor(SDL_DISABLE);
  
  
  /* get events for Nuklear GUI input */
  nk_input_begin(gui->ctx);
  if(SDL_PollEvent(&event)){
    if (event.type == SDL_QUIT) {
      //running = 0;
      if (gui->changed){
        gui->discard_changes = 1;
        gui->desired_action = EXIT;
        gui->hist_action = HIST_NONE;
      }
      else{
        running = 0;
      }
    }
    nk_sdl_handle_event(gui, gui->window, &event);
    ev_type = event.type;
  }
  nk_input_end(gui->ctx);
  
  /* ===============================*/
  if (nk_window_is_any_hovered(gui->ctx)) {
    //SDL_ShowCursor(SDL_ENABLE);
    SDL_SetCursor(gui->dflt_cur);
  }
  else{
    if (gui->pan_mode) SDL_SetCursor(gui->modal_cursor[PAN]);
    else SDL_SetCursor(gui->modal_cursor[gui->modal]);
    //SDL_ShowCursor(SDL_DISABLE);
    
    if (ev_type != 0){
      double wheel = 1.0;
      switch (event.type){
        case SDL_MOUSEBUTTONUP:
          gui->mouse_x = event.button.x;
          gui->mouse_y = event.button.y;
          gui->mouse_y = gui->win_h - gui->mouse_y;
          {
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
            gui->mouse_x = point[0];
            gui->mouse_y = point[1];
            //gui->mouse_z = point[2];
          }
          else{
            /* try xz plane*/
            plane[0] = 0.0; plane[1] = 1.0; plane[2] = 0.0; plane[3] = 0.0;
            if( ray_plane(ray_o, ray_dir, plane, point)){
              gui->mouse_x = point[0];
              gui->mouse_y = point[1];
              //gui->mouse_z = point[2];
            }
            else{
              /* try yz plane*/
              plane[0] = 1.0; plane[1] = 0.0; plane[2] = 0.0; plane[3] = 0.0;
              if( ray_plane(ray_o, ray_dir, plane, point)){
                gui->mouse_x = point[0];
                gui->mouse_y = point[1];
                //gui->mouse_z = point[2];
              }
              else{
                gui->mouse_x = ray_o[0];
                gui->mouse_y = ray_o[1];
                //gui->mouse_z = ray_o[2];
              }
            }
          }
        
          }
        
          if (event.button.button == SDL_BUTTON_LEFT){
            leftMouseButtonDown = 0;
          }
          else if(event.button.button == SDL_BUTTON_RIGHT){
            rightMouseButtonDown = 0;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          gui->mouse_x = event.button.x;
          gui->mouse_y = event.button.y;
          gui->mouse_y = gui->win_h - gui->mouse_y;
          
          {
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
            gui->mouse_x = point[0];
            gui->mouse_y = point[1];
            //gui->mouse_z = point[2];
          }
          else{
            /* try xz plane*/
            plane[0] = 0.0; plane[1] = 1.0; plane[2] = 0.0; plane[3] = 0.0;
            if( ray_plane(ray_o, ray_dir, plane, point)){
              gui->mouse_x = point[0];
              gui->mouse_y = point[1];
              //gui->mouse_z = point[2];
            }
            else{
              /* try yz plane*/
              plane[0] = 1.0; plane[1] = 0.0; plane[2] = 0.0; plane[3] = 0.0;
              if( ray_plane(ray_o, ray_dir, plane, point)){
                gui->mouse_x = point[0];
                gui->mouse_y = point[1];
                //gui->mouse_z = point[2];
              }
              else{
                gui->mouse_x = ray_o[0];
                gui->mouse_y = ray_o[1];
                //gui->mouse_z = ray_o[2];
              }
            }
          }
        
          }
          
          if (event.button.button == SDL_BUTTON_LEFT && !gui->pan_mode){
            leftMouseButtonDown = 1;
            leftMouseButtonClick = 1;
          }
          else if(event.button.button == SDL_BUTTON_RIGHT && !gui->pan_mode){
            rightMouseButtonDown = 1;
            rightMouseButtonClick = 1;
          }
          /* activate or toggle the pan mode */
          if (event.button.button == SDL_BUTTON_MIDDLE){
            gui->pan_mode = !gui->pan_mode;
          }
          else gui->pan_mode = 0;
          gui->draw = 1;
          break;
        case SDL_MOUSEMOTION:
          MouseMotion = 1;
          gui->mouse_x = event.motion.x;
          gui->mouse_y = event.motion.y;
          gui->mouse_y = gui->win_h - gui->mouse_y;
          {
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
            gui->mouse_x = point[0];
            gui->mouse_y = point[1];
            //gui->mouse_z = point[2];
          }
          else{
            /* try xz plane*/
            plane[0] = 0.0; plane[1] = 1.0; plane[2] = 0.0; plane[3] = 0.0;
            if( ray_plane(ray_o, ray_dir, plane, point)){
              gui->mouse_x = point[0];
              gui->mouse_y = point[1];
              //gui->mouse_z = point[2];
            }
            else{
              /* try yz plane*/
              plane[0] = 1.0; plane[1] = 0.0; plane[2] = 0.0; plane[3] = 0.0;
              if( ray_plane(ray_o, ray_dir, plane, point)){
                gui->mouse_x = point[0];
                gui->mouse_y = point[1];
                //gui->mouse_z = point[2];
              }
              else{
                gui->mouse_x = ray_o[0];
                gui->mouse_y = ray_o[1];
                //gui->mouse_z = ray_o[2];
              }
            }
          }
          
          /* pan drawing with middle button */
          
          if (gui->pan_mode){//(event.motion.state & SDL_BUTTON_MMASK){
            gui->ofs_x -= (double) (gui->mouse_x - gui->prev_mouse_x)/gui->zoom;
            gui->ofs_y -= (double) (gui->mouse_y - gui->prev_mouse_y)/gui->zoom;
          }
          gui->prev_mouse_x = gui->mouse_x;
          gui->prev_mouse_y = gui->mouse_y;
          }
          gui->draw = 1;
          break;
        case SDL_MOUSEWHEEL:
          
          if(event.wheel.y < 0) wheel = -1.0; // scroll down
          gui->prev_zoom = gui->zoom;
          gui->zoom = gui->zoom + wheel * 0.3 * gui->zoom;
          
          SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);
          gui->mouse_y = gui->win_h - gui->mouse_y;
          gui->ofs_x += ((double) gui->mouse_x)*(1/gui->prev_zoom - 1/gui->zoom);
          gui->ofs_y += ((double) gui->mouse_y)*(1/gui->prev_zoom - 1/gui->zoom);
          gui->draw = 1;
          break;
        #if(0)
        case (SDL_DROPFILE): {      /* In case if dropped file */
          /* get file path -> drop event has previously proccessed and string was moved to clipboard !!!*/
          char *dropped_filedir = SDL_GetClipboardText(); 
          if (!gui->show_open && !gui->show_save){
            strncpy (gui->curr_path, dropped_filedir, DXF_MAX_CHARS);
            gui->path_ok = 1;
            /* open file */
            if (gui->changed){
              gui->discard_changes = 1;
              gui->desired_action = FILE_OPEN;
              gui->hist_action = HIST_ADD;
            }
            else{
              gui->action = FILE_OPEN;
              gui->hist_new = 1; /* add to history entries */
            }
            
          }
          SDL_free(dropped_filedir);    // Free dropped_filedir memory
          }
          break;
        #endif
        case SDL_KEYDOWN: {
          SDL_Keycode key = event.key.keysym.sym;
          SDL_Keymod mod = event.key.keysym.mod;
          for (i = 0; i < func_keys_size; i++){
            if (func_keys[i].code == key && (func_keys[i].mod & mod || func_keys[i].mod == KMOD_NONE)){
              strncpy (function_key, func_keys[i].key, 19);
            }
          }
          
          if (key == SDLK_RCTRL || key == SDLK_LCTRL) ctrlDown = 1;
        
          if ((key == SDLK_RETURN) || (key == SDLK_RETURN2)){
            gui->keyEnter = 1;
          }
          else if (key == SDLK_SPACE){
            key_space = 1;
          }
          else if (key == SDLK_ESCAPE){
            key_esc = 1;
          }
          else if (key == SDLK_UP && (mod & KMOD_CTRL)){
            gui->action = VIEW_PAN_U;
          }
          else if (key == SDLK_DOWN && (mod & KMOD_CTRL)){
            gui->action = VIEW_PAN_D;
          }
          else if (key == SDLK_LEFT && (mod & KMOD_CTRL)){
            gui->action = VIEW_PAN_L;
          }
          else if (key == SDLK_RIGHT && (mod & KMOD_CTRL)){
            gui->action = VIEW_PAN_R;
          }
          else if ((key == SDLK_KP_MINUS || key == SDLK_MINUS) && 
            (mod & KMOD_CTRL)){
            gui->action = VIEW_ZOOM_M;
          }
          else if ((key == SDLK_KP_PLUS || key == SDLK_PLUS || key == SDLK_EQUALS) && 
            (mod & KMOD_CTRL)){
            gui->action = VIEW_ZOOM_P;
          }
          else if (key == SDLK_DELETE){
            //gui->ev |= EV_DEL;
            gui->action = DELETE;
          }
          else if (key == SDLK_c && (mod & KMOD_CTRL)){
            //gui->ev |= EV_YANK;
            gui->action = YANK;
          }
          else if (key == SDLK_x && (mod & KMOD_CTRL)){
            //gui->ev |= EV_CUT;
            gui->action = CUT;
          }
          else if (key == SDLK_v && (mod & KMOD_CTRL)){
            //gui->ev |= EV_PASTE;
            gui->action = START_PASTE;
          }
          else if (key == SDLK_z && (mod & KMOD_CTRL)){
            //gui->ev |= EV_UNDO;
            gui->action = UNDO;
          }
          else if (key == SDLK_r && (mod & KMOD_CTRL)){
            //gui->ev |= EV_REDO;
            gui->action = REDO;
          }}
          break;
          
        case SDL_KEYUP: {
          SDL_Keycode key = event.key.keysym.sym;
          SDL_Keymod mod = event.key.keysym.mod;
          
          if (key == SDLK_RCTRL || key == SDLK_LCTRL) ctrlDown = 0;
          }break;
        case SDL_TEXTINPUT:
          /* text input */
        
          /* if the user enters a character relative a number */
          if ((*event.text.text > 41) && (*event.text.text < 58) && (gui->en_distance||!gui->entry_relative)){
            gui->user_number = 1; /* sinalize a user flag */
          }
          else{
            macro[macro_len] = *event.text.text;
            if (macro_len < 63) macro_len++;
            macro[macro_len] = 0;
            macro_timer = 0;
            gui->draw = 1;
          }
          break;
        case SDL_WINDOWEVENT:
          if (event.window.event == SDL_WINDOWEVENT_RESIZED){
            gui->draw = 1;
          }
          if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            gui->draw = 1;
          }
      }
    }
    
  }
  
  if (MouseMotion) gui->ev |= EV_MOTION;
  if (leftMouseButtonClick) gui->ev |= EV_ENTER;
  if (rightMouseButtonClick || key_esc) gui->ev |= EV_CANCEL;
  if (key_space) gui->ev |= EV_LOCK_AX;
  if (ctrlDown) gui->ev |= EV_ADD;
  
  /* verify if drawing was changed */
  gui->changed = 0;
  if (gui->save_pt != gui->list_do.current){
    gui->changed = 1;
  }
  
  /* == Window title == */
  if (update_title || changed != gui->changed){
    /* update window title, for changes in drawing file name or during drawing editing */
    char title[DXF_MAX_CHARS] = "CadZinho - ";
    strncat (title, gui->dwg_file, DXF_MAX_CHARS - strlen(title));
    if (gui->changed){ /* if current drawing was changed, put "(*)" to indicate modifications */
      strncat (title, " (*)", DXF_MAX_CHARS - strlen(title));
    }
    
    SDL_SetWindowTitle(gui->window, title);
    update_title = 0;
  }
  
  changed = gui->changed;
  
  gui->next_win_h = 83;
  gui->next_win_x = 2;
  gui->next_win_y = 2;
  
  
  gui->next_win_y += gui->next_win_h + 3;
  gui->next_win_w = 210;
  gui->next_win_h = 500;
  
  gui_tools_win (gui);
  gui_main_win (gui);
  
  
  if (gui->show_open){
    gui->show_open = gui_file_open (gui, NULL);  // *** test
    if (gui->show_open == 2){
      gui->path_ok = 1;
      
      gui->show_open = 0;
    }
  }
  
  if (gui->show_save){
    gui->show_save = gui_file_save (gui, NULL);  // *** test
    if (gui->show_save == 2){
      gui->path_ok = 1;
      
      gui->show_save = 0;
    }
  }
  
  
/* ==============================================================
======    LAYER MANAGER   ==========================================
================================================================*/
  
  if (gui->show_lay_mng){
    gui->show_lay_mng = lay_mng (gui);
  }
  
/* ==============================================================
======    END LAYER MANAGER   ======================================
================================================================*/
  if (gui->show_ltyp_mng){
    gui->show_ltyp_mng = ltyp_mng (gui);
  }
  
  if (gui->show_info){
    gui->show_info = info_win(gui);
  }
  if (gui->show_script){
    gui->show_script = script_win(gui);
  }
  if (gui->show_print){
    gui->show_print = print_win(gui);
  }
  if (gui->show_export){
    gui->show_export = export_win(gui);
  }
  if (gui->show_blk_mng){
    gui->show_blk_mng = gui_blk_mng (gui);
  }
  
  if (gui->show_tstyles_mng){
    gui->show_tstyles_mng = tstyles_mng (gui);
  }
  
  if (gui->show_config){ /* configuration window */
    gui->show_config = config_win (gui);
  }
  
  if (gui->show_dim_mng){
    gui->show_dim_mng = gui_dim_mng (gui);
  }
  
  if (gui->show_hatch_mng){
    gui->show_hatch_mng = gui_hatch_mng (gui);
  }
  
  if (gui->show_plugins){ /* Additional tools window */
    gui->show_plugins = gui_plugins_win (gui);
  }
  
  if (progr_win){
    /* opening */
    if (nk_begin(gui->ctx, "Progress", nk_rect(200, 200, 400, 40),
    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR))
    //if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "opening", 0, nk_rect(200, 200, 400, 40)))
    {
      static char text[64];
      static int text_len;
      nk_layout_row_dynamic(gui->ctx, 20, 2);
      text_len = snprintf(text, 63, "Opening...");
      nk_label(gui->ctx, text, NK_TEXT_LEFT);
      nk_prog(gui->ctx, (nk_size)gui->progress, 100, NK_FIXED);
      //nk_popup_end(gui->ctx);
      nk_end(gui->ctx);
    }
    
    if (progr_end){
      progr_win = 0;
      progr_end = 0;
      nk_window_close(gui->ctx, "Progress");
    }
  }
  
  gui_bottom_win (gui);
  
  /* */
  
  
  if (!(nk_window_is_any_hovered(gui->ctx)) && (gui->modal != SELECT)){
    nk_window_set_focus(gui->ctx, "POS");
    
    
  }
  
  
  /*================================================
  ==================================================
  =================================================*/
  
  
  
  if (wait_open != 0){
    gui->low_proc = 0;
    gui->draw = 1;
    
    open_prg = dxf_read(gui->drawing, gui->file_buf->buffer, gui->file_size, &gui->progress);
    
    if(open_prg <= 0){
      //free(gui->file_buf);
      manage_buffer(0, BUF_RELEASE, 0);
      gui->file_buf = NULL;
      gui->file_size = 0;
      gui->low_proc = 1;
      
      dxf_ents_parse(gui->drawing);				
      gui->action = VIEW_ZOOM_EXT;
      gui->layer_idx = dxf_lay_idx (gui->drawing,
        strpool_inject( &name_pool, "0", 1));
      gui->ltypes_idx = dxf_ltype_idx (gui->drawing,
        strpool_inject( &name_pool, "BYLAYER", 7));
      gui->t_sty_idx = dxf_tstyle_idx (gui->drawing,
        strpool_inject( &name_pool, "STANDARD", 8), 0);
      gui->color_idx = 256;
      gui->lw_idx = DXF_LW_LEN;
      wait_open = 0;
      progr_end = 1;
      sel_list_clear (gui);
      
      if (gui->script_resume_reason == YIELD_DRWG_OPEN){
        gui->script_resume_reason = YIELD_NONE;
        gui->script_resume = 1;
        if(open_prg < 0) gui->script_resume = -1;
      }
    }
    
  }
  else gui->low_proc = 1;
  
  
  /*===============================*/
  if(gui->action == EXIT) {
    running = 0;
  }
  else if(gui->action == FILE_NEW && (wait_open == 0)) {
    gui->action = NONE;
    do_mem_pool(ZERO_DO_ITEM);
    do_mem_pool(ZERO_DO_ENTRY);
    init_do_list(&gui->list_do);
    gui->save_pt = gui->list_do.current;
    
    /* load and apply the fonts required for drawing */
    gui->drawing->font_list = gui->font_list;
    gui->drawing->dflt_font = get_font_list(gui->font_list, "txt.shx");
    gui->drawing->dflt_fonts_path = gui->dflt_fonts_path;
    
    while (dxf_read (gui->drawing, gui->seed, strlen(gui->seed), &gui->progress) > 0){
      
    }
    
    gui->layer_idx = dxf_lay_idx (gui->drawing,
      strpool_inject( &name_pool, "0", 1));
    gui->ltypes_idx = dxf_ltype_idx (gui->drawing,
      strpool_inject( &name_pool, "BYLAYER", 7));
    gui->t_sty_idx = dxf_tstyle_idx (gui->drawing,
      strpool_inject( &name_pool, "STANDARD", 8), 0);
    gui->color_idx = 256;
    gui->lw_idx = DXF_LW_LEN;
    gui->curr_path[0] = 0;
    gui->drwg_hist_pos ++;
    sel_list_clear (gui);

    //strncpy (gui->dwg_dir, get_dir(gui->curr_path) , PATH_MAX_CHARS);
    //strncpy (gui->dwg_file, get_filename(gui->curr_path) , PATH_MAX_CHARS);
    gui->dwg_file[0] = 0;
    
    update_title = 1;
    
    if (gui->script_resume_reason == YIELD_DRWG_NEW){
      gui->script_resume_reason = YIELD_NONE;
      gui->script_resume = 1;
    }
  }
  else if((gui->action == FILE_OPEN) && (gui->path_ok) && (wait_open == 0)) {
    gui->action = NONE; gui->path_ok = 0;
    gui->file_buf = load_file_reuse(gui->curr_path, &gui->file_size);
    
    if (gui->file_buf){
      /* change dir to main drawing folder */
      dir_change(get_dir(gui->curr_path));
      
      dxf_mem_pool(ZERO_DXF, DWG_LIFE);
      graph_mem_pool(ZERO_GRAPH, DWG_LIFE);
      graph_mem_pool(ZERO_LINE, DWG_LIFE);
      
      
      {
        /* reinit string pools */
        strpool_term( &value_pool );
        strpool_term( &name_pool );
        
        strpool_config_t str_pool_conf = strpool_default_config;
        str_pool_conf.counter_bits = 32;
        str_pool_conf.index_bits = 32;
        str_pool_conf.ignore_case = 1;
        strpool_init( &name_pool, &str_pool_conf);
        
        str_pool_conf.ignore_case = 0;
        str_pool_conf.min_length = 256;
        strpool_init( &value_pool, &str_pool_conf);
      }
      
      wait_open = 1;
      gui->progress = 0;
    
      /* load and apply the fonts required for drawing */
      gui->drawing->font_list = gui->font_list;
      gui->drawing->dflt_font = get_font_list(gui->font_list, "txt.shx");
      gui->drawing->dflt_fonts_path = gui->dflt_fonts_path;
      
      open_prg = dxf_read(gui->drawing, gui->file_buf->buffer, gui->file_size, &gui->progress);
      
      gui->low_proc = 0;
      progr_win = 1;
      
      SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
      
      /* add file to history */
      if (gui->hist_new && open_prg >= 0){
        gui_hist_add (gui);
        
      }
      if (open_prg >= 0){
        do_mem_pool(ZERO_DO_ITEM);
        do_mem_pool(ZERO_DO_ENTRY);
        init_do_list(&gui->list_do);
        gui->save_pt = gui->list_do.current;

        strncpy (gui->dwg_dir, get_dir(gui->curr_path) , PATH_MAX_CHARS);
        strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
        
        update_title = 1;
      }
    }
    else {
      snprintf(gui->log_msg, 63, "Error in opening drawing: Not file");
      if (gui->script_resume_reason == YIELD_DRWG_OPEN){
        gui->script_resume_reason = YIELD_NONE;
        gui->script_resume = -1;
      }
    }
  }
  else if((gui->action == FILE_SAVE) && (gui->path_ok) && (wait_open == 0)){
    gui->action = NONE; gui->path_ok = 0;
  
  
    if (dxf_save (gui->curr_path, gui->drawing)){
      if (strncmp (gui->dwg_dir, get_dir(gui->curr_path) , PATH_MAX_CHARS) == 0 &&
        strncmp (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS) == 0 )
      { gui->hist_new = 0; }
      
      strncpy (gui->dwg_dir, get_dir(gui->curr_path) , PATH_MAX_CHARS);
      strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
      
      gui->save_pt = gui->list_do.current;
      update_title = 1;
      if (gui->hist_new){
        gui_hist_add (gui);
      }
      if (gui->script_resume_reason == YIELD_DRWG_SAVE){
        gui->script_resume_reason = YIELD_NONE;
        gui->script_resume = 1;
      }
    }
    else {
      snprintf(gui->log_msg, 63, "Error in saving drawing");
      if (gui->script_resume_reason == YIELD_DRWG_SAVE){
        gui->script_resume_reason = YIELD_NONE;
        gui->script_resume = -1;
      }
    }
    gui->hist_new = 0;
  
  }
  else if((gui->action == EXPORT) && (gui->path_ok)) {
    gui->action = NONE; gui->path_ok = 0;
    
    if (gui->drawing->main_struct != NULL){
      dxf_ent_print_f (gui->drawing->main_struct, gui->curr_path);
      //dxf_save (url, gui->drawing);
    }
  }
  else if(gui->action == VIEW_ZOOM_EXT){
    gui->action = NONE;
    zoom_ext(gui->drawing, 0, 0, gui->win_w, gui->win_h, &gui->zoom, &gui->ofs_x, &gui->ofs_y);
    gui->draw = 1;
  }
  else if(gui->action == VIEW_ZOOM_P){
    gui->action = NONE;
    
    gui->prev_zoom = gui->zoom;
    gui->zoom = gui->zoom + 0.2 * gui->zoom;
    gui->ofs_x += (gui->win_w/2)*(1/gui->prev_zoom - 1/gui->zoom);
    gui->ofs_y += (gui->win_h/2)*(1/gui->prev_zoom - 1/gui->zoom);
    gui->draw = 1;
  }
  else if(gui->action == VIEW_ZOOM_M){
    gui->action = NONE;
    gui->prev_zoom = gui->zoom;
    gui->zoom = gui->zoom - 0.2 * gui->zoom;
    gui->ofs_x += (gui->win_w/2)*(1/gui->prev_zoom - 1/gui->zoom);
    gui->ofs_y += (gui->win_h/2)*(1/gui->prev_zoom - 1/gui->zoom);
    gui->draw = 1;
  }
  else if(gui->action == VIEW_PAN_U){
    gui->action = NONE;
    gui->ofs_y += (gui->win_h*0.1)/gui->zoom;
    gui->draw = 1;
  }
  else if(gui->action == VIEW_PAN_D){
    gui->action = NONE;
    gui->ofs_y -= (gui->win_h*0.1)/gui->zoom;
    gui->draw = 1;
  }
  else if(gui->action == VIEW_PAN_L){
    gui->action = NONE;
    gui->ofs_x -= (gui->win_w*0.1)/gui->zoom;
    gui->draw = 1;
  }
  else if(gui->action == VIEW_PAN_R){
    gui->action = NONE;
    gui->ofs_x += (gui->win_w*0.1)/gui->zoom;
    gui->draw = 1;
  }
  else if(gui->action == REDRAW){
    gui->action = NONE;
    
    dxf_ents_parse(gui->drawing);
    
    gui->draw = 1;
  }
  else if((gui->action == YANK || gui->action == CUT) && strlen(gui->clip_path) > 0) {
    if (gui->sel_list->next){ /* verify if  has elements in list */
      /* clear the clipboard drawing and init with basis seed */
      dxf_drawing_clear(gui->clip_drwg);
      
      /* load and apply the fonts required for clipboard drawing */
      gui->clip_drwg->font_list = gui->font_list;
      gui->clip_drwg->dflt_font = get_font_list(gui->font_list, "txt.shx");
      gui->clip_drwg->dflt_fonts_path = gui->dflt_fonts_path;
      
      while (dxf_read (gui->clip_drwg, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &gui->progress) > 0){
        
      }
      /* copy selected elements from main drawing to clipboard */
      dxf_drwg_ent_cpy(gui->drawing, gui->clip_drwg, gui->sel_list);
      
      /* validate the clipboard drawing with used layers, styles, line types and APPIDs */
      dxf_cpy_lay_drwg(gui->drawing, gui->clip_drwg);
      dxf_cpy_sty_drwg(gui->drawing, gui->clip_drwg);
      dxf_cpy_ltyp_drwg(gui->drawing, gui->clip_drwg);
      dxf_cpy_appid_drwg(gui->drawing, gui->clip_drwg);
      
      /* save clipboard to file */
      dxf_save (gui->clip_path, gui->clip_drwg);
      
      /* preapre to reuse memory of clipboard */
      dxf_mem_pool(ZERO_DXF, ONE_TIME);
    }
    if(gui->action == CUT){
      do_add_entry(&gui->list_do, "CUT");
      
      list_node *current = gui->sel_list->next;
      
      // starts the content sweep 
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            
            if (do_add_item(gui->list_do.current, (dxf_node *)current->data, NULL)) {
              
            }
            
            // -------------------------------------------
            //dxf_obj_detach((dxf_node *)current->data);
            dxf_obj_subst((dxf_node *)current->data, NULL);
            
            //---------------------------------------
          }
        }
        current = current->next;
      }
      sel_list_clear (gui);
      gui->element = NULL;
    }
    
    gui->action = NONE;
    gui->draw = 1;
  }
  else if (gui->action == START_PASTE && strlen(gui->clip_path) > 0) {
    /* clear memory pool used before */
    dxf_mem_pool(ZERO_DXF, ONE_TIME);
    graph_mem_pool(ZERO_GRAPH, ONE_TIME);
    graph_mem_pool(ZERO_LINE, ONE_TIME);
    dxf_drawing_clear(gui->clip_drwg);
    /* load the clipboard file */
    gui->file_size = 0;
    gui->file_buf = load_file_reuse(gui->clip_path, &gui->file_size);
    if (!gui->file_buf) {
      gui->action = NONE;
    }
    else{
      /* load and apply the fonts required for clipboard drawing */
      gui->clip_drwg->font_list = gui->font_list;
      gui->clip_drwg->dflt_font = get_font_list(gui->font_list, "txt.shx");
      gui->clip_drwg->dflt_fonts_path = gui->dflt_fonts_path;
      
      while (dxf_read (gui->clip_drwg, gui->file_buf->buffer, gui->file_size, &gui->progress) > 0){
        
      }
      
      /* clear the file buffer */
      manage_buffer(0, BUF_RELEASE, 0);
      gui->file_buf = NULL;
      gui->file_size = 0;
      
      /* prepare for next steps on paste */
      gui->modal = PASTE;
      gui->step = 0;
      gui->action = NONE;
      gui->draw = 1;
    }
  }
  else if(gui->action == DELETE){
    gui->action = NONE;
    
    list_node *deleted = dxf_delete_list(gui->drawing, gui->sel_list);
  
    if (deleted && deleted->next){ /* verify if  has elements in list */
      
      do_add_entry(&gui->list_do, "DELETE");
      
      list_node *current = deleted->next;
      
      // starts the content sweep 
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            
            if (do_add_item(gui->list_do.current, (dxf_node *)current->data, NULL)) {
              
            }
          }
        }
        current = current->next;
      }
      sel_list_clear (gui);
      gui->element = NULL;
    }
    gui->draw = 1;
  }
  else if(gui->action == UNDO){
    gui->action = NONE;
    sel_list_clear (gui);
    char *text = gui->list_do.current->text;
    
    if (do_undo(&gui->list_do)){
      snprintf(gui->log_msg, 63, "UNDO: %s", text);
      gui_first_step(gui);
    }
    else{
      snprintf(gui->log_msg, 63, "No actions to undo");
    }
    gui->draw = 1;
  }
  
  else if(gui->action == REDO){
    gui->action = NONE;
    sel_list_clear (gui);
    if (do_redo(&gui->list_do)){
      snprintf(gui->log_msg, 63, "REDO: %s", gui->list_do.current->text);
      gui_first_step(gui);
    }
    else{
      snprintf(gui->log_msg, 63, "No actions to redo");
    }
    gui->draw = 1;
  }
  
  else if(gui->action == LAYER_CHANGE){
    gui->action = NONE;
    if (gui->sel_list != NULL){
      /* sweep the selection list */
      list_node *current = gui->sel_list->next;
      dxf_node *new_ent = NULL;
      if (current != NULL){
        do_add_entry(&gui->list_do, "CHANGE LAYER");
      }
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
            
            dxf_attr_change(new_ent, 8,
              (void *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name));
            
            new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
            
            dxf_obj_subst((dxf_node *)current->data, new_ent);
            
            do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);

            current->data = new_ent;
          }
        }
        current = current->next;
      }
    }
    gui->draw = 1;
  }
  
  else if(gui->action == COLOR_CHANGE){
    gui->action = NONE;
    if (gui->sel_list != NULL){
      /* sweep the selection list */
      list_node *current = gui->sel_list->next;
      dxf_node *new_ent = NULL;
      if (current != NULL){
        do_add_entry(&gui->list_do, "CHANGE COLOR");
      }
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
            
            dxf_attr_change(new_ent, 62, &gui->color_idx);
            new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
            
            dxf_obj_subst((dxf_node *)current->data, new_ent);
            
            do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);

            current->data = new_ent;
          }
        }
        current = current->next;
      }
    }
    gui->draw = 1;
  }
  else if(gui->action == LTYPE_CHANGE){
    gui->action = NONE;
    if (gui->sel_list != NULL){
      /* sweep the selection list */
      list_node *current = gui->sel_list->next;
      dxf_node *new_ent = NULL;
      if (current != NULL){
        do_add_entry(&gui->list_do, "CHANGE LINE TYPE");
      }
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
            
            dxf_attr_change(new_ent, 6,
              (void *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name));
            new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
            
            dxf_obj_subst((dxf_node *)current->data, new_ent);
            
            do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);

            current->data = new_ent;
          }
        }
        current = current->next;
      }
    }
    gui->draw = 1;
  }
  else if(gui->action == LW_CHANGE){
    gui->action = NONE;
    if (gui->sel_list != NULL){
      /* sweep the selection list */
      list_node *current = gui->sel_list->next;
      dxf_node *new_ent = NULL;
      if (current != NULL){
        do_add_entry(&gui->list_do, "CHANGE LINE WEIGHT");
      }
      while (current != NULL){
        if (current->data){
          if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
            new_ent = dxf_ent_copy((dxf_node *)current->data, 0);
            
            dxf_attr_change(new_ent, 370, &dxf_lw[gui->lw_idx]);
            new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
            
            dxf_obj_subst((dxf_node *)current->data, new_ent);
            
            do_add_item(gui->list_do.current, (dxf_node *)current->data, new_ent);

            current->data = new_ent;
          }
        }
        current = current->next;
      }
    }
    gui->draw = 1;
  }
  
  /**********************************/
  
  gui_update_pos(gui);
  
  /**********************************/
  
  
  
  /*-------------------------------- macro script --------------------- */
  if ( gui->macro_script.active && strlen(macro) > 0 ){
    gui->macro_script.time = clock();
    gui->macro_script.timeout = 1.0; /* default timeout value */
    gui->macro_script.do_init = 0;
    
    lua_pushstring(gui->macro_script.T, macro);
    lua_setglobal(gui->macro_script.T, "macro");
    
    lua_pushboolean(gui->macro_script.T, 0);
    lua_setglobal(gui->macro_script.T, "accept");
    //print_lua_stack(gui->macro_script.T);
    
    //lua_getglobal(gui->macro_script.T, "cz_main_func");
    lua_pushvalue(gui->macro_script.T, 1);
    int n_results = 0; /* for Lua 5.4*/
    gui->macro_script.status = lua_resume(gui->macro_script.T, NULL, 0, &n_results); /* start thread */
    if (gui->macro_script.status != LUA_OK){
      gui->macro_script.active = 0;
      
    }
    else {
      
      lua_getglobal(gui->macro_script.T, "accept");
      if (lua_toboolean(gui->macro_script.T, -1)){
        macro_timer = 0;
        macro_len = 0;
        macro[0] = 0;
      }
      lua_pop(gui->macro_script.T, 1);
    }
  }
  
  /*-------------------------------- function key script --------------------- */
  if ( gui->func_keys_script.active && strlen(function_key) > 0 ){
    gui->func_keys_script.time = clock();
    gui->func_keys_script.timeout = 1.0; /* default timeout value */
    gui->func_keys_script.do_init = 0;
    
    lua_pushstring(gui->func_keys_script.T, function_key);
    lua_setglobal(gui->func_keys_script.T, "function_key");
    
    if (luaL_dofile (gui->func_keys_script.T,  gui->func_keys_path) != LUA_OK) {
      /* error*/
    }
  }
  
  gui_select_interactive(gui);
  gui_line_interactive(gui);
  gui_pline_interactive(gui);
  gui_circle_interactive(gui);
  gui_rect_interactive(gui);
  gui_text_interactive(gui);
  gui_mtext_interactive(gui);
  gui_move_interactive(gui);
  gui_dupli_interactive(gui);
  gui_scale_interactive(gui);
  gui_rotate_interactive(gui);
  gui_mirror_interactive(gui);
    
  gui_insert_interactive(gui);
  gui_block_interactive(gui);
  gui_hatch_interactive(gui);
  gui_paste_interactive(gui);
  gui_ed_text_interactive(gui);
  gui_script_interactive(gui);
  gui_spline_interactive(gui);
  //gui_arc_interactive(gui);
  gui_ellip_interactive(gui);
  gui_image_interactive(gui);
  
  gui_ed_attr_interactive(gui);
  gui_attrib_interactive(gui);
  gui_expl_interactive(gui);
  gui_measure_interactive(gui);
  gui_find_interactive(gui);
  gui_prop_interactive(gui);
  gui_txt_prop_interactive(gui);
  gui_vertex_interactive(gui);
  
  gui_dim_interactive(gui);
  
  if (gui->prev_modal != gui->modal){
    
    gui->en_distance = 0;
    gui->draw_tmp = 0;
    gui->element = NULL;
    gui->draw = 1;
    gui->step = 0;
    gui->draw_phanton = 0;
    if (gui->phanton){
      gui->phanton = NULL;
    }
    gui->lock_ax_x = 0;
    gui->lock_ax_y = 0;
    gui->user_flag_x = 0;
    gui->user_flag_y = 0;
    /*
    if (gui->prev_modal == SCRIPT){
      gui->lua_script[0].active = 0;
      gui->lua_script[0].dynamic = 0;
    }
    */
    gui->prev_modal = gui->modal;
  }
  
  /* window file browser */
  if (gui->show_file_br == 1){			
    gui->show_file_br = file_win(gui, gui->file_filter_types, gui->file_filter_descr, gui->file_filter_count, NULL);
    strncpy(file_path, gui->curr_path, DXF_MAX_CHARS);
    file_path_len = strlen(file_path);
  }
  
  if (gui_check_draw(gui) != 0){
    gui->draw = 1;
  }
  
  if (gui->draw != 0){
    /*get current window size and position*/
    SDL_GetWindowSize(gui->window, &gui->win_w, &gui->win_h);
    SDL_GetWindowPosition (gui->window, &gui->win_x, &gui->win_y);
    
    
    glUniform1i(gui->gl_ctx.tex_uni, 0);
    
    SDL_GetWindowSize(gui->window, &gui->gl_ctx.win_w, &gui->gl_ctx.win_h);
    glViewport(0, 0, gui->gl_ctx.win_w, gui->gl_ctx.win_h);
    
    d_param.ofs_x = gui->ofs_x;
    d_param.ofs_y = gui->ofs_y;
    d_param.ofs_z = 0;
    d_param.scale = gui->zoom;
    if (gui->background.r * 0.21 +  /* verify "brightness" of background color */
      gui->background.g * 0.72 + gui->background.b * 0.07 > 150) {
      /* in bright background, change defaut color (white) to black */
      d_param.list = list;
      d_param.subst = subst;
      d_param.len_subst = 1;
    } else {
      d_param.list = NULL;
      d_param.subst = NULL;
      d_param.len_subst = 0;
    }
    d_param.inc_thick = 0;
    
    
    /* 3D test */
    gui->gl_ctx.transf[0][0] = gui->drwg_view[0][0];
    gui->gl_ctx.transf[0][1] = gui->drwg_view[0][1];
    gui->gl_ctx.transf[0][2] = gui->drwg_view[0][2];
    gui->gl_ctx.transf[0][3] = gui->drwg_view[0][3];
    gui->gl_ctx.transf[1][0] = gui->drwg_view[1][0];
    gui->gl_ctx.transf[1][1] = gui->drwg_view[1][1];
    gui->gl_ctx.transf[1][2] = gui->drwg_view[1][2];
    gui->gl_ctx.transf[1][3] = gui->drwg_view[1][3];
    gui->gl_ctx.transf[2][0] = gui->drwg_view[2][0];
    gui->gl_ctx.transf[2][1] = gui->drwg_view[2][1];
    gui->gl_ctx.transf[2][2] = gui->drwg_view[2][2];
    gui->gl_ctx.transf[2][3] = gui->drwg_view[2][3];
    gui->gl_ctx.transf[3][0] = gui->drwg_view[3][0];
    gui->gl_ctx.transf[3][1] = gui->drwg_view[3][1];
    gui->gl_ctx.transf[3][2] = gui->drwg_view[3][2];
    gui->gl_ctx.transf[3][3] = gui->drwg_view[3][3];
    
    //glDepthFunc(GL_ALWAYS);
    glDepthFunc(GL_LEQUAL);
    
    
    /* Clear the screen to background color */
    glClearColor((GLfloat) gui->background.r/255, (GLfloat) gui->background.g/255, 
      (GLfloat) gui->background.b/255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    #ifndef GLES2
    if (gui->gl_ctx.elems == NULL){
      gui->gl_ctx.verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
      gui->gl_ctx.elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
    #endif
    gui->gl_ctx.vert_count = 0;
    gui->gl_ctx.elem_count = 0;
    glUniform1i(gui->gl_ctx.tex_uni, 0);
    
    
    dxf_ents_draw_gl(gui->drawing, &gui->gl_ctx, d_param);
    
    if (!gui->pan_mode) 
      draw_cursor_gl(gui, gui->mouse_x, gui->mouse_y, gui->cursor);
    
    draw_gl (&gui->gl_ctx, 1); /* force draw and cleanup */
    //glReadPixels(gui->mouse_x, gui->mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &gui->mouse_z);
    
    glDepthFunc(GL_ALWAYS);
    
    
    /*hilite test */
    if((gui->draw_tmp)&&(gui->element != NULL)){
      gui->element->obj.graphics = dxf_graph_parse(gui->drawing, gui->element, 0 , 1);
    }
    
    d_param.subst = &gui->hilite;
    d_param.len_subst = 1;
    d_param.inc_thick = 3;
    
    
    if(gui->element != NULL){
      graph_list_draw_gl2(gui->element->obj.graphics, &gui->gl_ctx, d_param);
    }
    if((gui->draw_phanton)&&(gui->phanton)){
      graph_list_draw_gl2(gui->phanton, &gui->gl_ctx, d_param);
    }
    if (gui->sel_list->next) {/* verify if  has elements in list */
      dxf_list_draw_gl(gui->sel_list, &gui->gl_ctx, d_param);
    }
    
    
    if ((gui->draw_vert) && (gui->element)){
      /* draw vertices */
      gui_draw_vert_gl(gui, gui->element);
    }
    
    if (gui->near_attr){ /* check if needs to draw an attractor mark */
      /* convert entities coordinates to screen coordinates */
      int attr_x = (int) round((gui->near_x - gui->ofs_x) * gui->zoom);
      int attr_y = (int) round((gui->near_y - gui->ofs_y) * gui->zoom);
      draw_attractor_gl(gui, gui->near_attr, attr_x, attr_y, yellow);
    }
    
    
    /* -------- macro --  rendering text by default general drawing engine 
    if (macro_len > 0){
      list_node * graph = list_new(NULL, FRAME_LIFE);
      if (graph) {
        if (font_parse_str(gui->dflt_font, graph, FRAME_LIFE, macro, NULL, 0)){
          graph_list_color(graph, white);
          graph_list_modify(graph, 230, 100, 20.0, 20.0, 0.0);
          
          struct draw_param param = {.ofs_x = 0, .ofs_y = 0, .scale = 1.0, .list = NULL, .subst = NULL, .len_subst = 0, .inc_thick = 0};
          graph_list_draw_gl(graph, &gui->gl_ctx, param);
        }
      }
    }
    */
    
    /* ---------------------------------- */
    draw_gl (&gui->gl_ctx, 1); /* force draw and cleanup */
    
    /*draw gui*/
    nk_gl_render(gui);
    
    gui->gl_ctx.vert_count = 0;
    gui->gl_ctx.elem_count = 0;
    
    /* Swap buffers */
    SDL_GL_SwapWindow(gui->window);
    
    gui->draw = 0;
    
  }
  
  leftMouseButtonClick = 0;
  rightMouseButtonClick = 0;
  MouseMotion = 0;
  gui->keyEnter = 0;
  key_space = 0;
  key_esc = 0;
  gui->ev = EV_NONE;
  
  graph_mem_pool(ZERO_GRAPH, 1);
  graph_mem_pool(ZERO_LINE, 1);
  graph_mem_pool(ZERO_GRAPH, FRAME_LIFE);
  graph_mem_pool(ZERO_LINE, FRAME_LIFE);
  list_mem_pool(ZERO_LIST, FRAME_LIFE);
  dxf_mem_pool(ZERO_DXF, FRAME_LIFE);
  
  nk_clear(gui->ctx); /* <-------- IMPORTANT   */
  
  /* -------- macro */
  if (macro_timer < 40){
    macro_timer++;
  } else {
    if (macro_len) gui->draw = 1;
    macro_timer = 0;
    macro_len = 0;
    macro[0] = 0;
  }
  
  function_key[0] = 0;
  
  if (gui->script_resume_reason == YIELD_GUI_REFRESH){
    if (refresh_timer < 3){
      refresh_timer++;
    } else {
      refresh_timer = 0;
      gui->script_resume_reason = YIELD_NONE;
      gui->script_resume = 1;
    }
  }
  
  return running;
}