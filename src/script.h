#ifndef _CZ_SCRIPT_LIB
#define _CZ_SCRIPT_LIB

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "gui.h"

struct script_obj{
	lua_State *L; /* main lua state */
	lua_State *T; /* thread for execution */
	int status;
	int active;
	char path[DXF_MAX_CHARS];
};

int set_timeout (lua_State *L);
int script_ent_append (lua_State *L);
int script_new_pline (lua_State *L);
int script_pline_append (lua_State *L);
int script_pline_close (lua_State *L);
int script_new_circle (lua_State *L);


#endif