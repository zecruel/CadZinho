#ifndef _CZ_SCRIPT_LIB
#define _CZ_SCRIPT_LIB

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "gui.h"

int set_timeout (lua_State *L);
int script_ent_append (lua_State *L);
int script_new_pline (lua_State *L);
int script_pline_append (lua_State *L);


#endif