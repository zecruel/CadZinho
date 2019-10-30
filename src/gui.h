
#ifndef _CZ_GUI_LIB
#define _CZ_GUI_LIB

#include <time.h>

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "i_svg_media.h"
#include "list.h"
#include "dxf_create.h"
#include "dxf_copy.h"
#include "dxf_edit.h"
#include "dxf_attract.h"
#include "dxf_hatch.h"
#include "font.h"
#include "script.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"

#include "nanosvg.h"
#include "nanosvgrast.h"

#define FONT_SCALE 1.4
#define FIXED_MEM 128*1024

#define MAX_PATH_LEN 512
#define DRWG_HIST_MAX 50
#define BRK_PTS_MAX 50

enum Action {
	NONE,
	FILE_OPEN,
	FILE_SAVE,
	EXPORT,
	VIEW_ZOOM_EXT,
	VIEW_ZOOM_P,
	VIEW_ZOOM_M,
	VIEW_ZOOM_W,
	VIEW_PAN_U,
	VIEW_PAN_D,
	VIEW_PAN_L,
	VIEW_PAN_R,
	DELETE,
	UNDO,
	REDO,
	LAYER_CHANGE,
	COLOR_CHANGE,
	LTYPE_CHANGE,
	LW_CHANGE,
	YANK,
	CUT,
	START_PASTE,
	EXIT
};

enum Modal {
	SELECT,
	LINE,
	POLYLINE,
	CIRCLE,
	RECT,
	TEXT,
	MTEXT,
	ARC,
	DUPLI,
	MOVE,
	SCALE,
	ROTATE,
	NEW_BLK,
	HATCH,
	INSERT,
	PASTE,
	MIRROR,
	ED_TEXT,
	ED_ATTR,
	SCRIPT
};

enum Gui_ev {
	EV_NONE = 0,
	EV_ENTER = 1,
	EV_CANCEL = 2,
	EV_MOTION = 4,
	EV_LOCK_AX = 8,
	EV_YANK = 16,
	EV_CUT = 32,
	EV_PASTE = 64,
	EV_UNDO = 128,
	EV_REDO = 256,
	EV_DEL = 512,
};

enum Font_sizes {
	FONT_SMALL,
	FONT_REGULAR,
	FONT_BIG,
	FONT_HUGE,
	
	FONT_NUM_SIZE
};

enum Sel_type {
	SEL_ELEMENT,
	SEL_RECTANGLE
};

struct gui_glyph{
	int code_p, x, y, w, h;
	double adv;
	unsigned char rast[20*25];
	struct gui_glyph *next;
};
	
struct gui_font{
	struct nk_user_font *font;
	struct gui_glyph *glyphs;
	struct gui_font *next;
};

struct script_brk_pt{
	char source[DXF_MAX_CHARS];
	unsigned long line;
	int enable;
};

struct Gui_obj {
	struct nk_context *ctx;
	struct nk_user_font *font;
	struct gui_font *ui_font_list;
	void *buf; /*for fixed memory */
	void *last; /* to verify if needs to draw */
	
	dxf_drawing *drawing;
	dxf_node *element, *near_el;
	
	list_node *near_list;
	int near_count;
	int sel_idx;
	enum list_op_mode sel_mode;
	enum Sel_type sel_type;
	int sel_count;
	
	
	/* background image dimension */
	unsigned int main_w;
	unsigned int main_h;
	
	/* Window dimension */
	unsigned int win_w;
	unsigned int win_h;
	
	/*gui pos variables */
	int next_win_x, next_win_y, next_win_w, next_win_h;
	int mouse_x, mouse_y;
	double zoom, ofs_x, ofs_y;
	double prev_zoom;
	
	double user_x, user_y;
	double step_x[10], step_y[10];
	double near_x, near_y;
	double bulge, scale, angle;
	double txt_h;
	double rect_w;
	
	double patt_scale, patt_ang;
	
	int color_idx, lw_idx, t_al_v, t_al_h;
	int layer_idx, ltypes_idx, t_sty_idx;
	
	int step, user_flag_x, user_flag_y, lock_ax_x, lock_ax_y, user_number;
	int keyEnter;
	int draw, draw_tmp, draw_phanton;
	int near_attr;
	
	int text2tag;
	
	int hatch_fam_idx, hatch_idx, h_type, hatch_assoc;
	
	int keep_orig;
	
	
	int en_distance; /* enable distance entry */
	int entry_relative;
	
	enum Action action;
	enum Modal modal, prev_modal;
	enum Gui_ev ev;
	int modstates;
	enum attract_type curr_attr_t;
	
	bmp_color background;
	
	NSVGimage **svg_curves;
	bmp_img **svg_bmp;
	bmp_img *preview_img;
	
	struct nk_style_button b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	struct nk_style_button b_icon_sel, b_icon_unsel;
	
	char log_msg[64];
	char txt[DXF_MAX_CHARS];
	char long_txt[5 * DXF_MAX_CHARS];
	char blk_name[DXF_MAX_CHARS];
	char tag_mark[DXF_MAX_CHARS];
	
	char patt_name[DXF_MAX_CHARS];
	char patt_descr[DXF_MAX_CHARS];
	char h_fam_name[DXF_MAX_CHARS];
	char h_fam_descr[DXF_MAX_CHARS];
	
	char dflt_fonts_path[5 * DXF_MAX_CHARS];
	
	list_node * sel_list;
	list_node *phanton;
	
	struct do_list list_do;
	
	list_node *font_list;
	//struct tfont * ui_font;
	struct nk_user_font ui_font;
	struct nk_user_font alt_font_sizes[FONT_NUM_SIZE];
	struct tfont * dflt_font;
	
	struct h_pattern list_pattern;
	struct hatch_line user_patt;
	struct h_family hatch_fam;
	struct h_family *end_fam;
	
	int show_file_br;
	char const * file_filter_types[20];
	char const * file_filter_descr[20];
	int file_filter_count;
	int filter_idx;
	
	char curr_path[MAX_PATH_LEN];
	char base_dir[DXF_MAX_CHARS];
	char dwg_file[DXF_MAX_CHARS];
	char dwg_dir[DXF_MAX_CHARS];
	
	char drwg_hist[DRWG_HIST_MAX][DXF_MAX_CHARS];
	int drwg_hist_size;
	int drwg_hist_pos;
	int drwg_hist_wr;
	int drwg_hist_head;
	
	struct nk_text_edit text_edit;
	struct nk_text_edit debug_edit;
	
	int paper_fam;
	int sel_paper;
	
	int show_edit_text;
	
	dxf_drawing *clip_drwg;
	
	struct script_obj lua_script;
	char curr_script[MAX_PATH_LEN];
	clock_t script_time;
	double script_timeout;
	struct script_brk_pt brk_pts[BRK_PTS_MAX];
	int num_brk_pts;
	
	char script_win[DXF_MAX_CHARS];
	char script_win_title[DXF_MAX_CHARS];
	int script_win_x, script_win_y, script_win_w, script_win_h;
	
	char script_dynamic[DXF_MAX_CHARS];
	
};
typedef struct Gui_obj gui_obj;

enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_ZE};

int gui_tab (gui_obj *gui, const char *title, int active);

void set_style(struct nk_context *ctx, enum theme theme);

float nk_user_font_get_text_width(nk_handle handle, float height, const char *text, int len);

bmp_color nk_to_bmp_color(struct nk_color color);

int gui_check_draw(gui_obj *gui);

NK_API void nk_sdl_render(gui_obj *gui, bmp_img *img);

static void nk_sdl_clipbard_paste(nk_handle usr, struct nk_text_edit *edit);

static void nk_sdl_clipbard_copy(nk_handle usr, const char *text, int len);

NK_API int nk_sdl_init(gui_obj* gui);

NK_API int nk_sdl_handle_event(gui_obj *gui, SDL_Window *win, SDL_Event *evt);

NK_API void nk_sdl_shutdown(gui_obj *gui);

int gui_start(gui_obj *gui);

int gui_tstyle(gui_obj *gui);

int gui_tstyle2(gui_obj *gui, dxf_drawing *drawing);

int gui_default_modal(gui_obj *gui);

int gui_first_step(gui_obj *gui);

int gui_next_step(gui_obj *gui);

extern int dxf_lw[];
extern const char *dxf_lw_descr[];
extern bmp_color dxf_colors[];
extern const char *text_al_h[];
extern const char *text_al_v[];
extern const char *std_h_pat;
extern const char *shp_font_romans;
extern const char *shp_font_txt;



#ifndef DXF_LW_LEN
	#define DXF_LW_LEN 24
#endif

#ifndef T_AL_H_LEN	
	#define T_AL_H_LEN 6
#endif

#ifndef T_AL_V_LEN
	#define T_AL_V_LEN 4
#endif

#endif
