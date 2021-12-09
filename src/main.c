

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "dxf_graph.h"
#include "list.h"
#include "dxf_create.h"
#include "dxf_copy.h"
#include "dxf_attract.h"
#include "dxf_print.h"

#include "draw_gl.h"

#include "dxf_seed.h"

#include "i_svg_media.h"

#include "gui.h"
#include "gui_lay.h"
#include "gui_ltype.h"
#include "gui_tstyle.h"
#include "gui_info.h"
#include "gui_xy.h"
#include "gui_use.h"
#include "gui_file.h"
#include "gui_print.h"
#include "gui_config.h"
#include "gui_script.h"

#include "rref.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
//#include <locale.h>


#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_ZERO_COMMAND_MEMORY
#include "nuklear.h"


#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#ifdef __linux__
#define OS_LINUX
#elif defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#endif

#define KGFLAGS_IMPLEMENTATION
#include "kgflags.h"


/* ---------------------------------------------------------*/
/* ------------------   GLOBALS  ----------------------- */

#include "dxf_colors.h"
struct Matrix *aux_mtx1 = NULL;
struct tfont *dflt_font = NULL;

/* ---------------------------------------------------------*/


void zoom_ext(dxf_drawing *drawing, bmp_img *img, double *zoom, double *ofs_x, double *ofs_y){
	double min_x = 0.0, min_y = 0.0, max_x = 20.0, max_y = 20.0;
	double zoom_x = 1.0, zoom_y = 1.0;
	
	dxf_ents_ext(drawing, &min_x, &min_y, &max_x, &max_y);
	zoom_x = (max_x - min_x)/img->width;
	zoom_y = (max_y - min_y)/img->height;
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	if (*zoom <= 0){ *zoom =1;}
	else{ *zoom = 1/(1.1 * (*zoom));}
	
	*ofs_x = min_x - (fabs((max_x - min_x)*(*zoom) - img->width)/2)/(*zoom);
	*ofs_y = min_y - (fabs((max_y - min_y)*(*zoom) - img->height)/2)/(*zoom);
}

void zoom_ext2(dxf_drawing *drawing, int x, int y, int width, int height, double *zoom, double *ofs_x, double *ofs_y){
	double min_x = 0.0, min_y = 0.0, max_x = 20.0, max_y = 20.0;
	double zoom_x = 1.0, zoom_y = 1.0;
	
	dxf_ents_ext(drawing, &min_x, &min_y, &max_x, &max_y);
	zoom_x = fabs(max_x - min_x)/width;
	zoom_y = fabs(max_y - min_y)/height;
	
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	*zoom = 1/(1.1 * (*zoom));
	
	*ofs_x = min_x - ((fabs((max_x - min_x)*(*zoom) - width)/2)+x)/(*zoom);
	*ofs_y = min_y - ((fabs((max_y - min_y)*(*zoom) - height)/2)+y)/(*zoom);
}

int main(int argc, char** argv){
	aux_mtx1 = malloc(sizeof(struct Matrix));
	
	char macro[64];
	int macro_len = 0;
	int macro_timer = 0;
	macro[0] = 0;
	
	char function_key[20];
	function_key[0] = 0;
	
	gui_obj *gui = malloc(sizeof(gui_obj));
	gui_start(gui);
	
	
	
	
#ifdef _MSC_VER
  //let msvcrt do the charset convensions
  setlocale(LC_ALL, ".UTF-8");
#else
	//setlocale(LC_ALL,""); //seta a localidade como a current do computador para aceitar acentuacao
#endif
	int i, ok;
	
	
	
	SDL_Rect win_r, display_r;
	
	/* init the SDL2 */
	SDL_Init(SDL_INIT_VIDEO);
	
	
	/* --------------- Configure paths ----------*/
	char *base_path = SDL_GetBasePath();
	if (base_path){
		strncpy(gui->base_dir, base_path, DXF_MAX_CHARS);
		SDL_free(base_path);
	}
	
	
	char *pref_path = NULL;  // guaranteed to be assigned only if kgflags_parse succeeds
	kgflags_string("pref", NULL, "Preferences directory.", false, (const char **) &pref_path);
	kgflags_parse(argc, argv);
	
	if (pref_path){
		strncpy(gui->pref_path, dir_full(pref_path), DXF_MAX_CHARS);
	}
	
	if (strlen (gui->pref_path) == 0){
		pref_path = SDL_GetPrefPath("CadZinho", "CadZinho");
		if (pref_path){
			if (strlen (pref_path) > 0){
				if (pref_path[strlen (pref_path) - 1] == DIR_SEPARATOR){
					strncpy(gui->pref_path, pref_path, DXF_MAX_CHARS);
				}
				else {
					snprintf(gui->pref_path, DXF_MAX_CHARS, "%s%c", pref_path, DIR_SEPARATOR);
				}
			}
			SDL_free(pref_path);
		}
	}
	
	if (strlen (gui->pref_path) == 0){ /* pref path is not present */
		/* pref path = base path */
		strncpy(gui->pref_path, gui->base_dir, DXF_MAX_CHARS);
	}
	else {
		char new_path[PATH_MAX_CHARS+1];
		/* script path */
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sscript%c", gui->pref_path, DIR_SEPARATOR);
		dir_miss (new_path);
		
		/* resources path and seed file*/
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%c", gui->pref_path, DIR_SEPARATOR);
		dir_miss (new_path);
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%cseed.dxf", gui->pref_path, DIR_SEPARATOR);
		miss_file (new_path, (char *)dxf_seed_2007);
		
		/* fonts path */
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%cfont%c", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		dir_miss (new_path);
		
		/* line types path and files */
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%clin%c", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		dir_miss (new_path);
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%clin%cdefault.lin", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		miss_file (new_path, (char *)ltype_lib_dflt());
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%clin%cextra.lin", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		miss_file (new_path, (char *)ltype_lib_extra());
		
		/* hatch patterns path and file*/
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%cpat%c", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		dir_miss (new_path);
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sres%cpat%cdefault.pat", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
		miss_file (new_path, (char *)h_pattern_lib_dflt());
	}
	
	/* full path of clipboard file */
	char clip_path[DXF_MAX_CHARS + 1];
	clip_path[0] = 0;
	snprintf(clip_path, DXF_MAX_CHARS, "%sclipboard.dxf", gui->pref_path);
	
	/* full path of init file */
	char init_path[DXF_MAX_CHARS + 1];
	init_path[0] = 0;
	snprintf(init_path, DXF_MAX_CHARS, "%sinit.lua", gui->pref_path);
	
	if (strlen(gui->base_dir)){
		dir_change(gui->base_dir); /* change working dir to base path*/
	}
	
	/* -------------- load main configuration options ---------------*/
	gui_load_conf (gui);
	
	/* -------------- load last session states ---------------*/
	
	/* init the Lua instance, to run init */
	struct script_obj conf_script;
	conf_script.L = NULL;
	conf_script.T = NULL;
	conf_script.active = 0;
	conf_script.dynamic = 0;
	
	/* try to run init file */
	if (gui_script_init (gui, &conf_script, init_path, NULL) == 1){
		conf_script.time = clock();
		conf_script.timeout = 1.0; /* default timeout value */
		conf_script.do_init = 0;
		
		lua_getglobal(conf_script.T, "cz_main_func");
		int n_results = 0; /* for Lua 5.4*/
		conf_script.status = lua_resume(conf_script.T, NULL, 0, &n_results); /* start thread */
		if (conf_script.status != LUA_OK){
			conf_script.active = 0; /* error */			
		}
		/* finaly get states from global variables in Lua instance */
		gui_get_ini (conf_script.T);
		
		/* close script and clean instance*/
		lua_close(conf_script.L);
		conf_script.L = NULL;
		conf_script.T = NULL;
		conf_script.active = 0;
		conf_script.dynamic = 0;
	}
	
	/* -------------------------------------------------------------------------- */
	
	/* try to change working dir from last opened drawing */
	if (strlen(get_dir(gui->drwg_recent[gui->drwg_rcnt_size - 1])) > 0){
		dir_change(get_dir(gui->drwg_recent[gui->drwg_rcnt_size - 1]));
	}
	/* ------------------------------------------------------------------------*/
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	/* enable ati-aliasing */
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
	
	SDL_Window * window = SDL_CreateWindow(
		"CadZinho", /* title */
		gui->win_x, /* x position */
		gui->win_y, /* y position */
		gui->win_w, /* width */
		gui->win_h, /* height */
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE); /* flags */
		
		
	/* ------------------------------ opengl --------------------------------------*/
	gui->gl_ctx.ctx = SDL_GL_CreateContext(window);
	
	draw_gl_init ((void *)gui, 0);
	
	/* ------------------------------------------------------------------------------- */
	
	double pos_x, pos_y;

	int open_prg = 0;
	//int progress = 0;
	int progr_win = 0;
	int progr_end = 0;
	unsigned int quit = 0;
	unsigned int wait_open = 0;
	/*int show_app_about = 0;
	int show_app_file = 0;
	int path_ok = 0;
	int show_info = 0;
	int show_script = 0;
	int show_print = 0;*/
	struct draw_param d_param;
	
	//int hist_new = 0;
	
	
	char file_path[DXF_MAX_CHARS];
	int file_path_len = 0;
	
	//int show_lay_mng = 0, sel_tmp = 0, show_color_pick = 0, show_lay_name = 0;
	//int show_tstyles_mng = 0;
	
	int ev_type;
	//struct nk_color background;
	
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int leftMouseButtonClick = 0;
	int rightMouseButtonClick = 0;
	int MouseMotion = 0;
	int ctrlDown = 0;
	
	//int en_attr = 1;
	
	SDL_Event event;
	int low_proc = 1;
	
	char *url = NULL;
	//char const * lFilterPatterns[2] = { "*.dxf", "*.txt" };
	
	struct Mem_buffer *file_buf = NULL;
	long file_size = 0;
	
	char* dropped_filedir;                  /* Pointer for directory of dropped file */
	
	
	gui->show_file_br = 0;
	
	//gui->file_filter_types[0] = ext_types[FILE_DXF];
	//gui->file_filter_types[1] = ext_types[FILE_ALL];
	
	//gui->file_filter_descr[0] = ext_descr[FILE_DXF];
	//gui->file_filter_descr[1] = ext_descr[FILE_ALL];
	
	gui->file_filter_count = 0;
	gui->filter_idx = 0;
	
	gui->curr_path[0] = 0;
	
	/* Colors in use */
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	bmp_color green = {.r = 0, .g = 255, .b =0, .a = 255};
	bmp_color yellow = {.r = 255, .g = 255, .b =0, .a = 255};
	bmp_color grey = {.r = 100, .g = 100, .b = 100, .a = 255};
	
	/* line types in use */
	double center [] = {12, -6, 2 , -6};
	double dash [] = {8, -8};
	double continuous[] = {1};
	
	/* initialize the selection list */
	gui->sel_list = list_new(NULL, PRG_LIFE);
	
	/* initialize the undo/redo list */
	//struct do_list gui->list_do;
	init_do_list(&gui->list_do);
		
	/* init Nuklear GUI */
	
	nk_sdl_init(gui);
	
	//enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_GREEN};
	//~ set_style(gui, THEME_BLACK);
	//~ set_style(gui, THEME_WHITE);
	//~ set_style(gui, THEME_RED);
	//~ set_style(gui, THEME_BLUE);
	//~ set_style(gui, THEME_DARK);
	set_style(gui, gui->theme);
	
	
	
	
	/* init the toolbox image */
	
	
	gui->svg_curves = i_svg_all_curves();
	gui->svg_bmp = i_svg_all_bmp(gui->svg_curves, ICON_SIZE-1, ICON_SIZE-1);
	
	bmp_free(gui->svg_bmp[SVG_LOCK]);
	gui->svg_bmp[SVG_LOCK] = i_svg_bmp(gui->svg_curves[SVG_LOCK], 16, 16);
	bmp_free(gui->svg_bmp[SVG_UNLOCK]);
	gui->svg_bmp[SVG_UNLOCK] = i_svg_bmp(gui->svg_curves[SVG_UNLOCK], 16, 16);
	bmp_free(gui->svg_bmp[SVG_EYE]);
	gui->svg_bmp[SVG_EYE] = i_svg_bmp(gui->svg_curves[SVG_EYE], 16, 16);
	bmp_free(gui->svg_bmp[SVG_NO_EYE]);
	gui->svg_bmp[SVG_NO_EYE] = i_svg_bmp(gui->svg_curves[SVG_NO_EYE], 16, 16);
	bmp_free(gui->svg_bmp[SVG_SUN]);
	gui->svg_bmp[SVG_SUN] = i_svg_bmp(gui->svg_curves[SVG_SUN], 16, 16);
	bmp_free(gui->svg_bmp[SVG_FREEZE]);
	gui->svg_bmp[SVG_FREEZE] = i_svg_bmp(gui->svg_curves[SVG_FREEZE], 16, 16);
	bmp_free(gui->svg_bmp[SVG_CZ]);
	gui->svg_bmp[SVG_CZ] = i_svg_bmp(gui->svg_curves[SVG_CZ], 32, 32);
	
	gui->i_cz48 = i_svg_bmp(gui->svg_curves[SVG_CZ], 48, 48);
	gui->i_trash = i_svg_bmp(gui->svg_curves[SVG_TRASH], 16, 16);
	
	
	//struct nk_style_button b_icon_style;
	
	
	gui->color_img = bmp_new(15, 13, black, red);
	struct nk_image i_color = nk_image_ptr(gui->color_img);
	
	/* other font */
	
	
	
	{
		double font_size = 0.8;
		for (i = 0; i < FONT_NUM_SIZE; i++){
			gui->alt_font_sizes[i] = gui->ui_font;
			gui->alt_font_sizes[i].height = font_size * gui->ui_font.height;
			font_size += 0.2;
		}
	}
	
	//int show_blk_pp = 0;
	//bmp_img * gui->preview_img;
	gui->preview_img = bmp_new(160, 160, grey, red);
	//char tag_mark[DXF_MAX_CHARS];
	
	
	/* init global variable font */
	dflt_font = get_font_list(gui->font_list, "txt.shx"); /* GLOBAL GLOBAL*/
	
	
	/* init the drawing */
	gui->drawing = dxf_drawing_new(DWG_LIFE);
	gui->drawing->font_list = gui->font_list;
	gui->drawing->dflt_font = gui->dflt_font;
	
	url = NULL; /* pass a null file only for initialize the drawing structure */
	
	/* **************** init the main drawing ************ */
	
	while (dxf_read (gui->drawing, gui->seed, strlen(gui->seed), &gui->progress) > 0){
		
	}
	
	gui->layer_idx = dxf_lay_idx (gui->drawing, "0");
	gui->ltypes_idx = dxf_ltype_idx (gui->drawing, "BYLAYER");
	gui->t_sty_idx = dxf_tstyle_idx (gui->drawing, "STANDARD");
	gui->color_idx = 256;
	gui->lw_idx = DXF_LW_LEN;
	/* *************************************************** */
	
	/* **************** init the clipboard drawing ************ */
	gui->clip_drwg = dxf_drawing_new(ONE_TIME);
	
	while (dxf_read (gui->clip_drwg, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &gui->progress) > 0){
		
	}
	/* *************************************************** */
	
	gui_tstyle(gui);
	
	
	//printf(dxf_seed_r12);
	
	//dxf_new_block(gui->drawing, "teste", "0");
	//dxf_ent_print2(gui->drawing->blks);
	//dxf_ent_print2(gui->drawing->blks_rec);
	//dxf_ent_print2(gui->drawing->head);
	/*-------------------- test -----------------------*/
	//printf("version = %d\n", gui->drawing->version);
	/*---------------------------------------*/
	
	dxf_ents_parse(gui->drawing);
	

	
	
	
	
	
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
		(void *)gui->svg_bmp[SVG_CZ]->buf,
		gui->svg_bmp[SVG_CZ]->width,
		gui->svg_bmp[SVG_CZ]->height,
		32, gui->svg_bmp[SVG_CZ]->width * 4,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	// The icon is attached to the window pointer
	SDL_SetWindowIcon(window, surface);

	// ...and the surface containing the icon pixel data is no longer required.
	SDL_FreeSurface(surface);

	

		
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	
	
	
	/*-------------------------------- keyboard macros script --------------------- */
	struct script_obj macro_script;
	macro_script.L = NULL;
	macro_script.T = NULL;
	macro_script.active = 0;
	macro_script.dynamic = 0;
	
	/* full path of macro file */
	char macro_path[DXF_MAX_CHARS + 1];
	macro_path[0] = 0;
	snprintf(macro_path, DXF_MAX_CHARS, "%smacro.lua", gui->pref_path);
	
	miss_file (macro_path, (char*)macro_dflt_file);
	
	if (gui_script_init (gui, &macro_script, macro_path, NULL) == 1){
		macro_script.active = 1;
	}
	
	
	/*-------------------------------- functions keys script --------------------- */
	struct script_obj func_keys_script;
	func_keys_script.L = NULL;
	func_keys_script.T = NULL;
	func_keys_script.active = 0;
	func_keys_script.dynamic = 0;
	
	/* full path of func_keys file */
	char func_keys_path[DXF_MAX_CHARS + 1];
	func_keys_path[0] = 0;
	snprintf(func_keys_path, DXF_MAX_CHARS, "%sfunc_keys.lua", gui->pref_path);
	
	miss_file (func_keys_path, (char*)func_key_dflt_file);
	
	if (gui_script_init (gui, &func_keys_script, func_keys_path, NULL) == 1){
		func_keys_script.active = 1;
	}
	
	/*===================== teste ===============*/
	
	//list_node * tt_test = list_new(NULL, PRG_LIFE);
	//double w_teste;
	//tt_parse4(tt_test, PRG_LIFE, "Ezequiel Rabelo de Aguiar  AV çβμπ");
	
	//struct tfont *font_teste = get_font_list(gui->font_list, "OpenSans-Light.ttf");
	//struct tfont *font_teste = get_font_list(gui->font_list, "Roboto-LightItalic.ttf");
	//font_str_w(font_teste, "Ezequiel Rabelo de Aguiar  AV çβμπ", &w_teste);
	//printf("\ntext w1 = %0.2f\n", w_teste);
	//font_parse_str(font_teste, tt_test, PRG_LIFE, "Ezequiel Rabelo de Aguiar  AV çβμπ", NULL);
	/*font_teste = get_font_list(gui->font_list, "romans.shx");
	
	font_str_w(font_teste, "Ezequiel Rabelo de Aguiar  AV çβμπ", &w_teste);
	printf("\ntext w1 = %0.2f\n", w_teste);
	font_parse_str(font_teste, tt_test, PRG_LIFE, "Ezequiel Rabelo de Aguiar  AV çβμπ", NULL);
	*/
	
	//graph_obj * hers = hershey_test (PRG_LIFE);
	/*===================== teste ===============*/
	
	const Uint8* state;
	
	/* main loop */
	while (quit == 0){
		ev_type = 0;
		low_proc = 1;
		
		//SDL_ShowCursor(SDL_DISABLE);
		
		/* get events for Nuklear GUI input */
		nk_input_begin(gui->ctx);
		if(SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT) quit = 1;
			nk_sdl_handle_event(gui, window, &event);
			ev_type = event.type;
		}
		nk_input_end(gui->ctx);
		
		/* ===============================*/
		if (nk_window_is_any_hovered(gui->ctx)) {
			SDL_ShowCursor(SDL_ENABLE);
			//printf("show\n");
		}
		else{
			SDL_ShowCursor(SDL_DISABLE);
			
			if (ev_type != 0){
				double wheel = 1.0;
				switch (event.type){
					case SDL_MOUSEBUTTONUP:
						gui->mouse_x = event.button.x;
						gui->mouse_y = event.button.y;
						gui->mouse_y = gui->win_h - gui->mouse_y;
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
						if (event.button.button == SDL_BUTTON_LEFT){
							leftMouseButtonDown = 1;
							leftMouseButtonClick = 1;
						}
						else if(event.button.button == SDL_BUTTON_RIGHT){
							rightMouseButtonDown = 1;
							rightMouseButtonClick = 1;
						}
						break;
					case SDL_MOUSEMOTION:
						MouseMotion = 1;
						gui->mouse_x = event.motion.x;
						gui->mouse_y = event.motion.y;
						gui->mouse_y = gui->win_h - gui->mouse_y;
						pos_x = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
						pos_y = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
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
					
					case (SDL_DROPFILE): {      // In case if dropped file
						/* get file path -> drop event has previously proccessed and string was moved to clipboard !!!*/
						char *dropped_filedir = SDL_GetClipboardText(); 
						strncpy (gui->curr_path, dropped_filedir, DXF_MAX_CHARS);
						//printf("File: %s\n", gui->curr_path);
						/* open file */
						gui->action = FILE_OPEN;
						gui->path_ok = 1;
						gui->hist_new = 1; /* not change history entries */
						
						SDL_free(dropped_filedir);    // Free dropped_filedir memory
						}
						break;
					case SDL_KEYDOWN: {
						SDL_Keycode key = event.key.keysym.sym;
						SDL_Keymod mod = event.key.keysym.mod;
						int n_func = sizeof(func_keys)/sizeof(struct func_key);
						for (i = 0; i < n_func; i++){
							if (func_keys[i].code == key && (func_keys[i].mod & mod || func_keys[i].mod == KMOD_NONE)){
								strncpy (function_key, func_keys[i].key, 19);
							}
						}
						
						if (key == SDLK_RCTRL || key == SDLK_LCTRL) ctrlDown = 1;
					
						//printf("%s\n", SDL_GetKeyName(event.key.keysym.sym));
						if ((key == SDLK_RETURN) || (key == SDLK_RETURN2)){
							gui->keyEnter = 1;
						}
						else if (key == SDLK_UP){
							gui->action = VIEW_PAN_U;
							//printf("%d\n", SDL_GetKeyFromName("x"));
							//printf("%d\n", SDL_GetKeyFromName("X"));
						}
						else if (key == SDLK_DOWN){
							gui->action = VIEW_PAN_D;
						}
						else if (key == SDLK_LEFT){
							gui->action = VIEW_PAN_L;
						}
						else if (key == SDLK_RIGHT){
							gui->action = VIEW_PAN_R;
						}
						else if (key == SDLK_KP_MINUS){
							gui->action = VIEW_ZOOM_M;
						}
						else if (key == SDLK_KP_PLUS){
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
		if (rightMouseButtonClick) gui->ev |= EV_CANCEL;
		if (gui->keyEnter) gui->ev |= EV_LOCK_AX;
		if (ctrlDown) gui->ev |= EV_ADD;
		
		gui->next_win_h = 6 + 4 + ICON_SIZE + 4 + 6 + 4 + ICON_SIZE + 4 + 6 + 8;
		gui->next_win_x = 2;
		gui->next_win_y = 2;
		
		/*
		if (nk_begin(gui->ctx, "Icons", nk_rect(500, 50, 200, 500),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){
			nk_layout_row_static(gui->ctx, 28, 28, 6);
			for (i = 0; i <  SVG_MEDIA_SIZE; i++){
				nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[i]));
			}
			
		}
		nk_end(gui->ctx);*/
		
		gui->next_win_y += gui->next_win_h + 3;
		gui->next_win_w = 210;
		gui->next_win_h = 500;
		
		gui_tools_win (gui);
		gui_main_win (gui);
		
		
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
		
		if (gui->show_blk_mng){
			gui->show_blk_mng = gui_blk_mng (gui);
		}
		
		if (gui->show_tstyles_mng){
			gui->show_tstyles_mng = tstyles_mng (gui);
		}
		
		if (gui->show_config){ /* configuration window */
			gui->show_config = config_win (gui);
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
				nk_progress(gui->ctx, (nk_size *)&gui->progress, 100, NK_FIXED);
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
			low_proc = 0;
			gui->draw = 1;
			
			open_prg = dxf_read(gui->drawing, file_buf->buffer, file_size, &gui->progress);
			
			if(open_prg <= 0){
				//free(file_buf);
				manage_buffer(0, BUF_RELEASE);
				file_buf = NULL;
				file_size = 0;
				low_proc = 1;
				
				gui->drawing->font_list = gui->font_list;
				gui->drawing->dflt_font = gui->dflt_font;
				
				//dxf_ent_print2(gui->drawing->blks);
				gui_tstyle(gui);
				dxf_ents_parse(gui->drawing);				
				gui->action = VIEW_ZOOM_EXT;
				gui->layer_idx = dxf_lay_idx (gui->drawing, "0");
				gui->ltypes_idx = dxf_ltype_idx (gui->drawing, "BYLAYER");
				gui->t_sty_idx = dxf_tstyle_idx (gui->drawing, "STANDARD");
				gui->color_idx = 256;
				gui->lw_idx = DXF_LW_LEN;
				wait_open = 0;
				progr_end = 1;
				sel_list_clear (gui);
			}
			
		}
		else low_proc = 1;
		
		
		/*===============================*/
		
		if((gui->action == FILE_OPEN) && (gui->path_ok)) {
			gui->action = NONE; gui->path_ok = 0;
			
			dxf_mem_pool(ZERO_DXF, DWG_LIFE);
			graph_mem_pool(ZERO_GRAPH, DWG_LIFE);
			graph_mem_pool(ZERO_LINE, DWG_LIFE);
			
			wait_open = 1;
			gui->progress = 0;
			
			file_buf = load_file_reuse(gui->curr_path, &file_size);
			gui->drawing->font_list = gui->font_list;
			gui->drawing->dflt_font = get_font_list(gui->font_list, "txt.shx");
			open_prg = dxf_read(gui->drawing, file_buf->buffer, file_size, &gui->progress);
			
			low_proc = 0;
			progr_win = 1;
			
			SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			
			/* add file to history */
			if (gui->hist_new && open_prg >= 0){
				/* get position in array, considering as circular buffer */
				int pos = (gui->drwg_hist_wr + gui->drwg_hist_head) % DRWG_HIST_MAX;
				/* put file path in history */
				strncpy (gui->drwg_hist[pos], gui->curr_path , DXF_MAX_CHARS);
				/* history buffer is full -> change the head of buffer to overwrite first entry */
				if(gui->drwg_hist_wr >= DRWG_HIST_MAX) gui->drwg_hist_head++;
				/* adjust circular buffer head */
				gui->drwg_hist_head %= DRWG_HIST_MAX;
				
				/* adjust buffer parameters to next entries */
				if (gui->drwg_hist_pos < gui->drwg_hist_size && gui->drwg_hist_pos < DRWG_HIST_MAX - 1)
					gui->drwg_hist_pos ++; /* position */
				if (gui->drwg_hist_wr < DRWG_HIST_MAX){
					gui->drwg_hist_wr ++; /* size */
					gui->drwg_hist_size = gui->drwg_hist_wr; /* next write position */
				}
				gui->hist_new = 0;
				
				/* put file path in recent file list */
				strncpy (gui->drwg_recent[gui->drwg_rcnt_pos], gui->curr_path , DXF_MAX_CHARS);
				if (gui->drwg_rcnt_pos < DRWG_RECENT_MAX - 1)
					gui->drwg_rcnt_pos++;
				else gui->drwg_rcnt_pos = 0; /* circular buffer */
				
				if (gui->drwg_rcnt_size < DRWG_RECENT_MAX)
					gui->drwg_rcnt_size++;
				
			}
			if (open_prg >= 0){
				strncpy (gui->dwg_dir, get_dir(gui->curr_path) , DXF_MAX_CHARS);
				strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
				
				char title[DXF_MAX_CHARS] = "CadZinho - ";
				strncat (title, gui->dwg_file, DXF_MAX_CHARS);
				SDL_SetWindowTitle(window, title);
			}
		}
		else if((gui->action == FILE_SAVE) && (gui->path_ok)){
			gui->action = NONE; gui->path_ok = 0;
			
			if (gui->drawing->main_struct != NULL){
				//dxf_ent_print_f (gui->drawing->main_struct, url);
				dxf_save (gui->curr_path, gui->drawing);
				
				strncpy (gui->dwg_dir, get_dir(gui->curr_path) , DXF_MAX_CHARS);
				strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
				
				char title[DXF_MAX_CHARS] = "CadZinho - ";
				strncat (title, gui->dwg_file, DXF_MAX_CHARS);
				SDL_SetWindowTitle(window, title);
			}
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
			zoom_ext2(gui->drawing, 0, 0, gui->win_w, gui->win_h, &gui->zoom, &gui->ofs_x, &gui->ofs_y);
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
		else if((gui->action == YANK || gui->action == CUT) && strlen(clip_path) > 0) {
			if (gui->sel_list->next){ /* verify if  has elements in list */
				/* clear the clipboard drawing and init with basis seed */
				dxf_drawing_clear(gui->clip_drwg);
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
				dxf_save (clip_path, gui->clip_drwg);
				
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
								//printf("add item = %d\n", current->data);
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
		else if (gui->action == START_PASTE && strlen(clip_path) > 0) {
			/* clear memory pool used before */
			dxf_mem_pool(ZERO_DXF, ONE_TIME);
			graph_mem_pool(ZERO_GRAPH, ONE_TIME);
			graph_mem_pool(ZERO_LINE, ONE_TIME);
			dxf_drawing_clear(gui->clip_drwg);
			/* load the clipboard file */
			file_size = 0;
			file_buf = load_file_reuse(clip_path, &file_size);
			while (dxf_read (gui->clip_drwg, file_buf->buffer, file_size, &gui->progress) > 0){
				
			}
			/* load and apply the fonts required for clipboard drawing */
			gui_tstyle2(gui, gui->clip_drwg);
			
			/* clear the file buffer */
			//free(file_buf);
			manage_buffer(0, BUF_RELEASE);
			file_buf = NULL;
			file_size = 0;
			
			/* prepare for next steps on paste */
			gui->modal = PASTE;
			gui->step = 0;
			gui->action = NONE;
			gui->draw = 1;
		}
		else if(gui->action == DELETE){
			gui->action = NONE;
		
			if (gui->sel_list->next){ /* verify if  has elements in list */
				
				do_add_entry(&gui->list_do, "DELETE");
				
				list_node *current = gui->sel_list->next;
				
				// starts the content sweep 
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							
							if (do_add_item(gui->list_do.current, (dxf_node *)current->data, NULL)) {
								//printf("add item = %d\n", current->data);
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
							
							dxf_attr_change(new_ent, 8, gui->drawing->layers[gui->layer_idx].name);
							
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
							
							dxf_attr_change(new_ent, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
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
		
		if (gui->prev_modal != gui->modal){
			
			gui->en_distance = 0;
			gui->draw_tmp = 0;
			gui->element = NULL;
			gui->draw = 1;
			gui->step = 0;
			gui->draw_phanton = 0;
			if (gui->phanton){
				//free(gui->phanton->data);
				//free(gui->phanton);
				gui->phanton = NULL;
			}
			gui->lock_ax_x = 0;
			gui->lock_ax_y = 0;
			gui->user_flag_x = 0;
			gui->user_flag_y = 0;
			//printf("change tool\n");
			
			if (gui->prev_modal == SCRIPT){
				gui->lua_script[0].active = 0;
				gui->lua_script[0].dynamic = 0;
			}
			
			gui->prev_modal = gui->modal;
		}
		
		/*-------------------------------- macro script --------------------- */
		if ( macro_script.active && strlen(macro) > 0 ){
			macro_script.time = clock();
			macro_script.timeout = 1.0; /* default timeout value */
			macro_script.do_init = 0;
			
			lua_pushstring(macro_script.T, macro);
			lua_setglobal(macro_script.T, "macro");
			
			lua_pushboolean(macro_script.T, 0);
			lua_setglobal(macro_script.T, "accept");
			//print_lua_stack(macro_script.T);
			
			lua_getglobal(macro_script.T, "cz_main_func");
			int n_results = 0; /* for Lua 5.4*/
			macro_script.status = lua_resume(macro_script.T, NULL, 0, &n_results); /* start thread */
			if (macro_script.status != LUA_OK){
				macro_script.active = 0;
				
			}
			else {
				
				lua_getglobal(macro_script.T, "accept");
				if (lua_toboolean(macro_script.T, -1)){
					macro_timer = 0;
					macro_len = 0;
					macro[0] = 0;
				}
				lua_pop(macro_script.T, 1);
			}
		}
		
		/*-------------------------------- function key script --------------------- */
		if ( func_keys_script.active && strlen(function_key) > 0 ){
			func_keys_script.time = clock();
			func_keys_script.timeout = 1.0; /* default timeout value */
			func_keys_script.do_init = 0;
			
			lua_pushstring(func_keys_script.T, function_key);
			lua_setglobal(func_keys_script.T, "function_key");
			
			if (luaL_dofile (func_keys_script.T,  func_keys_path) != LUA_OK) {
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
		gui_arc_interactive(gui);
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
			SDL_GetWindowSize(window, &gui->win_w, &gui->win_h);
			SDL_GetWindowPosition (window, &gui->win_x, &gui->win_y);
			
			
			glUniform1i(gui->gl_ctx.tex_uni, 0);
			
			SDL_GetWindowSize(window, &gui->gl_ctx.win_w, &gui->gl_ctx.win_h);
			glViewport(0, 0, gui->gl_ctx.win_w, gui->gl_ctx.win_h);
			
			
			d_param.ofs_x = gui->ofs_x;
			d_param.ofs_y = gui->ofs_y;
			d_param.scale = gui->zoom;
			d_param.list = NULL;
			d_param.subst = NULL;
			d_param.len_subst = 0;
			d_param.inc_thick = 0;
			
			/*
			gui->gl_ctx.transf[0][0] = 0.7071;
			gui->gl_ctx.transf[0][1] = 0.0;
			gui->gl_ctx.transf[0][2] = -0.7071;
			gui->gl_ctx.transf[0][3] = 0.0;
			gui->gl_ctx.transf[1][0] = 0.4082;
			gui->gl_ctx.transf[1][1] = 0.8165;
			gui->gl_ctx.transf[1][2] = 0.4082;
			gui->gl_ctx.transf[1][3] = 0.0;
			gui->gl_ctx.transf[2][0] = 0.5774;
			gui->gl_ctx.transf[2][1] = -0.5774;
			gui->gl_ctx.transf[2][2] = 0.5774;
			gui->gl_ctx.transf[2][3] = 0.0;
			gui->gl_ctx.transf[3][0] = 0.0;
			gui->gl_ctx.transf[3][1] = 0.0;
			gui->gl_ctx.transf[3][2] = 0.0;
			gui->gl_ctx.transf[3][3] = 1.0;
			glUniformMatrix4fv(gui->gl_ctx.transf_uni, 1,  GL_FALSE, &gui->gl_ctx.transf[0][0]);
			glDepthFunc(GL_LESS);
			*/
			
			/* Clear the screen to background color */
			glClearColor((GLfloat) gui->background.r/255, (GLfloat) gui->background.g/255, 
				(GLfloat) gui->background.b/255, 1.0);
			glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
			
			gui->gl_ctx.verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			gui->gl_ctx.elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
			gui->gl_ctx.vert_count = 0;
			gui->gl_ctx.elem_count = 0;
			glUniform1i(gui->gl_ctx.tex_uni, 0);
			
			
			dxf_ents_draw_gl(gui->drawing, &gui->gl_ctx, d_param);
			
			
			draw_cursor_gl(gui, gui->mouse_x, gui->mouse_y, gui->cursor);
			
			
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
			
			
			
			
			win_r.x = 0; win_r.y = 0;
			win_r.w = gui->win_w; win_r.h = gui->win_h;
			
			
			/*draw gui*/
			nk_gl_render(gui);
			
			//SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			
			gui->gl_ctx.vert_count = 0;
			gui->gl_ctx.elem_count = 0;
			
			/* Swap buffers */
			SDL_GL_SwapWindow(window);
			
			gui->draw = 0;
			
		}
		
		leftMouseButtonClick = 0;
		rightMouseButtonClick = 0;
		MouseMotion = 0;
		gui->keyEnter = 0;
		gui->ev = EV_NONE;
		
		//graph_mem_pool(ZERO_GRAPH, 2);
		//graph_mem_pool(ZERO_LINE, 2);
		
		//gui->phanton = NULL;
		
		graph_mem_pool(ZERO_GRAPH, 1);
		graph_mem_pool(ZERO_LINE, 1);
		graph_mem_pool(ZERO_GRAPH, FRAME_LIFE);
		graph_mem_pool(ZERO_LINE, FRAME_LIFE);
		list_mem_pool(ZERO_LIST, FRAME_LIFE);
		dxf_mem_pool(ZERO_DXF, FRAME_LIFE);
		
		nk_clear(gui->ctx); /*IMPORTANT */
		if (low_proc){
			SDL_Delay(30);
			SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEMOTION);
		}
		
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
	}
	
	/* safe quit */
	
	/* Delete allocated resources */
	draw_gl_init ((void *)gui, 1);
	
	SDL_GL_DeleteContext(gui->gl_ctx.ctx);
	
	dxf_drawing_clear(gui->drawing);
	dxf_drawing_clear(gui->clip_drwg);
	
	//SDL_DestroyTexture(canvas);
	//SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	SDL_Quit();
	
	if(free_font_list(gui->font_list)){
		//printf("\n------------------------------------------\n         FREE FONT OK\n---------------------------------\n");
	}
	
	
	gui_save_init (init_path, gui);
	
	list_mem_pool(FREE_LIST, 0);
	list_mem_pool(FREE_LIST, 1);
	list_mem_pool(FREE_LIST, ONE_TIME);
	list_mem_pool(FREE_LIST, PRG_LIFE);
	list_mem_pool(FREE_LIST, SEL_LIFE);
	dxf_mem_pool(FREE_DXF, 0);
	graph_mem_pool(FREE_ALL, 0);
	graph_mem_pool(FREE_ALL, 1);
	graph_mem_pool(FREE_ALL, 2);
	graph_mem_pool(FREE_ALL, PRG_LIFE);
	
	do_mem_pool(FREE_DO_ALL);
	
	bmp_free(gui->color_img);
	bmp_free(gui->preview_img);
	bmp_free(gui->i_cz48);
	bmp_free(gui->i_trash);
	
	i_svg_free_bmp(gui->svg_bmp);
	i_svg_free_curves(gui->svg_curves);
	
	
	
	
	//dxf_hatch_free(gui->list_pattern.next);
	dxf_h_fam_free(gui->hatch_fam.next);
	
	if (gui->seed) free(gui->seed);
	if (gui->dflt_pat) free(gui->dflt_pat);
	if (gui->dflt_lin) free(gui->dflt_lin);
	if (gui->extra_lin) free(gui->extra_lin);
	
	free(gui->clip_drwg);
	
	free(gui->drawing);
	free(aux_mtx1);
	nk_sdl_shutdown(gui);
	manage_buffer(0, BUF_FREE);
	
	/*-------------------------------- macro script clean-up--------------------- */
	if (macro_script.L){
		lua_close(macro_script.L);
	}
	
	/*-------------------------------- func_keys script clean-up--------------------- */
	if (func_keys_script.L){
		lua_close(func_keys_script.L);
	}
	
	return 0;
	
};

/*
//measure time
clock_t start = clock();
//Do something
clock_t end = clock();
float seconds = (float)(end - start) / CLOCKS_PER_SEC;
*/

/*
https://stackoverflow.com/questions/8972925/dxf-parser-ellipses-angle-direction


The ellipse might not lie in the 2D XY plane so just using the sign of the Z component of the extrusion direction isn't safe. Here's a more general approach for a 3D ellipse:

1) Create the ellipse in the XY plane with the major axis in the +X direction and going counter-clockwise from start parameter (group code 41) to end parameter (group code 42). First make sure the end parameter is greater than the start parameter and add 2pi if it's not. You can then calculate each point with:

X = [length of major radius] * cos(angle)
Y = [length of minor radius] * sin(angle)
2) Rotate it to this new coordinate system:

Direction of new X axis = endpoint of major axis
Direction of new Z axis = extrusion direction
Direction of new Y axis = [new Z axis] cross product [new X axis]

You can do this by normalizing these vectors and making a 3x3 transformation matrix where each column contains one of the vectors, then multiply this matrix by every point in the ellipse created in step 1.


answered Sep 11 '13 at 3:08

user1318499
*/