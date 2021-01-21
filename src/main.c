#include <SDL.h>

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "dxf_graph.h"
#include "list.h"
#include "dxf_create.h"
#include "dxf_copy.h"
#include "dxf_attract.h"
#include "dxf_print.h"


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



/* ---------------------------------------------------------*/
/* ------------------   GLOBALS  ----------------------- */

#include "dxf_colors.h"
struct Matrix *aux_mtx1 = NULL;

/* ---------------------------------------------------------*/
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

int main(int argc, char** argv){
	aux_mtx1 = malloc(sizeof(struct Matrix));
	
	gui_obj *gui = malloc(sizeof(gui_obj));
	gui_start(gui);
	
	
	
	/* full path of clipboard file */
	char clip_path[DXF_MAX_CHARS + 1];
	clip_path[0] = 0;
	strncpy(clip_path, gui->base_dir, DXF_MAX_CHARS);
	printf("base dir = %s\n", gui->base_dir);
	strncat(clip_path, "test_clip.dxf", DXF_MAX_CHARS);
	printf("clip path = %s\n", clip_path);
	
	/* full path of init file */
	char init_path[DXF_MAX_CHARS + 1];
	init_path[0] = 0;
	strncpy(init_path, gui->base_dir, DXF_MAX_CHARS);
	strncat(init_path, "init.lua", DXF_MAX_CHARS);
	
	/* initialize fonts paths with base directory  */
	if (strlen(gui->base_dir)){
		strncpy (gui->dflt_fonts_path, gui->base_dir, 5 * DXF_MAX_CHARS);
		strncat (gui->dflt_fonts_path, (char []){PATH_SEPARATOR, 0}, 5 * DXF_MAX_CHARS);
	}
	
	//setlocale(LC_ALL,""); //seta a localidade como a current do computador para aceitar acentuacao
	int i, ok;
	
	//load (Lua1, "config.lua", &gui->win_w, &gui->win_h);
	gui_load_conf ("config.lua", gui);
	gui_load_ini("init.lua", gui);
	
	SDL_Rect win_r, display_r;
	
	/* init the SDL2 */
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window * window = SDL_CreateWindow(
		"CadZinho", /* title */
		gui->win_x, /* x position */
		gui->win_y, /* y position */
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
	struct nk_color background;
	
	
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
	bmp_color magenta_l = {.r = 255, .g = 0, .b = 255, .a = 150};
	bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
	bmp_color cursor = {.r = 255, .g = 255, .b = 255, .a = 100};
	background = nk_rgb(28,48,62);
	
	bmp_color hilite[] = {magenta_l, };
	
	/* line types in use */
	double center [] = {12, -6, 2 , -6};
	double dash [] = {8, -8};
	double continuous[] = {1};
	
	/* initialize the selection list */
	gui->sel_list = list_new(NULL, PRG_LIFE);
	
	/* initialize the undo/redo list */
	//struct do_list gui->list_do;
	init_do_list(&gui->list_do);
	
	
	/* init the main image */
	bmp_img * img = bmp_new(gui->main_w, gui->main_h, grey, red);
	
	/* init Nuklear GUI */
	
	nk_sdl_init(gui);
	
	//enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_GREEN};
	//~ set_style(gui->ctx, THEME_BLACK);
	//~ set_style(gui->ctx, THEME_WHITE);
	//~ set_style(gui->ctx, THEME_RED);
	//~ set_style(gui->ctx, THEME_BLUE);
	//~ set_style(gui->ctx, THEME_DARK);
	set_style(gui->ctx, THEME_GREEN);
	
	gui->ctx->style.edit.padding = nk_vec2(4, -6);
	
	/* init the toolbox image */
	
	
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
	gui->svg_bmp[SVG_CZ] = i_svg_bmp(gui->svg_curves[SVG_CZ], 32, 32);
	
	gui->i_cz48 = i_svg_bmp(gui->svg_curves[SVG_CZ], 48, 48);
	gui->i_trash = i_svg_bmp(gui->svg_curves[SVG_TRASH], 16, 16);
	
	
	attrc_get_imgs(gui->attr_vec, 15, 16, 16);
	
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
	//gui->b_icon_unsel.normal = nk_style_item_color(nk_rgba(58, 67, 57, 255));
	gui->b_icon_unsel.normal = gui->ctx->style.checkbox.normal;
	//gui->b_icon_unsel.hover = nk_style_item_color(nk_rgba(73, 84, 72, 255));
	gui->b_icon_unsel.hover =  gui->ctx->style.checkbox.hover;
	
	//gui->b_icon_unsel.active = nk_style_item_color(nk_rgba(81, 92, 80, 255));
	gui->b_icon_sel.image_padding.x = -4;
	gui->b_icon_sel.image_padding.y = -4;
	gui->b_icon_unsel.image_padding.x = -4;
	gui->b_icon_unsel.image_padding.y = -4;
	
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
	
	
	/* init the drawing */
	gui->drawing = dxf_drawing_new(DWG_LIFE);
	gui->drawing->font_list = gui->font_list;
	gui->drawing->dflt_font = gui->dflt_font;
	
	url = NULL; /* pass a null file only for initialize the drawing structure */
	
	/* **************** init the main drawing ************ */
	char *seed = try_load_dflt("seed.dxf", (char *)dxf_seed_2007);
	
	while (dxf_read (gui->drawing, seed, strlen(seed), &gui->progress) > 0){
		
	}
	
	free(seed);
	seed = NULL;
	
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
						
						if(event.wheel.y < 0) wheel = -1.0; // scroll down
						gui->prev_zoom = gui->zoom;
						gui->zoom = gui->zoom + wheel * 0.3 * gui->zoom;
						
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
					case SDL_KEYDOWN: {
						SDL_Keycode key = event.key.keysym.sym;
						SDL_Keymod mod = event.key.keysym.mod;
						
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
		if (MouseMotion) gui->ev |= EV_MOTION;
		if (leftMouseButtonClick) gui->ev |= EV_ENTER;
		if (rightMouseButtonClick) gui->ev |= EV_CANCEL;
		if (gui->keyEnter) gui->ev |= EV_LOCK_AX;
		if (ctrlDown) gui->ev |= EV_ADD;
		
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
			
			strncpy (gui->dwg_dir, get_dir(gui->curr_path) , DXF_MAX_CHARS);
			strncpy (gui->dwg_file, get_filename(gui->curr_path) , DXF_MAX_CHARS);
			
			char title[DXF_MAX_CHARS] = "CadZinho - ";
			strncat (title, gui->dwg_file, DXF_MAX_CHARS);
			SDL_SetWindowTitle(window, title);
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
				gui->lua_script.active = 0;
				gui->lua_script.dynamic = 0;
			}
			
			gui->prev_modal = gui->modal;
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
			
			d_param.ofs_x = gui->ofs_x;
			d_param.ofs_y = gui->ofs_y;
			d_param.scale = gui->zoom;
			d_param.list = NULL;
			d_param.subst = NULL;
			d_param.len_subst = 0;
			d_param.inc_thick = 0;
		
			bmp_fill_clip(img, img->bkg); /* clear bitmap */
			//dxf_ents_draw(gui->drawing, img, gui->ofs_x, gui->ofs_y, gui->zoom); /* redraw */
			dxf_ents_draw(gui->drawing, img, d_param);
			
			/*===================== teste ===============*/
			//graph_list_draw(tt_test, img, gui->ofs_x, gui->ofs_y, gui->zoom);
			
			//graph_draw(hers, img, gui->ofs_x, gui->ofs_y, gui->zoom);
			/*===================== teste ===============*/
			
			draw_cursor(img, gui->mouse_x, gui->mouse_y, cursor);
			
			
			/*hilite test */
			if((gui->draw_tmp)&&(gui->element != NULL)){
				gui->element->obj.graphics = dxf_graph_parse(gui->drawing, gui->element, 0 , 1);
			}
			
			
			
			d_param.subst = hilite;
			d_param.len_subst = 1;
			d_param.inc_thick = 3;
			
			if(gui->element != NULL){
				//graph_list_draw_fix(gui->element->obj.graphics, img, gui->ofs_x, gui->ofs_y, gui->zoom, hilite);
				graph_list_draw(gui->element->obj.graphics, img, d_param);
			}
			if((gui->draw_phanton)&&(gui->phanton)){
				//dxf_list_draw(gui->sel_list, img, gui->ofs_x - (x1 - x0), gui->ofs_y - (y1 - y0), gui->zoom, hilite);
				graph_list_draw(gui->phanton, img, d_param);
			}
			if (gui->sel_list->next) /* verify if  has elements in list */
				dxf_list_draw(gui->sel_list, img, d_param);
			
			
			if ((gui->draw_vert) && (gui->element)){
				/* draw vertices */
				gui_draw_vert(gui, img, gui->element);
			}
			
			if (gui->near_attr){ /* check if needs to draw an attractor mark */
				/* convert entities coordinates to screen coordinates */
				int attr_x = (int) round((gui->near_x - gui->ofs_x) * gui->zoom);
				int attr_y = (int) round((gui->near_y - gui->ofs_y) * gui->zoom);
				draw_attractor(img, gui->near_attr, attr_x, attr_y, yellow);
			}
			
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
	}
	
	/* safe quit */
	//SDL_free(base_path);
	
	dxf_drawing_clear(gui->drawing);
	dxf_drawing_clear(gui->clip_drwg);
	
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
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
	
	bmp_free(img);
	bmp_free(gui->color_img);
	bmp_free(gui->preview_img);
	bmp_free(gui->i_cz48);
	bmp_free(gui->i_trash);
	
	i_svg_free_bmp(gui->svg_bmp);
	i_svg_free_curves(gui->svg_curves);
	
	
	
	
	//dxf_hatch_free(gui->list_pattern.next);
	dxf_h_fam_free(gui->hatch_fam.next);
	
	free(gui->clip_drwg);
	
	free(gui->drawing);
	free(aux_mtx1);
	nk_sdl_shutdown(gui);
	manage_buffer(0, BUF_FREE);
	
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