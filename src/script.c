#include "script.h"

/* set timeout variable */
int set_timeout (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* get passed value */
	int isnum = 0;
	double timeout = lua_tonumberx (L, -1, &isnum);
	/* verify if a valid number */
	if (isnum){
		gui->script_timeout = timeout; /* set timeout value */
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_ent_append (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	if (!lua_isuserdata(L, 1)) {
		lua_pushliteral(L, "ent_append: incorrect argument type");
		lua_error(L);
	}
	
	dxf_node *new_el = (dxf_node *) lua_touserdata (L, 1);
	if (new_el) {
		new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
		drawing_ent_append(gui->drawing, new_el);
		
		do_add_item(gui->list_do.current, NULL, new_el);
		lua_pushboolean(L, 1); /* return success */
	}
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_new_pline (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 6){
		lua_pushliteral(L, "new_pline: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 6; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_pline: incorrect argument type");
			lua_error(L);
		}
	}
	
	dxf_node *new_el = (dxf_node *) dxf_new_lwpolyline (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, /* pt1, */
		lua_tonumber(L, 3), /* bulge */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, DWG_LIFE); /* paper space */
	dxf_lwpoly_append (new_el, lua_tonumber(L, 4), lua_tonumber(L, 5), 0.0, lua_tonumber(L, 6), DWG_LIFE);
	
	if (!new_el) lua_pushnil(L); /* return fail */
	else lua_pushlightuserdata(L, (void *) new_el); /* return success */
	return 1;
}

int script_pline_append (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	if (!lua_isuserdata(L, 1)) {
		lua_pushliteral(L, "pline_append: incorrect argument type");
		lua_error(L);
	}
	int i;
	for (i = 2; i <= 4; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "pline_append: incorrect argument type");
			lua_error(L);
		}
	}
	
	dxf_node *new_el = (dxf_node *) lua_touserdata (L, 1);
	if (new_el) {
		dxf_lwpoly_append (new_el, lua_tonumber(L, 2), lua_tonumber(L, 3), 0.0, lua_tonumber(L, 4), DWG_LIFE);
		lua_pushboolean(L, 1); /* return success */
	}
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_pline_close (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	if (!lua_isuserdata(L, 1)) {
		lua_pushliteral(L, "pline_close: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isboolean(L, 2)) {
		lua_pushliteral(L, "pline_close: incorrect argument type");
		lua_error(L);
	}
	
	dxf_node *new_el = (dxf_node *) lua_touserdata (L, 1);
	if (new_el) {
		int closed = lua_toboolean(L, 2);
		lua_pushboolean(L, dxf_attr_change_i(new_el, 70, (void *) &closed, 0)); /* return success */
	}
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_new_circle (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "new_circle: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 3; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_circle: incorrect argument type");
			lua_error(L);
		}
	}
	
	dxf_node * new_el = (dxf_node *) dxf_new_circle (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, lua_tonumber(L, 3), /* pt1, radius */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, DWG_LIFE); /* paper space */
	
	if (!new_el) lua_pushnil(L); /* return fail */
	else lua_pushlightuserdata(L, (void *) new_el); /* return success */
	return 1;
}

int script_nk_begin (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 5){
		lua_pushliteral(L, "nk_begin: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_begin: incorrect argument type");
		lua_error(L);
	}
	
	int i;
	for (i = 2; i <= 5; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "nk_begin: incorrect argument type");
			lua_error(L);
		}
	}
	
	int ret = nk_begin(gui->ctx, lua_tostring(L, 1),
		nk_rect(lua_tonumber(L, 2), lua_tonumber(L, 3),
			lua_tonumber(L, 4), lua_tonumber(L, 5)),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
		NK_WINDOW_SCALABLE|
		NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE);
	
	if (ret) lua_pushboolean(L, 1); /* return success */
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_nk_end (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	nk_end(gui->ctx);
	return 0;
}

int script_win_show (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "win_show: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "win_show: incorrect argument type");
		lua_error(L);
	}
	const char *name = lua_tostring(L, 1);
	if (strlen(name) > 0){
		gui->lua_script.active = 1;
		strncpy(gui->script_win, name, DXF_MAX_CHARS - 1);
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_win_close (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if gui is valid */
	if (!gui){
		lua_pushliteral(L, "Auto check: no access to CadZinho enviroment");
		lua_error(L);
	}
	
	gui->lua_script.active = 0;
	gui->script_win[0] = 0;
	return 0;
}