#ifndef _CZ_SCRIPT_LIB
#define _CZ_SCRIPT_LIB

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dxf.h"

struct script_obj{
	lua_State *L; /* main lua state */
	lua_State *T; /* thread for execution */
	int status;
	int active;
	int dynamic;
	char path[DXF_MAX_CHARS];
};

struct ent_lua { /* DXF entity object, in Lua userdata */
	dxf_node *curr_ent;
	dxf_node *orig_ent;
	int sel;
};

#include "gui.h"

int set_timeout (lua_State *L);
int script_get_sel (lua_State *L);
int script_ent_write (lua_State *L);

int script_get_ent_typ (lua_State *L);
int script_count_attrib (lua_State *L);
int script_get_attrib_i (lua_State *L);
int script_get_points (lua_State *L);
int script_get_ext (lua_State *L);

int script_edit_attr (lua_State *L);

//int script_ent_append (lua_State *L);
int script_new_line (lua_State *L);
int script_new_pline (lua_State *L);
int script_pline_append (lua_State *L);
int script_pline_close (lua_State *L);
int script_new_circle (lua_State *L);
int script_new_hatch (lua_State *L);
int script_new_text (lua_State *L);
int script_new_block (lua_State *L);

int script_get_dwg_appids (lua_State *L);

int script_set_layer (lua_State *L);
int script_set_color (lua_State *L);
int script_set_ltype (lua_State *L);
int script_set_style (lua_State *L);
int script_set_lw (lua_State *L);

int script_start_dynamic (lua_State *L);
int script_stop_dynamic (lua_State *L);
int script_ent_draw (lua_State *L);

int script_win_show (lua_State *L);
int script_win_close (lua_State *L);
int script_nk_layout (lua_State *L);
int script_nk_button (lua_State *L);
int script_nk_label (lua_State *L);
int script_nk_edit (lua_State *L);

#endif