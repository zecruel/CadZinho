
#ifndef _CZ_GUI_LIB
#define _CZ_GUI_LIB

#include <time.h>

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "draw_gl.h"
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

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"

#include "nanosvg.h"
#include "nanosvgrast.h"

#include "strpool.h"

#include "shp_font.h"
#include "ltype.h"
#include "hatch_pat.h"

#define FONT_SCALE 1.4
#define FIXED_MEM 128*1024

//#define PATH_MAX_CHARS 512
#define DRWG_HIST_MAX 50
#define BRK_PTS_MAX 50
#define DRWG_RECENT_MAX 10

#define MAX_SCRIPTS 32

#define ICON_SIZE 24

#define SMART_STEP(x) pow(10.0, floor(log10(fabs(x) + 1.0e-8)) - 1.0)

enum Action {
	NONE,
	FILE_NEW,
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
	REDRAW,
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
	RECTANGLE,
	TEXT,
	MTEXT,
	SINGLE_POINT, /*ARC, */
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
	SCRIPT,
	SPLINE,
	ELLIPSE,
	IMAGE,
	ADD_ATTRIB,
	ADD_XDATA,
	EXPLODE,
	MEASURE,
	FIND,
	PROP,
	TXT_PROP,
	VERTEX,
	DIM_LINEAR,
	DIM_ANGULAR,
	DIM_RADIUS,
	DIM_ORDINATE,
	ZOOM,
  PAN,
	MODAL_SIZE
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
	EV_ADD = 1024,
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
	SEL_RECTANGLE,
	SEL_INTL
};

enum Spline_mode {
	SP_CTRL,
	SP_FIT
};

enum Circle_mode{
	CIRCLE_FULL,
	CIRCLE_ARC
};

enum Ellipse_mode{
	EL_FULL,
	EL_ARC,
	EL_ISO_CIRCLE,
	EL_ISO_ARC
};

enum Rotate_mode{
	ROT_ANGLE,
	ROT_3POINTS
};

enum Scale_mode{
	SCALE_FACTOR,
	SCALE_3POINTS
};

enum Ortho_view{
	O_TOP,
	O_FRONT,
	O_LEFT,
	O_BOT,
	O_RIGHT,
	O_BACK
};

enum theme {THEME_BLACK,
	THEME_WHITE,
	THEME_RED,
	THEME_BLUE,
	THEME_DARK,
	THEME_GREEN,
	THEME_BROWN,
	THEME_PURPLE,
	THEME_DRACULA,
	THEME_DEFAULT};
	
enum Cursor_type{
	CURSOR_CROSS,
	CURSOR_SQUARE,
	CURSOR_X,
	CURSOR_CIRCLE
};

enum Hist_action {
	HIST_NONE,
	HIST_ADD,
	HIST_PREV,
	HIST_NEXT
};

enum Prev_typ {
  PRV_BLOCK,
  PRV_PRINT,
  PRV_HATCH,
  PRV_INSERT,
  PRV_FONT,
  PRV_SCRIPT,
  PRV_OTHER,
  
  PRV_SIZE
};

struct gui_glyph{
	int code_p, x, y, w, h;
	double adv;
	struct gui_glyph *next;
	unsigned char rast[20*25][4];
	
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

struct func_key {
	char key[15];
	SDL_Keycode code;
	SDL_Keymod mod;
};

struct Gui_obj {
  int running;
  int low_proc;
  int changed;
  int near_count;
	int sel_idx;
  int sel_count;
	int free_sel;
  
  int next_win_x, next_win_y, next_win_w, next_win_h;
	int mouse_x, mouse_y;//, mouse_z;
  int prev_mouse_x, prev_mouse_y, pan_mode;
  
  int color_idx, lw_idx, t_al_v, t_al_h;
	int layer_idx, ltypes_idx, t_sty_idx;
	
	int step, user_flag_x, user_flag_y, lock_ax_x, lock_ax_y, user_number;
	int keyEnter;
	int draw, draw_tmp, draw_phanton, draw_vert, vert_idx, safe_draw;
	int near_attr;
	
	int text2tag, hide_tag;
	
	int hatch_fam_idx, hatch_idx, h_type, hatch_assoc, hatch_t_box;
	int hatch_sel;
	
	int keep_orig;
	int closed, proportional;
	
	int sp_degree;
  
  int image_w, image_h;
	
	int en_distance; /* enable distance entry */
	int entry_relative;
	int rect_polar; /* mode rectangular / polar to enter coordinates or distances */
  
  int file_filter_count;
	int filter_idx;
	
	int show_open;
	int show_save;
	
	int show_app_about;
	int show_app_file;
	int path_ok;
	int show_info;
	int show_script;
	int show_print;
	int show_export;
	
	int progress;
	int hist_new;
	int show_lay_mng;
	int show_color_pick;
	int show_tstyles_mng;
	int show_blk_mng;
	int show_ltyp_mng;
	int show_dim_mng;
	int show_hatch_mng;
	int show_config;
	int show_plugins;
  
  int show_file_br;
  
  int drwg_hist_size;
	int drwg_hist_pos;
	int drwg_hist_wr;
	int drwg_hist_head;
  
  int paper_fam;
	int sel_paper;
  
  int num_brk_pts;
	int script_resume_reason;
	int script_resume;
  
  int discard_changes;
  
  /* Window dimension */
	int win_x;
	int win_y;
	unsigned int win_w;
	unsigned int win_h;
  
  enum theme theme;
	enum Cursor_type cursor;
  enum list_op_mode sel_mode;
	enum Sel_type sel_type;
	enum dxf_graph sel_ent_filter;
  enum Spline_mode spline_mode;
	
	enum Scale_mode scale_mode;
	enum Rotate_mode rot_mode;
	
	enum Explode_mode expl_mode;
	
	enum Circle_mode circle_mode;
	
	enum Ellipse_mode el_mode;
	enum Ortho_view o_view;
	
	enum Action desired_action;
	enum Hist_action hist_action;
	
	enum Action action;
	enum Modal modal, prev_modal;
	enum Gui_ev ev;
	enum attract_type curr_attr_t;
  
  long file_size;
  
  char log_msg[64];
	char txt[DXF_MAX_CHARS + 1];
	char tag[DXF_MAX_CHARS + 1];
  char value[DXF_MAX_CHARS + 1];
	char long_txt[5 * DXF_MAX_CHARS];
	char blk_name[DXF_MAX_CHARS + 1];
	char blk_descr[DXF_MAX_CHARS + 1];
	char tag_mark[DXF_MAX_CHARS + 1];
	char hide_mark[DXF_MAX_CHARS + 1];
	char value_mark[DXF_MAX_CHARS + 1];
	char dflt_value[DXF_MAX_CHARS + 1];
	
	char patt_name[DXF_MAX_CHARS];
	char patt_descr[DXF_MAX_CHARS];
	char h_fam_name[DXF_MAX_CHARS];
	char h_fam_descr[DXF_MAX_CHARS];
	
	char dflt_fonts_path[5 * DXF_MAX_CHARS+1];
  
  char curr_path[PATH_MAX_CHARS + 1];
	char base_dir[PATH_MAX_CHARS + 1];
	char dwg_file[DXF_MAX_CHARS + 1];
	char dwg_dir[PATH_MAX_CHARS + 1];
	char pref_path[PATH_MAX_CHARS + 1];
	
	char drwg_hist[DRWG_HIST_MAX][DXF_MAX_CHARS+1];
  
  char const * file_filter_types[20];
	char const * file_filter_descr[20];
  
  char func_keys_path[DXF_MAX_CHARS + 1];
  
  char main_lang[DXF_MAX_CHARS + 1]; /* gui language */
  
  char clip_path[DXF_MAX_CHARS + 1];
  
  char image_path[DXF_MAX_CHARS];
  
  char curr_script[PATH_MAX_CHARS];
	
	unsigned char blank_tex[4*20*600]; /* blank texture */
  
  float alpha, beta, gamma;
	
	float drwg_view[4][4];
	float drwg_view_i[4][4];
  float model_view[3][3];
  
  /*gui pos variables */
	double zoom, ofs_x, ofs_y, ofs_z, mouse_z;
	double prev_zoom;
	
	double user_x, user_y;
	double near_x, near_y;
	double bulge, scale_x, scale_y, angle;
	double txt_h;
	double rect_w;
	
	double patt_scale, patt_ang;
  
  double step_x[1000], step_y[1000];
  
  char * seed;
	char * dflt_pat;
	char * dflt_lin;
	char * extra_lin;
  
  void * buf; /*for fixed memory */
	void * last; /* to verify if needs to draw */
  
	struct nk_context * ctx;
	struct nk_user_font * font;
  
	struct gui_font * ui_font_list;
  
  SDL_Window * window;
  SDL_Cursor * dflt_cur;
  SDL_Cursor * modal_cursor[MODAL_SIZE];
  
  struct do_entry * save_pt;
  
  struct Mem_buffer * file_buf;
  
  NSVGimage ** svg_curves;
	bmp_img ** svg_bmp;
	bmp_img * preview[PRV_SIZE];
	bmp_img * color_img;
	bmp_img * i_cz48;
	bmp_img * i_trash;
	bmp_img * attr_vec[15];
	
	dxf_drawing * drawing; /* main drawing buffer */
  dxf_drawing * clip_drwg;
	dxf_node * element, * near_el;
  
  list_node * near_list;
  
  list_node * sel_list;
	list_node * phanton;
  list_node * recent_drwg;
	list_node * font_list;
  
  struct h_family * end_fam;
  struct tfont * dflt_font;
  
  bmp_color background;
	bmp_color hilite;
	
  strpool_t file_pool;
  
  struct script_obj macro_script;
  struct script_obj func_keys_script;
  
  struct nk_user_font ui_font;
	struct nk_user_font alt_font_sizes[FONT_NUM_SIZE];
  
	struct ogl gl_ctx;
	
	struct nk_style_button b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	struct nk_style_button b_icon_sel, b_icon_unsel;
	
	struct h_pattern list_pattern;
	struct hatch_line user_patt;
	struct h_family hatch_fam;
	
  struct do_list list_do;
	
	struct nk_text_edit text_edit;
	struct nk_text_edit debug_edit;
	
  struct script_thread script_wait_t;
  
	struct script_obj plugins_script;
  
  struct script_obj main_lang_scr;
  
  struct script_obj lua_script[MAX_SCRIPTS];
  
  struct script_brk_pt brk_pts[BRK_PTS_MAX];
  
  SDL_Thread* debug_thread_id;
  char debug_host[DXF_MAX_CHARS + 1];
  char debug_port[11];
  int debug_connected;
  int debug_step;
  int debug_pause;
  int debug_level;
  int debug_step_level;
  
};
typedef struct Gui_obj gui_obj;

struct gui_font * gui_new_font (struct nk_user_font *base_font);

int gui_list_font_free (struct gui_font *list);



void gui_scr_coord (gui_obj *gui, int scr_x, int scr_y, double *x, double *y);

void gui_coord_scr (gui_obj *gui, double x, double y, int *scr_x, int *scr_y);

int gui_scr_visible (gui_obj *gui, double x, double y);

void gui_scr_centralize(gui_obj *gui, double x, double y);

void sel_list_clear (gui_obj *gui);

int gui_selectable (gui_obj *gui, const char *title, int active);
int gui_tab (gui_obj *gui, const char *title, int active);
int gui_tab_img (gui_obj *gui, bmp_img *img, int active, int w);

void set_style(gui_obj *gui, enum theme theme);

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

int gui_create_modal_cur(gui_obj *gui);

int gui_free_modal_cur(gui_obj *gui);

int gui_default_modal(gui_obj *gui);

int gui_first_step(gui_obj *gui);

int gui_next_step(gui_obj *gui);

void gui_simple_select(gui_obj *gui);

void gui_draw_vert(gui_obj *gui, bmp_img *img, dxf_node *obj);


int nk_gl_render(gui_obj *gui) ;

int draw_cursor_gl(gui_obj *gui, int x, int y, enum Cursor_type type);

int draw_attractor_gl(gui_obj *gui, enum attract_type type, int x, int y, bmp_color color);

void gui_draw_vert_gl(gui_obj *gui, dxf_node *obj);

char* gui_get_literal (gui_obj *gui, const char *literal);

extern int dxf_lw[];
extern const char *dxf_lw_descr[];
extern bmp_color dxf_colors[];
extern const char *dxf_seed_2007;
extern struct func_key func_keys[];
extern const int func_keys_size;
extern unsigned int wait_open;

#ifndef DXF_LW_LEN
	#define DXF_LW_LEN 24
#endif

#define _l(X) gui_get_literal (gui, X)

#endif
