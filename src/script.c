#include "script.h"

/* for debug purposes - print a Lua variable in given buffer string (idx is its position on Lua stack)*/
static int print_lua_var(char * value, lua_State * L, int idx){
	int type = lua_type(L, idx); /*get  Lua type*/

	switch(type) { /*print according type */
		case LUA_TSTRING: {
			snprintf(value, DXF_MAX_CHARS - 1, "s: %s", lua_tostring(L, idx));
			break;
		}
		case LUA_TNUMBER: {
		/* LUA_NUMBER may be double or integer */
			snprintf(value, DXF_MAX_CHARS - 1, "n: %.9g", lua_tonumber(L, idx));
			break;
		}
		case LUA_TTABLE: {
			snprintf(value, DXF_MAX_CHARS - 1, "t: 0x%08x", lua_topointer(L, idx));
			break;
		}
		case LUA_TFUNCTION: {
			snprintf(value, DXF_MAX_CHARS - 1, "f: 0x%08x", lua_topointer(L, idx));
			break;		}
		case LUA_TUSERDATA: {
			snprintf(value, DXF_MAX_CHARS - 1, "u: 0x%08x", lua_touserdata(L, idx));
			break;
		}
		case LUA_TLIGHTUSERDATA: {
			snprintf(value, DXF_MAX_CHARS - 1, "U: 0x%08x", lua_touserdata(L, idx));
			break;
		}
		case LUA_TBOOLEAN: {
			snprintf(value, DXF_MAX_CHARS - 1, "b: %d", lua_toboolean(L, idx) ? 1 : 0);
			break;
		}
		case LUA_TTHREAD: {
			snprintf(value, DXF_MAX_CHARS - 1, "d: 0x%08x", lua_topointer(L, idx));
			break;
		}
		case LUA_TNIL: {
			snprintf(value, DXF_MAX_CHARS - 1, "nil");
			break;
		}
	}
}

/* for debug purposes - print to stdout the entire Lua stack */
void print_lua_stack(lua_State * L){
	int n = lua_gettop(L);    /* number of arguments in stack*/
	int i;
	char value[DXF_MAX_CHARS];
	
	for (i = 1; i <= n; i++) {
		print_lua_var(value, L, i);
		printf("%d=%s\n", i, value);
	}
}


/* --------Lua functions------- */

/* set timeout variable */
/* given parameters:
	- time in seconds, as number
returns:
	- success, as boolean
*/
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

/* get selected entities in drawing */
/* given parameters:
	- none
returns:
	- a table (array) with selected entities
*/
int script_get_sel (lua_State *L) {
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
	int i = 1;
	lua_newtable(L);
	
	struct ent_lua *ent = NULL;
	/* sweep the selection list */
	list_node *current = gui->sel_list->next;
	while (current != NULL){
		if (current->data){
			if (((dxf_node *)current->data)->type == DXF_ENT){ /* DXF entity */
				ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
				ent->curr_ent = NULL;
				ent->orig_ent = (dxf_node *) current->data;
				ent->sel = 1;
				luaL_getmetatable(L, "cz_ent_obj");
				lua_setmetatable(L, -2);
				
				//lua_pushlightuserdata(L, current->data);  /* value */
				lua_rawseti(L, -2, i);  /* set table at key `i' */
				i++;
			}
		}
		current = current->next;
	}
	
	//lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a DXF entity in current drawing */
/* given parameters:
	- DXF entity, as userdata
returns:
	- success, as boolean
*/
int script_ent_write (lua_State *L) {
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
	
	struct ent_lua *ent_obj;
	
	/* verify passed arguments */
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "write: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	//if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* copy the entity to drawing's compatible memory pool*/
	dxf_node *new_ent = dxf_ent_copy(ent, DWG_LIFE);
	
	if (!new_ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* parse entity to graphics */
	new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 ,DWG_LIFE);
	/*append to drawing */
	if (ent_obj->orig_ent) dxf_obj_subst(ent_obj->orig_ent, new_ent);
	else drawing_ent_append(gui->drawing, new_ent);
	/* add to undo/redo list*/
	do_add_item(gui->list_do.current, ent_obj->orig_ent, new_ent);
	
	if(ent_obj->sel){
		list_node * list_el = NULL;
		if (ent_obj->orig_ent) {
			if (list_el = list_find_data(gui->sel_list, ent_obj->orig_ent)){ /* if data is present in list */
				list_el->data = new_ent;
			}
		}
		else {
			/* add to list */
			list_el = list_new(new_ent, SEL_LIFE);
			if (list_el){
				list_push(gui->sel_list, list_el);
			}
		}
	}
	ent_obj->curr_ent = NULL;
	ent_obj->orig_ent = new_ent;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* ========= fuctions to get informations from entities =========== */

/* get the number  of  ATTRIB in a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
returns:
	- success, number of ATTRIB
	- nil if not a INSERT
*/
int script_count_attrib (lua_State *L) {
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
	
	struct ent_lua *ent_obj;
	
	/* verify passed arguments */
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "count_attrib: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* verify if it is a INSERT ent */
	if (strcmp(ent->obj.name, "INSERT") != 0) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* count ATTRIB DXF entities inside INSERTs*/
	int num_attr = 0;
	dxf_node *attr = NULL, *nxt_attr = NULL;
	
	/* sweep INSERT looking for ATTRIBs */
	num_attr = 0;
	while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
		num_attr++; /* current index */
		
		if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
	}
	
	lua_pushinteger(L, num_attr);
	return 1;
}

/* get data (tag and value)  of  a ATTRIB in a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
	- ATTRIB index, as number (integer)
returns:
	- success, tag and value as strings, hidden flag as boolean
	- nil if not a INSERT or a invalid index
*/
int script_get_attrib_i (lua_State *L) {
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
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "get_attrib_i: invalid number of arguments");
		lua_error(L);
	}
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "get_attrib_i: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "get_attrib_i : incorrect argument type");
		lua_error(L);
	}
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* verify if it is a INSERT ent */
	if (strcmp(ent->obj.name, "INSERT") != 0) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	int idx = lua_tointeger(L, 2);
	
	/* count ATTRIB DXF entities inside INSERTs*/
	int num_attr = 0;
	dxf_node *attr = NULL, *nxt_attr = NULL;
	
	/* sweep INSERT looking for ATTRIBs */
	num_attr = 0;
	while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
		num_attr++; /* current index */
		
		if (num_attr == idx){
			dxf_node *tmp = NULL;
			int hidden = 0;
			char tag[DXF_MAX_CHARS+1] = "";
			char value[DXF_MAX_CHARS+1] = "";
			/* verify if it is a hidden attribute */
			if(tmp = dxf_find_attr2(attr, 70))
				hidden = tmp->value.i_data & 1;
			if(tmp = dxf_find_attr2(attr, 2))
				strncpy(tag, tmp->value.s_data, DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(attr, 1))
				strncpy(value, tmp->value.s_data, DXF_MAX_CHARS);
			
			lua_pushstring (L, tag);
			lua_pushstring (L, value);
			lua_pushboolean(L, hidden); 
			return 3; /* number of returned parrameters */
		}
		
		if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
	}
	
	lua_pushnil(L); /* return fail */
	return 1;
}

/* ========= entity creation functions =========== */

#if(0)
/* append a DXF entity to current drawing */
/* given parameters:
	- DXF entity, as userdata
returns:
	- success, as boolean
*/
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
	
	/* the entity is a pointer stored in Lua as userdata type*/
	if (!lua_isuserdata(L, 1)) { /* verify passed arguments */
		lua_pushliteral(L, "ent_append: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = (dxf_node *) lua_touserdata (L, 1);
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* copy the entity to drawing's compatible memory pool*/
	dxf_node *new_ent = dxf_ent_copy(ent, DWG_LIFE);
	
	if (!new_ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* parse entity to graphics */
	new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0 ,DWG_LIFE);
	/*append to drawing */
	drawing_ent_append(gui->drawing, new_ent);
	/* add to undo/redo list*/
	do_add_item(gui->list_do.current, NULL, new_ent);
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}
#endif

/* create a LINE DXF entity */
/* given parameters:
	- first vertex x, y and z, as numbers
	- second vertex x, y and z, as numbers
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
int script_new_line (lua_State *L) {
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
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 6){
		lua_pushliteral(L, "new_line: invalid number of arguments");
		lua_error(L);
	}
	int i;
	for (i = 1; i <= 6; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_line: incorrect argument type");
			lua_error(L);
		}
	}
	
	/* new LINE entity */
	dxf_node * new_el = (dxf_node *) dxf_new_line (
		lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), /* first point */
		lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), /* end point */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, FRAME_LIFE); /* paper space */
	
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* return success */
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	luaL_getmetatable(L, "cz_ent_obj");
	lua_setmetatable(L, -2);
	return 1;
}

/* create a LWPOLYLINE DXF entity */
/* given parameters:
	- first vertex x, y and bulge, as numbers
	- second vertex x, y and bulge, as numbers
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
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
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 6){
		lua_pushliteral(L, "new_pline: invalid number of arguments");
		lua_error(L);
	}
	int i;
	for (i = 1; i <= 6; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_pline: incorrect argument type");
			lua_error(L);
		}
	}
	
	/* new LWPOLYLINE entity */
	dxf_node *new_el = (dxf_node *) dxf_new_lwpolyline (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, /* pt1, */
		lua_tonumber(L, 3), /* bulge */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, FRAME_LIFE); /* paper space */
	/* append second vertex to ensure entity's validity */
	dxf_lwpoly_append (new_el, lua_tonumber(L, 4), lua_tonumber(L, 5), 0.0, lua_tonumber(L, 6), FRAME_LIFE);
	
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* return success */
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	luaL_getmetatable(L, "cz_ent_obj");
	lua_setmetatable(L, -2);
	return 1;
}

/* append a vertex to LWPOLYLINE DXF entity */
/* given parameters:
	- vertex x, y and bulge, as numbers
returns:
	- success, as boolean
*/
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
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 4){
		lua_pushliteral(L, "pline_append: invalid number of arguments");
		lua_error(L);
	}
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "pline_append: incorrect argument type");
		lua_error(L);
	}
	int i;
	for (i = 2; i <= 4; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "pline_append: incorrect argument type");
			lua_error(L);
		}
	}
	
	/* get polyline */
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
	}
	dxf_node *new_el = ent_obj->curr_ent;  /*try to get current entity */
	
	if (new_el) {
		/* append vertex */
		dxf_lwpoly_append (new_el, lua_tonumber(L, 2), lua_tonumber(L, 3), 0.0, lua_tonumber(L, 4), FRAME_LIFE);
		lua_pushboolean(L, 1); /* return success */
	}
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

/* change closed flag of a LWPOLYLINE DXF entity */
/* given parameters:
	- closed flag, as boolean
returns:
	- success, as boolean
*/
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "pline_close: invalid number of arguments");
		lua_error(L);
	}
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "pline_close: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isboolean(L, 2)) { /* arguments types */
		lua_pushliteral(L, "pline_close: incorrect argument type");
		lua_error(L);
	}
	/* get polyline */
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
	}
	dxf_node *new_el = ent_obj->curr_ent;  /*try to get current entity */
	if (new_el) {
		/* change flag */
		int closed = lua_toboolean(L, 2);
		lua_pushboolean(L, dxf_attr_change_i(new_el, 70, (void *) &closed, 0)); /* return success */
	}
	else lua_pushboolean(L, 0); /* return fail */
	return 1;
}

/* create a CIRCLE DXF entity */
/* given parameters:
	- center x, y, as numbers
	- radius, as number
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "new_circle: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 3; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_circle: incorrect argument type");
			lua_error(L);
		}
	}
	/* new CIRCLE entity */
	dxf_node * new_el = (dxf_node *) dxf_new_circle (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, lua_tonumber(L, 3), /* pt1, radius */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, FRAME_LIFE); /* paper space */
	
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* return success */
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	luaL_getmetatable(L, "cz_ent_obj");
	lua_setmetatable(L, -2);
	return 1;
}

/* create a HATCH DXF entity */
/* given parameters:
	- boundary vertexes, as table (elements in table are tables too,
		with "x" and "y" labeled numbers elements)
	- hatch type, as string ( acceptable values = USER, SOLID, PREDEF)
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
int script_new_hatch (lua_State *L) {
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
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "new_hatch: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) { /* arguments types */
		lua_pushliteral(L, "new_hatch: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) { /* arguments types */
		lua_pushliteral(L, "new_hatch: incorrect argument type");
		lua_error(L);
	}
	
	/* -----------create boundary as graph vector -------------- */
	graph_obj *bound = graph_new(FRAME_LIFE);
	
	if (!bound){
		lua_pushliteral(L, "new_hatch: internal error");
		lua_error(L);
	}
	
	int i = 0;
	double x0, y0, x1, y1, x2, y2;
	double rot = 0.0, scale = 1.0;
	int solid = 0;
	struct h_pattern *curr_h;
	struct h_family *curr_fam = gui->hatch_fam.next;
	
	/* iterate over table */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, 1) != 0) { /* table index are shifted*/
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if (lua_istable(L, -1)){
			/* get vertex coordinates */
			lua_getfield(L, -1, "x");
			x2 = lua_tonumber(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "y");
			y2 = lua_tonumber(L, -1);
			lua_pop(L, 1);
			if (i > 0) /* add line bondary */
				line_add(bound, x1, y1, 0, x2, y2, 0);
			else { /* first vertex */
				x0 = x2;
				y0 = y2;
			}
			/* prepare for next vertex */
			x1 = x2;
			y1 = y2;
			i++;
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	/* close polygon, if not yet */
	if (fabs(x0 - x1) > 1e-9 || fabs(y0 - y1) > 1e-9)
		line_add(bound, x1, y1, 0, x0, y0, 0);
	/*-----------------------------------------------------*/
	
	/* ----------------switch the hatch type options --------------*/
	char type[DXF_MAX_CHARS];
	strncpy(type, lua_tostring(L, 2), DXF_MAX_CHARS - 1);
	str_upp(type);
	char *new_type = trimwhitespace(type);
	
	if (strcmp(new_type, "USER") == 0){
		strncpy(gui->list_pattern.name, "USER_DEF", DXF_MAX_CHARS - 1);
		curr_h = &(gui->list_pattern);
		rot = 0.0;
		scale = 1.0;
	}
	else if (strcmp(new_type, "SOLID") == 0){
		strncpy(gui->list_pattern.name, "SOLID", DXF_MAX_CHARS - 1);
		curr_h = &(gui->list_pattern);
		rot = 0.0;
		scale = 1.0;
		solid = 1;
	}
	else if (strcmp(new_type, "PREDEF") == 0){
		/* get current family */
		curr_h = NULL;
		i = 0;
		while (curr_fam){
			if (gui->hatch_fam_idx == i){
				curr_h = curr_fam->list->next;
				break;
			}
			
			i++;
			curr_fam = curr_fam->next;
		}
		
		/* get current hatch pattern */
		i = 0;
		while ((curr_h) && (i < gui->hatch_idx)){
			i++;
			curr_h = curr_h->next;
		}
		/* optional rotation and scale */
		rot = gui->patt_ang;
		scale = gui->patt_scale;
	}
	else { /* invalid type */
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	/*--------------------------------*/
	
	
	/* make DXF HATCH entity */
	dxf_node *new_el = dxf_new_hatch (curr_h, bound,
	solid, gui->hatch_assoc,
	0, 0, /* style, type */
	rot, scale,
	gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
	gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
	0, FRAME_LIFE); /* paper space */
	
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* return success */
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	luaL_getmetatable(L, "cz_ent_obj");
	lua_setmetatable(L, -2);
	return 1;
}

/* create a TEXT DXF entity */
/* given parameters:
	- insertion point x, y, as numbers
	- text, as string
	- height, as number - OPTIONAL
	- horizontal alingment, as string - OPTIONAL
	- vertical alingment, as string - OPTIONAL
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
int script_new_text (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "new_text: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	for (i = 1; i <= 2; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_text: incorrect argument type");
			lua_error(L);
		}
	}
	if (!lua_isstring(L, 3)) { /* arguments types */
		lua_pushliteral(L, "new_text: incorrect argument type");
		lua_error(L);
	}
	
	/* get text */
	char txt[DXF_MAX_CHARS];
	txt[0] = 0;
	strncpy(txt, lua_tostring(L, 3), DXF_MAX_CHARS - 1);
	if (strlen(txt) <= 0) { /* invalid text */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* get height, if exist*/
	double txt_h = 1;
	if (lua_isnumber(L, 4)) {
		txt_h = lua_tonumber(L, 4);
	}
	
	int t_al_h = 0, t_al_v = 0;
	const char *al_v[] = {"BASE LINE", "BOTTOM", "MIDDLE", "TOP"};
	const char *al_h[] = {"LEFT", "CENTER", "RIGHT", "ALIGNED", "MIDDLE", "FIT"};
	
	/* get horizontal aligment, if exist*/
	if (lua_isstring(L, 5)) {
		char al[DXF_MAX_CHARS];
		strncpy(al, lua_tostring(L, 5), DXF_MAX_CHARS - 1);
		str_upp(al);
		char *new_al = trimwhitespace(al);
		
		for (i = 0; i < 6; i++){
			if (strcmp(al, al_h[i]) == 0) t_al_h = i;
		}
	}
	
	/* get vertical aligment, if exist*/
	if (lua_isstring(L, 6)) {
		char al[DXF_MAX_CHARS];
		strncpy(al, lua_tostring(L, 6), DXF_MAX_CHARS - 1);
		str_upp(al);
		char *new_al = trimwhitespace(al);
		
		for (i = 0; i < 4; i++){
			if (strcmp(al, al_v[i]) == 0) t_al_v = i;
		}
	}
	
	/* create a new DXF TEXT */
	dxf_node * new_el = (dxf_node *) dxf_new_text (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, txt_h, /* pt1, height */
		txt, /* text, */
		gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
		gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
		0, DWG_LIFE); /* paper space */
	dxf_attr_change_i(new_el, 72, &t_al_h, -1);
	dxf_attr_change_i(new_el, 73, &t_al_v, -1);
	dxf_attr_change(new_el, 7, gui->drawing->text_styles[gui->t_sty_idx].name);
	
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* return success */
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdata(L, sizeof(struct ent_lua));  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	luaL_getmetatable(L, "cz_ent_obj");
	lua_setmetatable(L, -2);
	return 1;
}

/* create a BLOCK */
/* given parameters:
	- entities, as table of userdata
	- block name, as string
returns:
	- block entry, as userdata
Notes:
	- This function will append the created block to drawing's Block section.
	No actions are requiried after this.
*/
int script_new_block (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "new_block: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) { /* arguments types */
		lua_pushliteral(L, "new_block: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) { /* arguments types */
		lua_pushliteral(L, "new_block: incorrect argument type");
		lua_error(L);
	}
	
	/* verify if table is empty */
	if (lua_rawlen(L, 1) <=0){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* get name */
	char name[DXF_MAX_CHARS];
	name[0] = 0;
	strncpy(name, lua_tostring(L, 2), DXF_MAX_CHARS - 1);
	if (strlen(name) <= 0) { /* invalid text */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_drawing *drawing = gui->drawing;
	
	/* verify if block not exist */
	if (dxf_find_obj_descr2(drawing->blks, "BLOCK", name) != NULL){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_node *blkrec = NULL, *blk = NULL, *endblk = NULL, *handle = NULL;
	dxf_node *obj, *new_ent, *text, *attdef;
	//list_node *vec_graph = NULL;
	double max_x = 0.0, max_y = 0.0;
	double min_x = 0.0, min_y = 0.0;
	int init_ext = 0, ok = 0;
	char txt[DXF_MAX_CHARS], tag[DXF_MAX_CHARS];
	txt[0] = 0;
	tag[0] = 0;
	
	//int mark_len = strlen(mark);
	
	/* create BLOCK_RECORD table entry*/
	blkrec = dxf_new_blkrec (name, DWG_LIFE);
	ok = ent_handle(drawing, blkrec);
	if (ok) handle = dxf_find_attr2(blkrec, 5); ok = 0;
	
	/* begin block */
	if (handle) blk = dxf_new_begblk (name, gui->drawing->layers[gui->layer_idx].name, (char *)handle->value.s_data, DWG_LIFE);
	/* get a handle */
	ok = ent_handle(drawing, blk);
	/* use the handle to owning the ENDBLK ent */
	if (ok) handle = dxf_find_attr2(blk, 5); ok = 0;
	if (handle) endblk = dxf_new_endblk (gui->drawing->layers[gui->layer_idx].name, (char *)handle->value.s_data, DWG_LIFE);
	else {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	struct ent_lua *ent_obj;
	
	/* first get the list coordinates extention */
	/* iterate over table */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, 1) != 0) { /* table index are shifted*/
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if ( ent_obj =  luaL_checkudata(L, -1, "cz_ent_obj") ){
			/* get entity */
			obj = ent_obj->curr_ent;  /*try to get current entity */
			if (!obj) obj = ent_obj->orig_ent; /* if not current, try original entity */
			
			list_node *graphics = dxf_graph_parse(gui->drawing, obj, 0, FRAME_LIFE);
			graph_list_ext(graphics, &init_ext, &min_x, &min_y, &max_x, &max_y);
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	/*then copy the entities of list and apply offset in their coordinates*/
	/* iterate over table */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, 1) != 0) { /* table index are shifted*/
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if ( ent_obj =  luaL_checkudata(L, -1, "cz_ent_obj") ){
			/* get entity */
			obj = ent_obj->curr_ent;  /*try to get current entity */
			if (!obj) obj = ent_obj->orig_ent; /* if not current, try original entity */
			if (obj->type == DXF_ENT){ /* DXF entity  */
				
				new_ent = dxf_ent_copy(obj, DWG_LIFE);
				ent_handle(drawing, new_ent);
				dxf_edit_move(new_ent, -min_x, -min_y, 0.0);
				dxf_obj_append(blk, new_ent);
			}
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	/* end the block*/
	if (endblk) ok = ent_handle(drawing, endblk);
	if (ok) ok = dxf_obj_append(blk, endblk);
	
	/*attach to blocks section*/
	if (ok) ok = dxf_obj_append(drawing->blks_rec, blkrec);
	if (ok) ok = dxf_obj_append(drawing->blks, blk);
	
	if (ok) {
		/* undo/redo list*/
		do_add_item(gui->list_do.current, NULL, blkrec);
		do_add_item(gui->list_do.current, NULL, blk);
		lua_pushlightuserdata(L, (void *) blk); /* return success */
	}
	else lua_pushnil(L); /* return fail */
	return 1;
}


/* ========= set drawing's global parameters =========== */

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


/* ========= dynamic mode functions =========== */

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
	
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "ent_draw: incorrect argument type");
		lua_error(L);
	}
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
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

/* ========= gui functions =========== */

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