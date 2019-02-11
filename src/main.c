#include <SDL.h>

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "dxf_graph.h"
#include "list.h"
#include "dxf_create.h"
#include "dxf_attract.h"

#include "dxf_colors.h"
#include "dxf_seed.h"
#include "i_svg_media.h"

#include "gui.h"
#include "gui_lay.h"
#include "gui_info.h"
#include "gui_xy.h"
#include "gui_use.h"
#include "gui_file.h"

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


void draw_cursor(bmp_img *img, int x, int y, bmp_color color){
	/* draw cursor */
	/* set the pattern */
	double pat = 1;
	patt_change(img, &pat, 1);
	/* set the color */
	img->frg = color;
	/* set the tickness */
	img->tick = 3;
	bmp_line(img, 0, y, img->width, y);
	bmp_line(img, x, 0, x, img->height);
	img->tick = 1;
	bmp_line(img, x-5, y+5, x+5, y+5);
	bmp_line(img, x-5, y-5, x+5, y-5);
	bmp_line(img, x+5, y-5, x+5, y+5);
	bmp_line(img, x-5, y-5, x-5, y+5);
}

void draw_attractor(bmp_img *img, enum attract_type type, int x, int y, bmp_color color){
	/* draw attractor mark*/
	if (type != ATRC_NONE){
		/* set the pattern */
		double pat = 1;
		patt_change(img, &pat, 1);
		/* set the color */
		img->frg = color;
		/* set the tickness */
		img->tick = 1;
	}
	switch (type){
		case ATRC_END:
			/* draw square */
			bmp_line(img, x-5, y-5, x-5, y+5);
			bmp_line(img, x-5, y+5, x+5, y+5);
			bmp_line(img, x+5, y+5, x+5, y-5);
			bmp_line(img, x+5, y-5, x-5, y-5);
			break;
		case ATRC_MID:
			/* draw triangle */
			bmp_line(img, x-5, y-5, x+5, y-5);
			bmp_line(img, x-5, y-5, x, y+5);
			bmp_line(img, x+5, y-5, x, y+5);
			break;
		case ATRC_CENTER:
			/* draw circle */
			bmp_circle(img, x, y, 7);
			break;
		case ATRC_OCENTER:
			/* draw a *star */
			bmp_line(img, x, y-5, x, y+5);
			bmp_line(img, x-5, y, x+5, y);
			bmp_line(img, x-5, y-5, x+5, y+5);
			bmp_line(img, x-5, y+5, x+5, y-5);
			break;
		case ATRC_NODE:
			/* draw circle with x*/
			bmp_circle(img, x, y, 5);
			bmp_line(img, x-5, y-5, x+5, y+5);
			bmp_line(img, x-5, y+5, x+5, y-5);
			break;
		case ATRC_QUAD:
			/* draw diamond */
			bmp_line(img, x-7, y, x, y+7);
			bmp_line(img, x, y+7, x+7, y);
			bmp_line(img, x+7, y, x, y-7);
			bmp_line(img, x, y-7, x-7, y);
			break;
		case ATRC_INTER:
			/* draw x */
			img->tick = 3;
			bmp_line(img, x-5, y-5, x+5, y+5);
			bmp_line(img, x-5, y+5, x+5, y-5);
			break;
		case ATRC_EXT:
			img->tick = 3;
			bmp_line(img, x-7, y, x-2, y);
			bmp_line(img, x, y, x+3, y);
			bmp_line(img, x+5, y, x+7, y);
			break;
		case ATRC_PERP:
			/* draw square */
			bmp_line(img, x-5, y-5, x-5, y+5);
			bmp_line(img, x-5, y-5, x+5, y-5);
			bmp_line(img, x-5, y, x, y);
			bmp_line(img, x, y-5, x, y);
			break;
		case ATRC_INS:
			bmp_line(img, x-5, y+5, x+1, y+5);
			bmp_line(img, x+1, y+5, x+1, y+1);
			bmp_line(img, x+1, y+1, x+5, y+1);
			bmp_line(img, x+5, y+1, x+5, y-5);
			bmp_line(img, x+5, y-5, x-1, y-5);
			bmp_line(img, x-1, y-5, x-1, y-1);
			bmp_line(img, x-1, y-1, x-5, y-1);
			bmp_line(img, x-5, y-1, x-5, y+5);
			break;
		case ATRC_TAN:
			bmp_line(img, x-5, y+5, x+5, y+5);
			bmp_circle(img, x, y, 5);
			break;
		case ATRC_PAR:
			/* draw two lines */
			bmp_line(img, x-5, y-1, x+1, y+5);
			bmp_line(img, x-1, y-5, x+5, y+1);
			break;
		case ATRC_CTRL:
			/* draw a cross */
			bmp_line(img, x, y-5, x, y+5);
			bmp_line(img, x-5, y, x+5, y);
			break;
		case ATRC_AINT:
			/* draw square with x */
			bmp_line(img, x-5, y-5, x-5, y+5);
			bmp_line(img, x-5, y+5, x+5, y+5);
			bmp_line(img, x+5, y+5, x+5, y-5);
			bmp_line(img, x+5, y-5, x-5, y-5);
			bmp_line(img, x-5, y-5, x+5, y+5);
			bmp_line(img, x-5, y+5, x+5, y-5);
			break;
		case ATRC_ANY:
			/* draw a Hourglass */
			bmp_line(img, x-5, y-5, x+5, y+5);
			bmp_line(img, x-5, y+5, x+5, y-5);
			img->tick = 2;
			bmp_line(img, x-5, y+5, x+5, y+5);
			bmp_line(img, x-5, y-5, x+5, y-5);
			break;
	}
}

void attrc_get_imgs(bmp_img *vec[], int num, int w, int h){
	
	bmp_color yellow = {.r = 255, .g = 255, .b =0, .a = 255};
	bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
	
	int i, attr = 1, x = w/2, y = h/2;
	for (i = 0; i < num; i++){
		vec[i] = bmp_new(w, h, transp, yellow);
		
		draw_attractor(vec[i], attr, x, y, yellow);
		if (vec[i]) vec[i]->zero_tl = 1;
		attr <<= 1;
	}
	
}

void zoom_ext(dxf_drawing *drawing, bmp_img *img, double *zoom, double *ofs_x, double *ofs_y){
	double min_x, min_y, max_x, max_y;
	double zoom_x, zoom_y;
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
	double min_x, min_y, max_x, max_y;
	double zoom_x, zoom_y;
	dxf_ents_ext(drawing, &min_x, &min_y, &max_x, &max_y);
	zoom_x = fabs(max_x - min_x)/width;
	zoom_y = fabs(max_y - min_y)/height;
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	*zoom = 1/(1.1 * (*zoom));
	
	*ofs_x = min_x - ((fabs((max_x - min_x)*(*zoom) - width)/2)+x)/(*zoom);
	*ofs_y = min_y - ((fabs((max_y - min_y)*(*zoom) - height)/2)+y)/(*zoom);
}

int getglobint (lua_State *L, const char *var) {
	int isnum, result;
	lua_getglobal(L, var);
	result = (int)lua_tointegerx(L, -1, &isnum);
	if (!isnum)
		printf("'%s' should be a number\n", var);
	lua_pop(L, 1); /* remove result from the stack */
	return result;
}
void load (lua_State *L, const char *fname, int *w, int *h) {
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
		printf("cannot run config. file: %s", lua_tostring(L, -1));
	*w = getglobint(L, "width");
	*h = getglobint(L, "height");
}

void load_conf (lua_State *L, const char *fname, gui_obj *gui) {
	/* load configuration file as Lua script*/
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
		printf("cannot run config. file: %s", lua_tostring(L, -1));
	
	
	
	/* -------------------- get screen width -------------------*/
	lua_getglobal(L, "width");
	if (lua_isnumber(L, -1)) gui->win_w = (int)lua_tonumber(L, -1);
	else gui->win_w = 1200; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get screen height -------------------*/
	lua_getglobal(L, "height");
	if (lua_isnumber(L, -1)) gui->win_h = (int)lua_tonumber(L, -1);
	else gui->win_h = 710; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get fonts paths -------------------*/
	lua_getglobal(L, "font_path");
	if (lua_isstring(L, -1)){
		const char *font_path = lua_tostring(L, -1);
		strncat(gui->dflt_fonts_path, font_path, 5 * DXF_MAX_CHARS);
		
	}
	else{ /* default value, if not definied in file*/
		#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
		strncpy(gui->dflt_fonts_path, "C:\\Windows\\Fonts\\", 5 * DXF_MAX_CHARS);
		#else
		strncat(gui->dflt_fonts_path, "/usr/share/fonts/", 5 * DXF_MAX_CHARS);
		#endif
	}
	lua_pop(L, 1);
	
	/* -------------------- load list of extra fonts  -------------------*/
	lua_getglobal(L, "fonts");
	if (lua_istable(L, -1)){
		
		/* iterate over table */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) { /* table index are shifted*/
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			//printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
			if (lua_isstring(L, -1)){
				//printf("%s\n", lua_tostring(L, -1));
				add_font_list(gui->font_list, (char *)lua_tostring(L, -1), gui->dflt_fonts_path);
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	
	/* -------------------- get font for use in GUI  -------------------*/
	lua_getglobal(L, "ui_font");
	if (lua_istable(L, -1)){
		/*---- name of font */
		lua_pushnumber(L, 1); /* key*/
		lua_gettable(L, -2);  /* get table[key] */
		if (lua_isstring(L, -1)){
			//printf("%s\n", lua_tostring(L, -1));
			struct tfont *ui_font = get_font_list(gui->font_list, (char *)lua_tostring(L, -1));
			if (!ui_font) { /* default font, if fail to find in list*/
				ui_font = get_font_list(gui->font_list, "romans.shx");
			}
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		else{ /* default value, if not definied in file*/
			struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		lua_pop(L, 1);
		
		/*---- size of text */
		lua_pushnumber(L, 2); /* key*/
		lua_gettable(L, -2);  /* get table[key] */
		if (lua_isnumber(L, -1)){
			//printf("%0.2f\n", lua_tonumber(L, -1));
			gui->ui_font.height = lua_tonumber(L, -1);
		}
		else{/* default value, if not definied in file*/
			gui->ui_font.height = 10.0;
		}
		lua_pop(L, 1);
		
		#if(0)
		/* iterate over table */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) { /* table index are shifted*/
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
		#endif
	}
	else
	{ /* default font, if not definied in file*/
		struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
		gui->ui_font.userdata = nk_handle_ptr(ui_font);
		gui->ui_font.height = 10.0;
	}
	lua_pop(L, 1);
}

int main(int argc, char** argv){
	gui_obj *gui = malloc(sizeof(gui_obj));
	gui_start(gui);
	/* initialize base directory (executable dir) */
	strncpy (gui->base_dir, get_dir(argv[0]), DXF_MAX_CHARS);
	/* initialize fonts paths with base directory  */
	if (strlen(gui->base_dir)){
		strncpy (gui->dflt_fonts_path, gui->base_dir, 5 * DXF_MAX_CHARS);
		strncat (gui->dflt_fonts_path, (char []){PATH_SEPARATOR, 0}, 5 * DXF_MAX_CHARS);
	}
	lua_State *Lua1 = luaL_newstate(); /* opens Lua */
	luaL_openlibs(Lua1); /* opens the standard libraries */
	
	//setlocale(LC_ALL,""); //seta a localidade como a current do computador para aceitar acentuacao
	int i, ok;
	
	//load (Lua1, "config.lua", &gui->win_w, &gui->win_h);
	load_conf (Lua1, "config.lua", gui);
	
	SDL_Rect win_r, display_r;
	
	/* init the SDL2 */
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window * window = SDL_CreateWindow(
		"CadZinho", /* title */
		SDL_WINDOWPOS_UNDEFINED, /* x position */
		SDL_WINDOWPOS_UNDEFINED, /* y position */
		gui->win_w, /* width */
		gui->win_h, /* height */
		SDL_WINDOW_RESIZABLE); /* flags */
		
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	
	SDL_RendererInfo rend_info;
	
	SDL_GetRendererInfo(renderer, &rend_info);
	
	gui->main_w = rend_info.max_texture_width;
	gui->main_h = rend_info.max_texture_height;
	
	if ((gui->main_w <= 0) || (gui->main_h <= 0)){
		gui->main_w = 2048;
		gui->main_h = 2048;
	}
	
	SDL_Texture * canvas = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC, 
		gui->main_w, /* width */
		gui->main_h); /* height */
		
	//SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
	
	char *base_path = SDL_GetBasePath();
	printf ("%s\n", base_path);
	printf ("%s\n", argv[0]);
	printf ("%s\n", get_dir(argv[0]));
	printf ("%s\n", gui->dflt_fonts_path);
	
	#if(0)
	char fonts_path[DXF_MAX_CHARS];
	fonts_path[0] = 0;
	
	char *base_path = SDL_GetBasePath();
	
	strncpy(fonts_path, base_path, DXF_MAX_CHARS);
	strncat(fonts_path, "fonts/", DXF_MAX_CHARS - strlen(fonts_path));
	
	#ifdef OS_WIN
	char *sys_fonts_path = "C:\\Windows\\Fonts\\";
	#elif defined(OS_LINUX)
	char *sys_fonts_path = "/usr/share/fonts/";
	#else
	char *sys_fonts_path = NULL;
	#endif
	
	printf(fonts_path);
	printf(sys_fonts_path);
	#endif
	
	double pos_x, pos_y;

	int open_prg = 0;
	int progress = 0;
	int progr_win = 0;
	int progr_end = 0;
	unsigned int quit = 0;
	unsigned int wait_open = 0;
	int show_app_about = 0;
	int show_app_file = 0;
	int path_ok = 0;
	int show_info = 0;
	
	
	char file_path[DXF_MAX_CHARS];
	int file_path_len = 0;
	
	int show_lay_mng = 0, sel_tmp = 0, show_color_pick = 0, show_lay_name = 0;
	int show_tstyles_mng = 0;
	
	int ev_type;
	struct nk_color background;
	
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int leftMouseButtonClick = 0;
	int rightMouseButtonClick = 0;
	int MouseMotion = 0;
	
	int en_attr = 1;
	
	SDL_Event event;
	int low_proc = 1;
		
	char recv_comm[64];
	int recv_comm_flag = 0;
	
	char *url = NULL;
	//char const * lFilterPatterns[2] = { "*.dxf", "*.txt" };
	
	char *file_buf = NULL;
	long file_size = 0;
	
	char* dropped_filedir;                  /* Pointer for directory of dropped file */
	
	
	gui->show_file_br = 0;
	
	//gui->file_filter_types[0] = ext_types[FILE_DXF];
	//gui->file_filter_types[1] = ext_types[FILE_ALL];
	
	//gui->file_filter_descr[0] = ext_descr[FILE_DXF];
	//gui->file_filter_descr[1] = ext_descr[FILE_ALL];
	
	gui->file_filter_count = 0;
	
	gui->curr_path[0] = 0;
	
	/* Colors in use */
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	bmp_color green = {.r = 0, .g = 255, .b =0, .a = 255};
	bmp_color yellow = {.r = 255, .g = 255, .b =0, .a = 255};
	bmp_color grey = {.r = 100, .g = 100, .b = 100, .a = 255};
	bmp_color hilite = {.r = 255, .g = 0, .b = 255, .a = 150};
	bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
	bmp_color cursor = {.r = 255, .g = 255, .b = 255, .a = 100};
	background = nk_rgb(28,48,62);
	
	/* line types in use */
	double center [] = {12, -6, 2 , -6};
	double dash [] = {8, -8};
	double continuous[] = {1};
	
	/* initialize the selection list */
	gui->sel_list = list_new(NULL, 0);
	
	/* initialize the undo/redo list */
	//struct do_list gui->list_do;
	init_do_list(&gui->list_do);
	
	
	/* init the main image */
	bmp_img * img = bmp_new(gui->main_w, gui->main_h, grey, red);
	
	/* init Nuklear GUI */
	
	nk_sdl_init(gui);
	
	set_style(gui->ctx, THEME_ZE);
	
	gui->ctx->style.edit.padding = nk_vec2(4, -6);
	
	/* init the toolbox image */
	#define ICON_SIZE 24
	
	gui->svg_curves = i_svg_all_curves();
	gui->svg_bmp = i_svg_all_bmp(gui->svg_curves, ICON_SIZE, ICON_SIZE);
	
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
	gui->svg_bmp[SVG_CZ] = i_svg_bmp(gui->svg_curves[SVG_CZ], 16, 16);
	
	bmp_img *i_cz48 = i_svg_bmp(gui->svg_curves[SVG_CZ], 48, 48);
	
	bmp_img * attr_vec[15];
	attrc_get_imgs(attr_vec, 15, 16, 16);
	
	//struct nk_style_button b_icon_style;
	if (gui){
		gui->b_icon = gui->ctx->style.button;
	}
	gui->b_icon.image_padding.x = -4;
	gui->b_icon.image_padding.y = -4;
	
	/* style for toggle buttons (or select buttons) with image */
	//struct nk_style_button gui->b_icon_sel, gui->b_icon_unsel;
	if (gui){
		gui->b_icon_sel = gui->ctx->style.button;
		gui->b_icon_unsel = gui->ctx->style.button;
	}
	gui->b_icon_unsel.normal = nk_style_item_color(nk_rgba(58, 67, 57, 255));
	gui->b_icon_unsel.hover = nk_style_item_color(nk_rgba(73, 84, 72, 255));
	//gui->b_icon_unsel.active = nk_style_item_color(nk_rgba(81, 92, 80, 255));
	gui->b_icon_sel.image_padding.x = -4;
	gui->b_icon_sel.image_padding.y = -4;
	gui->b_icon_unsel.image_padding.x = -4;
	gui->b_icon_unsel.image_padding.y = -4;
	
	bmp_img * color_img = bmp_new(15, 13, black, red);
	struct nk_image i_color = nk_image_ptr(color_img);
	
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
	
	
	/* init comands */
	recv_comm[0] = 0;
	
	
	/* init the drawing */
	gui->drawing = malloc(sizeof(dxf_drawing));
	gui->drawing->font_list = gui->font_list;
	gui->drawing->dflt_font = gui->dflt_font;
	
	url = NULL; /* pass a null file only for initialize the drawing structure */
	//dxf_open(gui->drawing, url);
	while (dxf_read (gui->drawing, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &progress) > 0){
		
	}
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

	
	/*
	SDL_RendererInfo info;
	if (SDL_GetRenderDriverInfo(0, &info) == 0){
		//printf("Driver num pix format = %d\n", info.num_texture_formats);
		for (i = 0; i < info.num_texture_formats; i++){
			switch (info.texture_formats[i]){
				case SDL_PIXELFORMAT_ARGB8888:
					//printf("pix format ARGB8888\n");
					//img->r_i = 2;
					//img->g_i = 1;
					//img->b_i = 0;
					//img->a_i = 3;
					break;
				case SDL_PIXELFORMAT_RGBA8888:
					//printf("pix format RGBA8888\n");
					break;
				case SDL_PIXELFORMAT_ABGR8888:
					//printf("pix format ABGR8888\n");
					break;
				case SDL_PIXELFORMAT_BGRA8888:
					//printf("pix format BGRA8888\n");
					break;
			}
		}
	}
	*/

		
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	
	
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
				
				switch (event.type){
					case SDL_MOUSEBUTTONUP:
						gui->mouse_x = event.button.x;
						gui->mouse_y = event.button.y;
						gui->mouse_y = gui->main_h - gui->mouse_y;
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
						gui->mouse_y = gui->main_h - gui->mouse_y;
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
						gui->mouse_y = gui->main_h - gui->mouse_y;
						pos_x = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
						pos_y = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
						gui->draw = 1;
						break;
					case SDL_MOUSEWHEEL:
						gui->prev_zoom = gui->zoom;
						gui->zoom = gui->zoom + event.wheel.y * 0.2 * gui->zoom;
						
						SDL_GetMouseState(&gui->mouse_x, &gui->mouse_y);
						gui->mouse_y = gui->main_h - gui->mouse_y;
						gui->ofs_x += ((double) gui->mouse_x)*(1/gui->prev_zoom - 1/gui->zoom);
						gui->ofs_y += ((double) gui->mouse_y)*(1/gui->prev_zoom - 1/gui->zoom);
						gui->draw = 1;
						break;
					
					case (SDL_DROPFILE): {      // In case if dropped file
						//dropped_filedir = event.drop.file;
						// Shows directory of dropped file
						//SDL_ShowSimpleMessageBox(
						//	SDL_MESSAGEBOX_INFORMATION,
						//	"File dropped on window",
						//	dropped_filedir,
						//	window);
						//printf(dropped_filedir);
						//printf("\n----------------------------\n");
						//SDL_free(dropped_filedir);    // Free dropped_filedir memory
						}
						break;
					case SDL_KEYDOWN:
						if ((event.key.keysym.sym == SDLK_RETURN) || (event.key.keysym.sym == SDLK_RETURN2)){
							gui->keyEnter = 1;
						}
						else if (event.key.keysym.sym == SDLK_UP){
							gui->action = VIEW_PAN_U;
						}
						else if (event.key.keysym.sym == SDLK_DOWN){
							gui->action = VIEW_PAN_D;
						}
						else if (event.key.keysym.sym == SDLK_LEFT){
							gui->action = VIEW_PAN_L;
						}
						else if (event.key.keysym.sym == SDLK_RIGHT){
							gui->action = VIEW_PAN_R;
						}
						else if (event.key.keysym.sym == SDLK_KP_MINUS){
							gui->action = VIEW_ZOOM_M;
						}
						else if (event.key.keysym.sym == SDLK_KP_PLUS){
							gui->action = VIEW_ZOOM_P;
						}
						break;
					case SDL_TEXTINPUT:
						/* text input */
						/* if the user enters a character relative a number */
						if ((*event.text.text > 41) && (*event.text.text < 58)){
							gui->user_number = 1; /* sinalize a user flag */
						}
						break;
					case SDL_WINDOWEVENT:
						if (event.window.event == SDL_WINDOWEVENT_RESIZED){
							
						}
						if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
							
						}
				}
			}
		}
		
		gui->next_win_h = 6 + 4 + ICON_SIZE + 4 + 6 + 4 + ICON_SIZE + 4 + 6 + 8;
		gui->next_win_x = 2;
		gui->next_win_y = 2;
		
		if (nk_begin(gui->ctx, "Main", nk_rect(gui->next_win_x, gui->next_win_y, gui->win_w - 4, gui->next_win_h),
		NK_WINDOW_BORDER)){
			/* first line */
			nk_layout_row_begin(gui->ctx, NK_STATIC, ICON_SIZE + 12, 8);
			
			/* file tools*/
			nk_layout_row_push(gui->ctx, 6*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "file", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NEW]))){
					printf("NEW\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_OPEN]))){
					gui->action = FILE_OPEN;
					show_app_file = 1;
					
					gui->curr_path[0] = 0;
					
					path_ok = 0;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SAVE]))){
					gui->action = FILE_SAVE;
					show_app_file = 1;
					path_ok = 0;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPORT]))){
					gui->action = EXPORT;
					show_app_file = 1;
					path_ok = 0;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CLOSE]))){
					printf("CLOSE\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PRINT]))){
					printf("PRINT\n");
				}
				
				nk_group_end(gui->ctx);
			}
			
			/* clipboard tools*/
			nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "clipboard", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				
				
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_COPY]))){
					printf("Copy\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CUT]))){
					printf("Cut\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PASTE]))){
					printf("Paste\n");
				}
				
				nk_group_end(gui->ctx);
			}
			
			/* undo/redo tools*/
			nk_layout_row_push(gui->ctx, 2*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "undo-redo", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UNDO]))){
					gui->action = UNDO;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_REDO]))){
					gui->action = REDO;
				}
				nk_group_end(gui->ctx);
			}
			
			/* managers*/
			nk_layout_row_push(gui->ctx, 5*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "managers", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LAYERS]))){
					//printf("Layers\n");
					show_lay_mng = 1;
					//sel_tmp = -1;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FONT]))){
					//printf("FONT\n");
					show_tstyles_mng = 1;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LTYPE]))){
					printf("Line types\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PUZZLE]))){
					printf("Blocks\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAGS]))){
					printf("APPID\n");
				}
				nk_group_end(gui->ctx);
			}
			
			/* zoom tools*/
			nk_layout_row_push(gui->ctx, 4*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "zoom", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_P]))){
					gui->action = VIEW_ZOOM_P;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_M]))){
					gui->action = VIEW_ZOOM_M;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_W]))){
					gui->action = VIEW_ZOOM_W;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ZOOM_A]))){
					gui->action = VIEW_ZOOM_EXT;
				}
				nk_group_end(gui->ctx);
			}
			
			/* pan tools*/
			nk_layout_row_push(gui->ctx, 4*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "pan", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_UP]))){
					gui->action = VIEW_PAN_U;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DOWN]))){
					gui->action = VIEW_PAN_D;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LEFT]))){
					gui->action = VIEW_PAN_L;
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RIGTH]))){
					gui->action = VIEW_PAN_R;
				}
				nk_group_end(gui->ctx);
			}
			
			/* config tools*/
			nk_layout_row_push(gui->ctx, 3*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "config", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_INFO]))){
					show_info = 1;
					//printf("Info\n");
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TOOL]))){
					//printf("Tools\n");
					#ifdef OS_WIN
					DebugBreak();
					#endif
				}
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_GEAR]))){
					//printf("Config\n");
					//gui->file_filter_types[0] = ext_types[FILE_ALL];
					//gui->file_filter_descr[0] = ext_descr[FILE_ALL];
					
					//gui->file_filter_count = 1;
					
					//gui->show_file_br = 1;
					
					show_app_file = 1;
				}
				nk_group_end(gui->ctx);
			}
			
			/* config tools*/
			nk_layout_row_push(gui->ctx, 1*(ICON_SIZE + 4 + 4) + 13);
			if (nk_group_begin(gui->ctx, "help", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_static(gui->ctx, ICON_SIZE + 4, ICON_SIZE + 4, 10);
				if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HELP]))){
					show_app_about = 1;
					//printf("HELP\n");
				}
				nk_group_end(gui->ctx);
			}
			
			nk_layout_row_end(gui->ctx);
			/*------------ end first line --------------*/
			
			/* second line */
			nk_layout_row_begin(gui->ctx, NK_STATIC, ICON_SIZE + 4, ICON_SIZE + 4);
			
			static char text[64];
			int text_len;
			nk_layout_row_push(gui->ctx, 1000);
			if (nk_group_begin(gui->ctx, "Prop", NK_WINDOW_NO_SCROLLBAR)) {
				nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 20);
				
				/*layer*/
				nk_layout_row_push(gui->ctx, 60);
				nk_label(gui->ctx, "Layer: ", NK_TEXT_RIGHT);
				nk_layout_row_push(gui->ctx, 200);
				layer_prop(gui);
				
				
				/*color picker */
				int c_idx = gui->color_idx;
				if (c_idx >255){
					c_idx = gui->drawing->layers[gui->layer_idx].color;
					if (c_idx >255){
						c_idx = 0;
					}
				}
				/* fill the tiny bitmap with selected color*/
				bmp_fill(color_img, dxf_colors[c_idx]);
				
				/* print the name (number) of color */
				if (gui->color_idx == 0){
					text_len = snprintf(text, 63, "%s", "ByB");
				}
				else if (gui->color_idx < 256){
					text_len = snprintf(text, 63, "%d", gui->color_idx);
				}
				else{
					text_len = snprintf(text, 63, "%s", "ByL");
				}
				nk_layout_row_push(gui->ctx, 70);
				nk_label(gui->ctx, "Color: ", NK_TEXT_RIGHT);
				nk_layout_row_push(gui->ctx, 70);
				if (nk_combo_begin_image_label(gui->ctx, text, i_color, nk_vec2(215,320))){
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					if (nk_button_label(gui->ctx, "By Layer")){
						gui->color_idx = 256;
						gui->action = COLOR_CHANGE;
						nk_combo_close(gui->ctx);
					}
					if (nk_button_label(gui->ctx, "By Block")){
						gui->color_idx = 0;
						gui->action = COLOR_CHANGE;
						nk_combo_close(gui->ctx);
					}
					nk_layout_row_static(gui->ctx, 15, 15, 10);
					nk_label(gui->ctx, " ", NK_TEXT_RIGHT); /* for padding color alingment */
					
					for (i = 1; i < 256; i++){
						struct nk_color b_color = {
							.r = dxf_colors[i].r,
							.g = dxf_colors[i].g,
							.b = dxf_colors[i].b,
							.a = dxf_colors[i].a
						};
						if(nk_button_color(gui->ctx, b_color)){
							gui->color_idx = i;
							gui->action = COLOR_CHANGE;
							nk_combo_close(gui->ctx);
						}
					}
					
					nk_combo_end(gui->ctx);
				}
				
				/*line type*/
				nk_layout_row_push(gui->ctx, 100);
				nk_label(gui->ctx, "Line type: ", NK_TEXT_RIGHT);
				nk_layout_row_push(gui->ctx, 200);
				if (nk_combo_begin_label(gui->ctx, gui->drawing->ltypes[gui->ltypes_idx].name, nk_vec2(300,200))){
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					int num_ltypes = gui->drawing->num_ltypes;
					
					for (i = 0; i < num_ltypes; i++){
						
						if (nk_button_label(gui->ctx, gui->drawing->ltypes[i].name)){
							gui->ltypes_idx = i;
							gui->action = LTYPE_CHANGE;
							nk_combo_close(gui->ctx);
						}
						nk_label(gui->ctx, gui->drawing->ltypes[i].descr, NK_TEXT_LEFT);
					}
					
					nk_combo_end(gui->ctx);
				}
				
				/* thickness 
				nk_layout_row_push(gui->ctx, 150);
				//nk_property_float(struct nk_context*, const char *name, float min, float *val, float max, float step, float inc_per_pixel);
				//nk_property_float(gui->ctx, "Thick:", 0.0, &thick, 20.0, 0.1, 0.1);
				
				//double nk_propertyd(struct nk_context*, const char *name, double min, double val, double max, double step, float inc_per_pixel);
				thick = nk_propertyd(gui->ctx, "Thickness", 0.0d, thick_prev, 20.0d, 0.1d, 0.1d);
				if (thick_prev != thick){
					gui->action = THICK_CHANGE;
					//printf ("thick change\n");
				}
				thick_prev = thick;*/
				
				nk_layout_row_push(gui->ctx, 120);
				nk_label(gui->ctx, "Line weight: ", NK_TEXT_RIGHT);
				nk_layout_row_push(gui->ctx, 120);
				
				if (nk_combo_begin_label(gui->ctx, dxf_lw_descr[gui->lw_idx], nk_vec2(200,300))){
					nk_layout_row_dynamic(gui->ctx, 25, 2);
					if (nk_button_label(gui->ctx, "By Layer")){
						gui->lw_idx = DXF_LW_LEN;
						gui->action = LW_CHANGE;
						nk_combo_close(gui->ctx);
					}
					if (nk_button_label(gui->ctx, "By Block")){
						gui->lw_idx = DXF_LW_LEN + 1;
						gui->action = LW_CHANGE;
						nk_combo_close(gui->ctx);
					}
					nk_layout_row_dynamic(gui->ctx, 17, 1);
					for (i = 0; i < DXF_LW_LEN; i++){
						if (nk_button_label(gui->ctx, dxf_lw_descr[i])){
							gui->lw_idx = i;
							gui->action = LW_CHANGE;
							nk_combo_close(gui->ctx);
						}
					}
					
					nk_combo_end(gui->ctx);
				}
				
				nk_layout_row_end(gui->ctx);
				
				nk_group_end(gui->ctx);
			}
			
			nk_layout_row_end(gui->ctx);
			/*------------ end second line --------------*/
			
			if (show_app_about){
				/* file open popup */
				static struct nk_rect s = {20, 100, 400, 150};
				if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "About", NK_WINDOW_CLOSABLE, s)){
					nk_layout_row_dynamic(gui->ctx, 50, 2);
					nk_label(gui->ctx, "CadZinho", NK_TEXT_RIGHT);
					nk_image(gui->ctx, nk_image_ptr(i_cz48));
					//nk_layout_row_dynamic(gui->ctx, 165, 1);
					//nk_image(gui->ctx, i_cz);
					nk_layout_row_begin(gui->ctx, NK_DYNAMIC, 20, 2);
					nk_layout_row_push(gui->ctx, 0.7f);
					nk_label(gui->ctx, "By Ezequiel Rabelo de Aguiar", NK_TEXT_RIGHT);
					nk_layout_row_push(gui->ctx, 0.3f);
					nk_image(gui->ctx, nk_image_ptr(gui->svg_bmp[SVG_BRAZIL]));
					nk_layout_row_end(gui->ctx);
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "CadZinho is licensed under the MIT License.",  NK_TEXT_LEFT);
					nk_popup_end(gui->ctx);
				} else show_app_about = nk_false;
			}
			
			if (show_app_file){
				enum files_types filters[2] = {FILE_DXF, FILE_ALL};
				char path[20];
				show_app_file = file_pop (gui, filters, 2, NULL);
				if (show_app_file == 2){
						//if (strlen(file_path) > 4){
							path_ok = 1;
						//}
					
					show_app_file = 0;
				}
			}
		}
		nk_end(gui->ctx);
		
		
		
		
		
		
		
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
		
		if (nk_begin(gui->ctx, "Toolbox", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)){
			
			//if (nk_tree_push(gui->ctx, NK_TREE_TAB, "Place", NK_MAXIMIZED)) {
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, "Modify:", NK_TEXT_LEFT);
			
			nk_layout_row_static(gui->ctx, 28, 28, 6);
			
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CURSOR]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","SELECT");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT_E]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MOVE]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","MOVE");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_DUPLI]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","DUPLI");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SCALE]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","SCALE");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ROT]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_MIRROR]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BLOCK]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","NEW_BLK");
				/*dxf_new_block(gui->drawing, "teste", "0", gui->sel_list, &gui->list_do);
				dxf_ent_print2(gui->drawing->blks);
				dxf_ent_print2(gui->drawing->blks_rec);*/
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EXPLODE]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_EDIT]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TAG_E]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_FIND]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RULER]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_STYLE]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TRASH]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","DELETE");
			}
				
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, "Place:", NK_TEXT_LEFT);
				
			nk_layout_row_static(gui->ctx, 28, 28, 6);
			
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_LINE]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","LINE");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PLINE]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","POLYLINE");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_RECT]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","RECT");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_TEXT]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","TEXT");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_CIRCLE]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","CIRCLE");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ELIPSE]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_ARC]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_SPLINE]))){
				
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_HATCH]))){
				gui->modal = HATCH;
				gui->step = 0;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_I_TEXT]))){
				gui->modal = MTEXT;
				gui->step = 0;
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_BOOK]))){
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s","INSERT");
			}
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_IMAGE]))){
				
			}
			
			nk_layout_row_dynamic(gui->ctx, 20, 1); /*space*/
			
			struct nk_vec2 wid_pos = nk_widget_position(gui->ctx);
			struct nk_vec2 win_pos = nk_window_get_position(gui->ctx);
			struct nk_rect win_cont = nk_window_get_content_region(gui->ctx);
		
			nk_layout_row_dynamic(gui->ctx, win_cont.h - (wid_pos.y - win_pos.y), 1);
			if (nk_group_begin(gui->ctx, "especific", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				gui_select_info (gui);
				gui_line_info (gui);
				gui_pline_info (gui);
				gui_circle_info (gui);
				gui_rect_info (gui);
				gui_text_info (gui);
				gui_mtext_info (gui);
				gui_move_info (gui);
				gui_dupli_info (gui);
				gui_scale_info (gui);
				gui_insert_info (gui);
				gui_block_info (gui);
				gui_hatch_info (gui);
				
				nk_group_end(gui->ctx);
			}
		}
		nk_end(gui->ctx);
		
/* ==============================================================
======    LAYER MANAGER   ==========================================
================================================================*/
		
		if (show_lay_mng){
			show_lay_mng = lay_mng (gui);
		}
		
/* ==============================================================
======    END LAYER MANAGER   ======================================
================================================================*/

		
		if (show_info){
			show_info = info_win(gui);
		}
		
		if (gui->show_file_br){			
			gui->show_file_br = file_win(gui, gui->file_filter_types, gui->file_filter_descr, gui->file_filter_count, NULL);
			strncpy(file_path, gui->curr_path, DXF_MAX_CHARS);
			file_path_len = strlen(file_path);
		}
		
		if (show_tstyles_mng){
			show_tstyles_mng = tstyles_mng (gui);
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
				nk_progress(gui->ctx, (nk_size *)&progress, 100, NK_FIXED);
				//nk_popup_end(gui->ctx);
				nk_end(gui->ctx);
			}
			
			if (progr_end){
				progr_win = 0;
				progr_end = 0;
				nk_window_close(gui->ctx, "Progress");
			}
		}
		
		/* interface to the user visualize and enter coordinates distances*/
		if (nk_begin(gui->ctx, "POS", nk_rect(2, gui->win_h - 80, gui->win_w - 4, 78),
		NK_WINDOW_BORDER))
		{
			/* interface to the user visualize and enter coordinates and distances*/
			gui_xy(gui);
			
			
			/*----------- attractors --------------*/
			//nk_layout_row_push(gui->ctx, 160);
			//nk_label(gui->ctx, "Attractors ->", NK_TEXT_RIGHT);
			
			/* Toggle on/off attractors*/
			nk_layout_row_push(gui->ctx, 30);
			/*if (en_attr){
				nk_selectable_label(gui->ctx, "On", NK_TEXT_CENTERED, &en_attr);
			}
			else nk_selectable_label(gui->ctx, "Off", NK_TEXT_CENTERED, &en_attr);*/
			if (en_attr){
				if (nk_button_image_styled(gui->ctx, &gui->b_icon_sel, nk_image_ptr(gui->svg_bmp[SVG_MAGNET]))){
					en_attr = 0;
				}
			}else {
				if (nk_button_image_styled(gui->ctx, &gui->b_icon_unsel, nk_image_ptr(gui->svg_bmp[SVG_MAGNET]))){
					en_attr = 1;
				}
			}
			nk_layout_row_push(gui->ctx, 16*(22 + 3) + 13);
			if (nk_group_begin(gui->ctx, "attractors", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
				/* Buttons to select attractor mode*/
				int selected, i, attr = 1;
				nk_layout_row_static(gui->ctx, 22, 22, 15);
				for (i = 0; i < 15; i++){
					selected = (gui->curr_attr_t & attr);
					/* uses styles "sel" or "unsel", deppending each status*/
					//nk_layout_row_push(gui->ctx, 22);
					if (selected){
						if (nk_button_image_styled(gui->ctx, &gui->b_icon_sel, nk_image_ptr(attr_vec[i]))){
							gui->curr_attr_t &= ~attr; /* clear bit of current type*/
						}
					}else {
						if (nk_button_image_styled(gui->ctx, &gui->b_icon_unsel, nk_image_ptr(attr_vec[i]))){
							gui->curr_attr_t |= attr; /* set bit of current type*/
						}
					}
					attr <<= 1; /* next attractor type (bit coded)*/
				}
				/*-------------------------------*/
				nk_group_end(gui->ctx);
			}
			
			nk_layout_row_push(gui->ctx, ICON_SIZE + 4);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_PREV]))){
				
			}
			nk_layout_row_push(gui->ctx, ICON_SIZE + 4);
			if (nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(gui->svg_bmp[SVG_NEXT]))){
				
			}
			nk_layout_row_end(gui->ctx);
			
			nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 20);
			
			int text_len;
			char text[64];
			
			nk_style_push_font(gui->ctx, &(gui->alt_font_sizes[FONT_SMALL])); /* change font to tiny*/
			
			/* view coordinates of mouse in drawing units */
			nk_layout_row_push(gui->ctx, 280);
			text_len = snprintf(text, 63, "(%f,  %f)", pos_x, pos_y);
			nk_label(gui->ctx, text, NK_TEXT_CENTERED);
			
			nk_layout_row_push(gui->ctx, 280);
			nk_label(gui->ctx, gui->log_msg, NK_TEXT_LEFT);
			
			nk_style_pop_font(gui->ctx); /* return to the default font*/
			
			/*-------------------------------*/
			nk_layout_row_end(gui->ctx);
		}
		nk_end(gui->ctx);
		
		/* */
		
		
		if (!(nk_window_is_any_hovered(gui->ctx)) && (gui->modal != SELECT)){
			nk_window_set_focus(gui->ctx, "POS");
		}
		
		/*================================================
		==================================================
		=================================================*/
		
		if (leftMouseButtonClick) gui->ev |= EV_ENTER;
		if (rightMouseButtonClick) gui->ev |= EV_CANCEL;
		if (MouseMotion) gui->ev |= EV_MOTION;
		if (gui->keyEnter) gui->ev |= EV_LOCK_AX;
		
		if (wait_open != 0){
			low_proc = 0;
			gui->draw = 1;
			
			open_prg = dxf_read(gui->drawing, file_buf, file_size, &progress);
			
			if(open_prg <= 0){
				free(file_buf);
				file_buf = NULL;
				file_size = 0;
				low_proc = 1;
				
				//dxf_ent_print2(gui->drawing->blks);
				gui_tstyle(gui);
				dxf_ents_parse(gui->drawing);				
				gui->action = VIEW_ZOOM_EXT;
				gui->layer_idx = 0;
				gui->ltypes_idx = 0;
				gui->t_sty_idx = 0;
				gui->color_idx = 256;
				wait_open = 0;
				progr_end = 1;
				list_clear(gui->sel_list);
			}
			
		}
		else low_proc = 1;
		
		
		/*===============================*/
		
		if((gui->action == FILE_OPEN) && (path_ok)) {
			gui->action = NONE; path_ok = 0;
			
			dxf_mem_pool(ZERO_DXF, 0);
			graph_mem_pool(ZERO_GRAPH, 0);
			graph_mem_pool(ZERO_LINE, 0);
			
			wait_open = 1;
			progress = 0;
			
			file_buf = dxf_load_file(gui->curr_path, &file_size);
			open_prg = dxf_read(gui->drawing, file_buf, file_size, &progress);
			
			low_proc = 0;
			progr_win = 1;
			
			SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			
			strncpy (gui->dwg_dir, get_dir(gui->curr_path) , DXF_MAX_CHARS);
			strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
			
			char title[DXF_MAX_CHARS] = "CadZinho - ";
			strncat (title, gui->dwg_file, DXF_MAX_CHARS);
			SDL_SetWindowTitle(window, title);
		}
		else if((gui->action == FILE_SAVE) && (path_ok)){
			gui->action = NONE; path_ok = 0;
			
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
		else if((gui->action == EXPORT) && (path_ok)) {
			gui->action = NONE; path_ok = 0;
			
			if (gui->drawing->main_struct != NULL){
				dxf_ent_print_f (gui->drawing->main_struct, gui->curr_path);
				//dxf_save (url, gui->drawing);
			}
		}
		else if(gui->action == VIEW_ZOOM_EXT){
			gui->action = NONE;
			zoom_ext2(gui->drawing, 0, gui->main_h - gui->win_h, gui->win_w, gui->win_h, &gui->zoom, &gui->ofs_x, &gui->ofs_y);
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
		else if(gui->action == DELETE){
			gui->action = NONE;
		
			if (gui->sel_list != NULL){
				
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
				list_clear(gui->sel_list);
			}
			gui->draw = 1;
		}
		else if(gui->action == UNDO){
			gui->action = NONE;
			list_clear(gui->sel_list);
			char *text = gui->list_do.current->text;
			
			if (do_undo(&gui->list_do)){
				snprintf(gui->log_msg, 63, "UNDO: %s", text);
				goto first_step;
			}
			else{
				snprintf(gui->log_msg, 63, "No actions to undo");
			}
			gui->draw = 1;
		}
		
		else if(gui->action == REDO){
			gui->action = NONE;
			list_clear(gui->sel_list);
			if (do_redo(&gui->list_do)){
				snprintf(gui->log_msg, 63, "REDO: %s", gui->list_do.current->text);
				goto first_step;
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
		
		if(recv_comm_flag){
			recv_comm_flag =0;
			str_upp(recv_comm);
			if (strcmp(recv_comm, "SELECT") == 0){
				gui->modal = SELECT;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "LINE") == 0){
				gui->modal = LINE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "POLYLINE") == 0){
				gui->modal = POLYLINE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "DELETE") == 0){
				gui->action = DELETE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "CIRCLE") == 0){
				gui->modal = CIRCLE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "RECT") == 0){
				gui->modal = RECT;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "TEXT") == 0){
				gui->modal = TEXT;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "MOVE") == 0){
				gui->modal = MOVE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "DUPLI") == 0){
				gui->modal = DUPLI;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "SCALE") == 0){
				gui->modal = SCALE;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "NEW_BLK") == 0){
				gui->modal = NEW_BLK;
				gui->step = 0;
			}
			else if (strcmp(recv_comm, "INSERT") == 0){
				gui->modal = INSERT;
				gui->step = 0;
			}
		}
		/**********************************/
		
		gui_update_pos(gui);
		
		/**********************************/
		
		if (gui->prev_modal != gui->modal){
			gui->prev_modal = gui->modal;
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
			
		gui_insert_interactive(gui);
		gui_block_interactive(gui);
		gui_hatch_interactive(gui);
		
		
		
		
		
		
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
			if (gui->phanton){
				//free(phanton->data);
				//free(phanton);
				gui->phanton = NULL;
			}
		next_step:
			
			//gui->draw_tmp = 0;
			//gui->element = NULL;
			gui->lock_ax_x = 0;
			gui->lock_ax_y = 0;
			gui->user_flag_x = 0;
			gui->user_flag_y = 0;

			gui->draw = 1;
		end_step: ;
		
		
		
		if (gui_check_draw(gui) != 0){
			gui->draw = 1;
		}
		
		if (gui->draw != 0){
			/*get current window size */
			SDL_GetWindowSize(window, &gui->win_w, &gui->win_h);
			if (gui->win_w > gui->main_w){ /* if window exceedes main image */
				/* fit windo to main image size*/
				gui->win_w = gui->main_w;
				SDL_SetWindowSize(window, gui->win_w, gui->win_h);
			}
			if (gui->win_h > gui->main_h){/* if window exceedes main image */
				/* fit windo to main image size*/
				gui->win_h = gui->main_h;
				SDL_SetWindowSize(window, gui->win_w, gui->win_h);
			}
			
			/* set image visible window*/
			img->clip_x = 0; img->clip_y = gui->main_h - gui->win_h;
			img->clip_w = gui->win_w;
			img->clip_h = gui->win_h;
		
			bmp_fill_clip(img, img->bkg); /* clear bitmap */
			dxf_ents_draw(gui->drawing, img, gui->ofs_x, gui->ofs_y, gui->zoom); /* redraw */
			
			/*===================== teste ===============*/
			//graph_list_draw(tt_test, img, gui->ofs_x, gui->ofs_y, gui->zoom);
			
			//graph_draw(hers, img, gui->ofs_x, gui->ofs_y, gui->zoom);
			/*===================== teste ===============*/
			
			draw_cursor(img, gui->mouse_x, gui->mouse_y, cursor);
			
			if (gui->near_attr){ /* check if needs to draw an attractor mark */
				/* convert entities coordinates to screen coordinates */
				int attr_x = (int) round((gui->near_x - gui->ofs_x) * gui->zoom);
				int attr_y = (int) round((gui->near_y - gui->ofs_y) * gui->zoom);
				draw_attractor(img, gui->near_attr, attr_x, attr_y, yellow);
			}
			/*hilite test */
			if((gui->draw_tmp)&&(gui->element != NULL)){
				gui->element->obj.graphics = dxf_graph_parse(gui->drawing, gui->element, 0 , 1);
			}
			if(gui->element != NULL){
				graph_list_draw_fix(gui->element->obj.graphics, img, gui->ofs_x, gui->ofs_y, gui->zoom, hilite);
			}
			if((gui->draw_phanton)&&(gui->phanton)){
				//dxf_list_draw(gui->sel_list, img, gui->ofs_x - (x1 - x0), gui->ofs_y - (y1 - y0), gui->zoom, hilite);
				graph_list_draw_fix(gui->phanton, img, gui->ofs_x, gui->ofs_y, gui->zoom, hilite);
			}
			dxf_list_draw(gui->sel_list, img, gui->ofs_x, gui->ofs_y, gui->zoom, hilite);
			
			
			/* set image visible window*/
			img->clip_x = 0; img->clip_y = 0;
			
			/*draw gui*/
			nk_sdl_render(gui, img);
			
			
			
			
			win_r.x = 0; win_r.y = 0;
			win_r.w = gui->win_w; win_r.h = gui->win_h;
			
			SDL_UpdateTexture(canvas, &win_r, img->buf, gui->main_w * 4);
			//SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, canvas, &win_r, NULL);
			SDL_RenderPresent(renderer);
			
			//SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			
			gui->draw = 0;
			
		}
		
		leftMouseButtonClick = 0;
		rightMouseButtonClick = 0;
		MouseMotion = 0;
		gui->keyEnter = 0;
		gui->ev = EV_NONE;
		
		//graph_mem_pool(ZERO_GRAPH, 2);
		//graph_mem_pool(ZERO_LINE, 2);
		
		graph_mem_pool(ZERO_GRAPH, 1);
		graph_mem_pool(ZERO_LINE, 1);
		graph_mem_pool(ZERO_GRAPH, FRAME_LIFE);
		graph_mem_pool(ZERO_LINE, FRAME_LIFE);
		list_mem_pool(ZERO_LIST, FRAME_LIFE);
		
		nk_clear(gui->ctx); /*IMPORTANT */
		if (low_proc){
			SDL_Delay(30);
			SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEMOTION);
		}
	}
	
	/* safe quit */
	//SDL_free(base_path);
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	if(free_font_list(gui->font_list)){
		//printf("\n------------------------------------------\n         FREE FONT OK\n---------------------------------\n");
	}
	
	list_mem_pool(FREE_LIST, 0);
	list_mem_pool(FREE_LIST, 1);
	list_mem_pool(FREE_LIST, ONE_TIME);
	list_mem_pool(FREE_LIST, PRG_LIFE);
	dxf_mem_pool(FREE_DXF, 0);
	graph_mem_pool(FREE_ALL, 0);
	graph_mem_pool(FREE_ALL, 1);
	graph_mem_pool(FREE_ALL, 2);
	graph_mem_pool(FREE_ALL, PRG_LIFE);
	
	do_mem_pool(FREE_DO_ALL);
	
	bmp_free(img);
	bmp_free(color_img);
	bmp_free(gui->preview_img);
	bmp_free(i_cz48);
	
	i_svg_free_bmp(gui->svg_bmp);
	i_svg_free_curves(gui->svg_curves);
	
	
	
	
	//dxf_hatch_free(gui->list_pattern.next);
	dxf_h_fam_free(gui->hatch_fam.next);
	
	
	free(gui->drawing);
	nk_sdl_shutdown(gui);
	
	
	lua_close(Lua1);
	return 0;
	
};

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