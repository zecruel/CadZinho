

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
#include "gui_export.h"
#include "gui_config.h"
#include "gui_script.h"

#include "rref.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
#include <locale.h>


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

#define  STRPOOL_IMPLEMENTATION
#include "strpool.h"

#ifdef __linux__
#define OS_LINUX
#elif defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#endif

#define KGFLAGS_IMPLEMENTATION
#include "kgflags.h"


/* ---------------------------------------------------------*/
/* ------------------   GLOBALS  ----------------------- */

#include "dxf_globals.h"
struct Matrix *aux_mtx1 = NULL;
struct tfont *dflt_font = NULL;

/* ---------------------------------------------------------*/

int main(int argc, char** argv){
	aux_mtx1 = malloc(sizeof(struct Matrix));
	
	gui_obj *gui = malloc(sizeof(gui_obj));
	gui_start(gui);
	
	int update_title = 0, changed = 0;
	
	
  setlocale(LC_ALL, "C.UTF-8");
	int i, ok, key_space = 0, key_esc = 0;
	
	time_t t;
	/* Intializes random number generator */
	srand((unsigned) time(&t));
	
	/* init the SDL2 */
	SDL_Init(SDL_INIT_VIDEO);
	
	
	/* --------------- Configure paths ----------*/
	char *base_path = SDL_GetBasePath();
	if (base_path){
		strncpy(gui->base_dir, base_path, PATH_MAX_CHARS);
		SDL_free(base_path);
	}
	
	
	char *pref_path = NULL;  // guaranteed to be assigned only if kgflags_parse succeeds
	kgflags_string("pref", NULL, "Preferences directory.", false, (const char **) &pref_path);
	kgflags_parse(argc, argv);
	//int kgflags_get_non_flag_args_count(void);
	const char * arg_file = kgflags_get_non_flag_arg(0);
	
	if (pref_path){
		strncpy(gui->pref_path, dir_full(pref_path), PATH_MAX_CHARS);
	}
	
	if (strlen (gui->pref_path) == 0){
		pref_path = SDL_GetPrefPath("CadZinho", "CadZinho");
		if (pref_path){
			if (strlen (pref_path) > 0){
				if (pref_path[strlen (pref_path) - 1] == DIR_SEPARATOR){
					strncpy(gui->pref_path, pref_path, PATH_MAX_CHARS);
				}
				else {
					snprintf(gui->pref_path, PATH_MAX_CHARS, "%s%c", pref_path, DIR_SEPARATOR);
				}
			}
			SDL_free(pref_path);
		}
	}
	
	if (strlen (gui->pref_path) == 0){ /* pref path is not present */
		/* pref path = base path */
		strncpy(gui->pref_path, gui->base_dir, PATH_MAX_CHARS);
	}
	else {
		char new_path[PATH_MAX_CHARS+1];
		
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

		/* plugins, script path and files */
		new_path[0] = 0;
		snprintf(new_path, PATH_MAX_CHARS, "%sscript%c", gui->pref_path, DIR_SEPARATOR);
		if (!dir_check (new_path)){
      if (dir_make (new_path)){
        new_path[0] = 0;
		    snprintf(new_path, PATH_MAX_CHARS,
            "%sscript%creg_poly_plugin.lua", gui->pref_path,
            DIR_SEPARATOR);
		    miss_file (new_path, (char *)plugin_rpoly_file);
        new_path[0] = 0;
		    snprintf(new_path, PATH_MAX_CHARS,
            "%sscript%cbatch_print_plugin.lua", gui->pref_path,
            DIR_SEPARATOR);
		    miss_file (new_path, (char *)plugin_bprint_file);
        new_path[0] = 0;
		    snprintf(new_path, PATH_MAX_CHARS,
            "%sscript%ccatenary_plugin.lua", gui->pref_path,
            DIR_SEPARATOR);
		    miss_file (new_path, (char *)plugin_catenary_file);
        new_path[0] = 0;
		    snprintf(new_path, PATH_MAX_CHARS,
            "%sscript%chyperbolic.lua", gui->pref_path,
            DIR_SEPARATOR);
		    miss_file (new_path, (char *)plugin_hyper_file);
        new_path[0] = 0;
		    snprintf(new_path, PATH_MAX_CHARS,
            "%splugins.lua", gui->pref_path);
		    miss_file (new_path, (char *)plugins_dflt_file);

      }
    }

	}
	
	/* full path of clipboard file */
	gui->clip_path[0] = 0;
	snprintf(gui->clip_path, DXF_MAX_CHARS, "%sclipboard.dxf", gui->pref_path);
	
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
	
	gui->window = SDL_CreateWindow(
		"CadZinho", /* title */
		gui->win_x, /* x position */
		gui->win_y, /* y position */
		gui->win_w, /* width */
		gui->win_h, /* height */
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE); /* flags */
  
	/* ------------------------------ opengl --------------------------------------*/
	gui->gl_ctx.ctx = SDL_GL_CreateContext(gui->window);
	draw_gl_init ((void *)gui, 0);
	
	/* ------------------------------------------------------------------------------- */
	
	char *url = NULL;
	
	gui->file_buf = NULL;
	gui->file_size = 0;
	
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
	init_do_list(&gui->list_do);
	gui->save_pt = gui->list_do.current;
		
	/* init Nuklear GUI */
	
	nk_sdl_init(gui);
	set_style(gui, gui->theme);
	
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
  
  /* init preview images */
  for (i = 0; i < PRV_SIZE; i++){
    gui->preview[i] = bmp_new(160, 160, grey, red);
  }
	
	/* init global variable font */
	dflt_font = get_font_list(gui->font_list, "txt.shx"); /* GLOBAL GLOBAL*/
	
	
	/* init the drawing */
	gui->drawing = dxf_drawing_new(DWG_LIFE);
	
	
	url = NULL; /* pass a null file only for initialize the drawing structure */
	
	/* **************** init the main drawing ************ */
	
	/* load and apply the fonts required for drawing */
	gui->drawing->font_list = gui->font_list;
	gui->drawing->dflt_font = get_font_list(gui->font_list, "txt.shx");
	gui->drawing->dflt_fonts_path = gui->dflt_fonts_path;
	
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
	
	/* load and apply the fonts required for clipboard drawing */
	gui->clip_drwg->font_list = gui->font_list;
	gui->clip_drwg->dflt_font = get_font_list(gui->font_list, "txt.shx");
	gui->clip_drwg->dflt_fonts_path = gui->dflt_fonts_path;
	
	while (dxf_read (gui->clip_drwg, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &gui->progress) > 0){
		
	}
	/* *************************************************** */
	
	dxf_ents_parse(gui->drawing);
	
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
		(void *)gui->svg_bmp[SVG_CZ]->buf,
		gui->svg_bmp[SVG_CZ]->width,
		gui->svg_bmp[SVG_CZ]->height,
		32, gui->svg_bmp[SVG_CZ]->width * 4,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	/* The icon is attached to the window pointer */
	SDL_SetWindowIcon(gui->window, surface);

	/* ...and the surface containing the icon pixel data is no longer required. */
	SDL_FreeSurface(surface);

	
	/* ****************** test cursor ************************** */
	gui->dflt_cur = SDL_GetDefaultCursor();
	gui_create_modal_cur(gui);
	/* ******************************************************* */
		
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	
	/*-------------------------------- keyboard macros script --------------------- */
	gui->macro_script.L = NULL;
	gui->macro_script.T = NULL;
	gui->macro_script.active = 0;
	gui->macro_script.dynamic = 0;
	
	/* full path of macro file */
	char macro_path[DXF_MAX_CHARS + 1];
	macro_path[0] = 0;
	snprintf(macro_path, DXF_MAX_CHARS, "%smacro.lua", gui->pref_path);
	
	miss_file (macro_path, (char*)macro_dflt_file);
	
	if (gui_script_init (gui, &gui->macro_script, macro_path, NULL) == 1){
		gui->macro_script.active = 1;
    lua_getglobal(gui->macro_script.T, "cz_main_func"); /* stack pos = 1 (always in stack to prevent garbage colector) */
	}
	
	/*-------------------------------- functions keys script --------------------- */
	gui->func_keys_script.L = NULL;
	gui->func_keys_script.T = NULL;
	gui->func_keys_script.active = 0;
	gui->func_keys_script.dynamic = 0;
	
	/* full path of func_keys file */
	gui->func_keys_path[0] = 0;
	snprintf(gui->func_keys_path, DXF_MAX_CHARS, "%sfunc_keys.lua", gui->pref_path);
	
	miss_file (gui->func_keys_path, (char*)func_key_dflt_file);
	
	if (gui_script_init (gui, &gui->func_keys_script, gui->func_keys_path, NULL) == 1){
		gui->func_keys_script.active = 1;
	}
	
	/*-------------------------------- Additional tools script --------------------- */
	
	/* full path of macro file */
	char plugins_path[DXF_MAX_CHARS + 1];
	plugins_path[0] = 0;
	snprintf(plugins_path, DXF_MAX_CHARS, "%splugins.lua", gui->pref_path);
	
	//miss_file (macro_path, (char*)macro_dflt_file);
	
	if (gui_script_init (gui, &gui->plugins_script, plugins_path, NULL) == 1){
		gui->plugins_script.active = 1;
		gui->plugins_script.time = clock();
		gui->plugins_script.timeout = 1.0; /* default timeout value */
		gui->plugins_script.do_init = 0;
		
		//print_lua_stack(gui->plugins_script.T);
		
		lua_getglobal(gui->plugins_script.T, "cz_main_func");
		int n_results = 0; /* for Lua 5.4*/
		gui->plugins_script.status = lua_resume(gui->plugins_script.T, NULL, 0, &n_results); /* start thread */
		if (gui->plugins_script.status != LUA_OK){
			gui->plugins_script.active = 0;
			
		}
	}
	
	/* try to open file passed in command line arg */
	if (strcmp(get_ext((char *) arg_file), "dxf") == 0){
		gui->action = FILE_OPEN;
		gui->path_ok = 1;
		strncpy(gui->curr_path, arg_file, PATH_MAX_CHARS);
		gui->hist_new = 1;
	}
	else if(gui->recent_drwg->next) { /* try to change working dir from last opened drawing */
		if (gui->recent_drwg->next->data){
			STRPOOL_U64 str_a = (STRPOOL_U64) gui->recent_drwg->next->data;
			dir_change(get_dir((char*)strpool_cstr( &gui->file_pool, str_a)));
			strncpy (gui->dwg_dir, get_dir((char*)strpool_cstr( &gui->file_pool, str_a)) , PATH_MAX_CHARS);
		}
	}
	/* ------------------------------------------------------------------------*/
	
	/* main loop */
	while (gui_main_loop (gui)){
    
    if (gui->low_proc){
      SDL_Delay(20);
      SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEMOTION);
    }
	}
	
	/* safe quit */
  
  /*-------------------------------- macro script clean-up--------------------- */
	if (gui->macro_script.L){
		lua_close(gui->macro_script.L);
	}
	
	/*-------------------------------- func_keys script clean-up--------------------- */
	if (gui->func_keys_script.L){
		lua_close(gui->func_keys_script.L);
	}
  
  if (gui->main_lang_scr.L){
		lua_close(gui->main_lang_scr.L);
	}
	
	/* Delete allocated resources */
	draw_gl_init ((void *)gui, 1);
	
	SDL_GL_DeleteContext(gui->gl_ctx.ctx);
	
	dxf_drawing_clear(gui->drawing);
	dxf_drawing_clear(gui->clip_drwg);
	
	SDL_DestroyWindow(gui->window);
	
	SDL_Quit();
	
  free_font_list(gui->font_list);
	
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
  for (i = 0; i < PRV_SIZE; i++){
    bmp_free(gui->preview[i]);
  }
	bmp_free(gui->i_cz48);
	bmp_free(gui->i_trash);
	
	i_svg_free_bmp(gui->svg_bmp);
	i_svg_free_curves(gui->svg_curves);
	
	gui_free_modal_cur(gui);
	
	
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
	manage_buffer(0, BUF_FREE, 0);
	manage_buffer(0, BUF_FREE, 1);
	manage_buffer(0, BUF_FREE, 2);
	manage_buffer(0, BUF_FREE, 3);
	
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
