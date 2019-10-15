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
	
	dxf_node *ent = (dxf_node *) lua_touserdata (L, 1);
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	dxf_node *new_ent = dxf_ent_copy(ent, DWG_LIFE);
	
	if (!new_ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 , 0);
	drawing_ent_append(gui->drawing, new_ent);
	
	do_add_item(gui->list_do.current, NULL, new_ent);
	
	lua_pushboolean(L, 1); /* return success */
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
		0, FRAME_LIFE); /* paper space */
	dxf_lwpoly_append (new_el, lua_tonumber(L, 4), lua_tonumber(L, 5), 0.0, lua_tonumber(L, 6), FRAME_LIFE);
	
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
		dxf_lwpoly_append (new_el, lua_tonumber(L, 2), lua_tonumber(L, 3), 0.0, lua_tonumber(L, 4), FRAME_LIFE);
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
		0, FRAME_LIFE); /* paper space */
	
	if (!new_el) lua_pushnil(L); /* return fail */
	else lua_pushlightuserdata(L, (void *) new_el); /* return success */
	return 1;
}

int script_set_layer (lua_State *L) {
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
		lua_pushliteral(L, "set_layer: invalid number of arguments");
		lua_error(L);
	}
	
	if (lua_type(L, 1) == LUA_TNUMBER) {
		int idx = lua_tonumber(L, 1);
		int num_layers = gui->drawing->num_layers;
		if (idx >= num_layers && idx < 0){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		/* change current layer*/
		gui->layer_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char *name = (char *) lua_tostring(L, 1);
		int idx = dxf_lay_idx (gui->drawing, name);
		/* change current layer*/
		gui->layer_idx = idx;
	}
	else {
		lua_pushliteral(L, "set_layer: incorrect argument type");
		lua_error(L);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_set_color (lua_State *L) {
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
		lua_pushliteral(L, "set_color: invalid number of arguments");
		lua_error(L);
	}
	
	if (lua_isnumber(L, 1)) {
		int idx = lua_tonumber(L, 1);
		if (idx >= 257 && idx < 0){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		/* change current color*/
		gui->color_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char name[DXF_MAX_CHARS];
		strncpy(name, lua_tostring(L, 1), DXF_MAX_CHARS - 1);
		str_upp(name);
		char *new_name = trimwhitespace(name);
		
		if (strcmp(new_name, "BY BLOCK") == 0){
			/* change current color*/
			gui->color_idx = 0;
		}
		else if (strcmp(new_name, "BY LAYER") == 0){
			/* change current color*/
			gui->color_idx = 256;
		}
		else {
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		
	}
	else {
		lua_pushliteral(L, "set_color: incorrect argument type");
		lua_error(L);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_set_ltype (lua_State *L) {
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
		lua_pushliteral(L, "set_ltype: invalid number of arguments");
		lua_error(L);
	}
	
	if (lua_type(L, 1) == LUA_TNUMBER) {
		int idx = lua_tonumber(L, 1);
		int num_ltypes = gui->drawing->num_ltypes;
		if (idx >= num_ltypes && idx < 0){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		/* change current layer*/
		gui->ltypes_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char *name = (char *) lua_tostring(L, 1);
		int idx = dxf_ltype_idx (gui->drawing, name);
		/* change current layer*/
		gui->ltypes_idx = idx;
	}
	else {
		lua_pushliteral(L, "set_ltype: incorrect argument type");
		lua_error(L);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_set_style (lua_State *L) {
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
		lua_pushliteral(L, "set_style: invalid number of arguments");
		lua_error(L);
	}
	
	if (lua_type(L, 1) == LUA_TNUMBER) {
		int idx = lua_tonumber(L, 1);
		int num_tstyles = gui->drawing->num_tstyles;
		if (idx >= num_tstyles && idx < 0){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		/* change current layer*/
		gui->t_sty_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char *name = (char *) lua_tostring(L, 1);
		int idx = dxf_tstyle_idx (gui->drawing, name);
		/* change current layer*/
		gui->t_sty_idx = idx;
	}
	else {
		lua_pushliteral(L, "set_style: incorrect argument type");
		lua_error(L);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_set_lw (lua_State *L) {
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
		lua_pushliteral(L, "set_lw: invalid number of arguments");
		lua_error(L);
	}
	
	if (lua_isnumber(L, 1)) {
		int idx = lua_tonumber(L, 1);
		if (idx >= DXF_LW_LEN && idx < 0){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		/* change current color*/
		gui->lw_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char name[DXF_MAX_CHARS];
		strncpy(name, lua_tostring(L, 1), DXF_MAX_CHARS - 1);
		str_upp(name);
		char *new_name = trimwhitespace(name);
		
		if (strcmp(new_name, "BY BLOCK") == 0){
			/* change current color*/
			gui->lw_idx = DXF_LW_LEN + 1;
		}
		else if (strcmp(new_name, "BY LAYER") == 0){
			/* change current color*/
			gui->lw_idx = DXF_LW_LEN;
		}
		else {
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		
	}
	else {
		lua_pushliteral(L, "set_lw: incorrect argument type");
		lua_error(L);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_start_dynamic (lua_State *L) {
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
		lua_pushliteral(L, "start_dynamic: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "start_dynamic: incorrect argument type");
		lua_error(L);
	}
	
	const char *name = lua_tostring(L, 1);
	if (strlen(name) <= 0){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	gui->lua_script.dynamic = 1;
	gui->modal = SCRIPT;
	gui_first_step(gui);
	strncpy(gui->script_dynamic, name, DXF_MAX_CHARS - 1);
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_stop_dynamic (lua_State *L) {
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
	
	gui->lua_script.dynamic = 0;
	gui->script_dynamic[0] = 0;
	gui_default_modal(gui);
	
	return 0;
}

int script_ent_draw (lua_State *L) {
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
		lua_pushliteral(L, "ent_draw: incorrect argument type");
		lua_error(L);
	}
	
	dxf_node *ent = (dxf_node *) lua_touserdata (L, 1);
	if (ent) {
		list_node *vec_graph = dxf_graph_parse(gui->drawing, ent, 0, FRAME_LIFE);
		if (!vec_graph){
			lua_pushboolean(L, 0); /* return fail */
			return 1;
		}
		
		if (!gui->phanton)
			gui->phanton = vec_graph;
		else
			list_merge(gui->phanton, vec_graph);
		
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	
	lua_pushboolean(L, 0); /* return fail */
	return 1;
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
	if (n < 6){
		lua_pushliteral(L, "win_show: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 2; i++) {
		if (!lua_isstring(L, i)) {
			lua_pushliteral(L, "win_show: incorrect argument type");
			lua_error(L);
		}
	}
	
	for (i = 3; i <= 6; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "win_show: incorrect argument type");
			lua_error(L);
		}
	}
	
	const char *name = lua_tostring(L, 1);
	if (strlen(name) <= 0){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	gui->lua_script.active = 1;
	gui->modal = SCRIPT;
	gui_first_step(gui);
	strncpy(gui->script_win, name, DXF_MAX_CHARS - 1);
	
	strncpy(gui->script_win_title, lua_tostring(L, 2), DXF_MAX_CHARS - 1);
	gui->script_win_x = lua_tonumber(L, 3);
	gui->script_win_y = lua_tonumber(L, 4);
	gui->script_win_w = lua_tonumber(L, 5);
	gui->script_win_h = lua_tonumber(L, 6);
	
	lua_pushboolean(L, 1); /* return success */
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
	gui_default_modal(gui);
	
	return 0;
}

int script_nk_layout (lua_State *L) {
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
	if (n < 2){
		lua_pushliteral(L, "nk_layout: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 2; i++) {
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "nk_layout: incorrect argument type");
			lua_error(L);
		}
	}
	
	nk_layout_row_dynamic(gui->ctx, lua_tonumber(L, 1), lua_tonumber(L, 2));
	
	return 0;
}

int script_nk_button (lua_State *L) {
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
		lua_pushliteral(L, "nk_button: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_button: incorrect argument type");
		lua_error(L);
	}
	
	int ret = nk_button_label(gui->ctx, lua_tostring(L, 1));
	
	if (ret) lua_pushboolean(L, 1); /* return success */
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

int script_nk_label (lua_State *L) {
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
		lua_pushliteral(L, "nk_label: invalid number of arguments");
		lua_error(L);
	}
	
	/*
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_label: incorrect argument type");
		lua_error(L);
	}
	*/
	
	nk_label(gui->ctx, lua_tostring(L, 1), NK_TEXT_LEFT);
	return 0;
}

int script_nk_edit (lua_State *L) {
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
		lua_pushliteral(L, "nk_edit: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) {
		lua_pushliteral(L, "nk_edit: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 1, "value");
	if (!lua_isstring(L, -1)){
		lua_pushliteral(L, "nk_edit: incorrect argument type");
		lua_error(L);
	}
	const char *value = lua_tostring(L, -1);
	lua_pop(L, 1);
	char buff[DXF_MAX_CHARS];
	strncpy(buff, value, DXF_MAX_CHARS);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, buff, DXF_MAX_CHARS, nk_filter_default);
	lua_pushstring(L, buff);
	lua_setfield(L, -2, "value");
	lua_pop(L, 1);
	
	return 0;
}