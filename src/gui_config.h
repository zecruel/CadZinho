#ifndef _CZ_GUI_CONFIG_LIB
#define _CZ_GUI_CONFIG_LIB

#include "gui.h"
#include "gui_file.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

const char* gui_dflt_conf();

int gui_load_conf (gui_obj *gui);

int gui_reload_conf (gui_obj *gui);

int gui_get_conf (lua_State *L);

int gui_save_init (char *fname, gui_obj *gui);

int gui_get_ini (lua_State *L);

int config_win (gui_obj *gui);


#endif