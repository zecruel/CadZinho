#include "script.h"
#include "miniz.h"
#include "yxml.h"
#include "gui_script.h"

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
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if script is valid */
	if (!script){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* get passed value */
	int isnum = 0;
	double timeout = lua_tonumberx (L, -1, &isnum);
	/* verify if a valid number */
	if (isnum){
		script->timeout = timeout; /* set timeout value */
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}

/* equivalent to a "print" lua function, that outputs to a text edit widget */
int debug_print (lua_State *L) {
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
	
	char msg[DXF_MAX_CHARS];
	int n = lua_gettop(L);    /* number of arguments */
	int i;
	int type;
	
	for (i = 1; i <= n; i++) {
		type = lua_type(L, i); /* identify Lua variable type */
		
		/* print variables separator (4 spaces) */
		if (i > 1) nk_str_append_str_char(&gui->debug_edit.string, "    ");
		
		switch(type) {
			case LUA_TSTRING: {
				snprintf(msg, DXF_MAX_CHARS - 1, "%s", lua_tostring(L, i));
				break;
			}
			case LUA_TNUMBER: {
			/* LUA_NUMBER may be double or integer */
				snprintf(msg, DXF_MAX_CHARS - 1, "%.9g", lua_tonumber(L, i));
				break;
			}
			case LUA_TTABLE: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
				break;
			}
			case LUA_TFUNCTION: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
				break;		}
			case LUA_TUSERDATA: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_touserdata(L, i));
				break;
			}
			case LUA_TLIGHTUSERDATA: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_touserdata(L, i));
				break;
			}
			case LUA_TBOOLEAN: {
				snprintf(msg, DXF_MAX_CHARS - 1, "%s", lua_toboolean(L, i) ? "true" : "false");
				break;
			}
			case LUA_TTHREAD: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
				break;
			}
			case LUA_TNIL: {
				snprintf(msg, DXF_MAX_CHARS - 1, "nil");
				break;
			}
		}
		nk_str_append_str_char(&gui->debug_edit.string, msg);
	}
	/*enter a new line*/
	nk_str_append_str_char(&gui->debug_edit.string, "\n");
	
	lua_pushboolean(L, 1); /* return success */
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
				ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
				ent->curr_ent = NULL;
				ent->orig_ent = (dxf_node *) current->data;
				
				ent->drawing = gui->drawing;
				
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
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
	new_ent->obj.graphics = dxf_graph_parse(ent_obj->drawing, new_ent, 0 ,DWG_LIFE);
	/*append to drawing */
	if (ent_obj->orig_ent) dxf_obj_subst(ent_obj->orig_ent, new_ent);
	else drawing_ent_append(ent_obj->drawing, new_ent);
	
	/* add to undo/redo list*/
	if (!script->do_init){
		char msg[DXF_MAX_CHARS];
		strncpy(msg, "SCRIPT:", DXF_MAX_CHARS - 1);
		lua_Debug ar;
		lua_getstack (L, 1, &ar);
		lua_getinfo(L, "S", &ar); /* get script file name */
		if (strlen (get_filename(ar.short_src)) > 0)
			strncat(msg, get_filename(ar.short_src), DXF_MAX_CHARS - 8);
		else
			strncat(msg, get_filename(script->path), DXF_MAX_CHARS - 8);
		do_add_entry(&gui->list_do, msg);
		script->do_init = 1;
	}
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

/* get type of an entity */
/* given parameters:
	- DXF entity, as userdata
returns:
	- success, DXF entity type as string (upper case)
	- nil if not a entity
*/
int script_get_ent_typ (lua_State *L) {
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
		lua_pushliteral(L, "get_ent_typ: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	if (ent->type != DXF_ENT){ /* not a DXF entity */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* success - return type of entity */
	lua_pushstring (L, ent->obj.name);
	return 1;
}

/* get the block name in a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
returns:
	- success, block name
	- nil if not a INSERT
*/
int script_get_blk_name (lua_State *L) {
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
		lua_pushliteral(L, "get_blk_name: incorrect argument type");
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
	if (!( (strcmp(ent->obj.name, "INSERT") == 0) || (strcmp(ent->obj.name, "DIMENSION") == 0) )) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_node *tmp = NULL;
	if(tmp = dxf_find_attr2(ent, 2)) /* look for block name */
		lua_pushstring (L, tmp->value.s_data);
	else lua_pushnil(L); /* return fail */
	return 1;
}

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

/* get data (tag, value and hidden flag)  of  a ATTRIB in a INSERT entity */
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
		lua_pushliteral(L, "get_attrib_i: incorrect argument type");
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

/* get data (tag, value and hidden flag)  of  all ATTRIBs in a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
returns:
	- a table, where each element has tag and value as strings, hidden flag as boolean
*/
int script_get_attribs (lua_State *L) {
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
	if (n < 1){
		lua_pushliteral(L, "get_attribs: invalid number of arguments");
		lua_error(L);
	}
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "get_attribs: incorrect argument type");
		lua_error(L);
	}
	
	lua_newtable(L); /* main returned table */
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		return 1;
	}
	
	/* verify if it is a INSERT ent */
	if (strcmp(ent->obj.name, "INSERT") != 0) {
		return 1;
	}
	
	/* count ATTRIB DXF entities inside INSERTs*/
	int num_attr = 0;
	dxf_node *attr = NULL, *nxt_attr = NULL;
	
	/* sweep INSERT looking for ATTRIBs */
	num_attr = 0;
	while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
		num_attr++; /* current index */
		
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
		
		lua_newtable(L); /* table to store attr data */
		lua_pushstring(L, "tag");
		lua_pushstring (L, tag);
		lua_rawset(L, -3);
		
		lua_pushstring(L, "value");
		lua_pushstring (L, value);
		lua_rawset(L, -3);
		
		lua_pushstring(L, "hidden");
		lua_pushboolean(L, hidden); 
		lua_rawset(L, -3);
		
		lua_rawseti(L, -2, num_attr);  /* set table at key `num_attr' */
		
		if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
	}
	
	return 1;
}

/* get points of an entity */
/* given parameters:
	- DXF INSERT entity, as userdata
returns:
	- success, table with coordinates
	- nil if not a entity
*/
int script_get_points (lua_State *L) {
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
	if (ent->type != DXF_ENT){ /* not a DXF entity */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	lua_newtable(L); /*main returned table */
	
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *stop = NULL;
	int pt = 0;
	enum dxf_graph ent_type = DXF_NONE;
	double x = 0.0, y = 0.0, z = 0.0;
	double point[3];
	
	int ellip = 0;
	
	int vert_count = 0;
	
	stop = ent;
	ent_type =  dxf_ident_ent_type (ent);
	
	if ((ent_type != DXF_HATCH) && (ent->obj.content)){
		current = ent->obj.content->next;
		prev = current;
	}
	else if ((ent_type == DXF_HATCH) && (ent->obj.content)){
		current = dxf_find_attr_i(ent, 91, 0);
		if (current){
			current = current->next;
			prev = current;
		}
		dxf_node *end_bond = dxf_find_attr_i(ent, 75, 0);
		if (end_bond) stop = end_bond;
	}
	
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/*
			point[0] = of_x;
			point[1] = of_y;
			point[2] = of_z;
			
			dxf_get_extru(ent, point);
			
			ofs_x = point[0];
			ofs_y = point[1];
			ofs_z = point[2];
			*/
			if (current->obj.content){
				ent_type =  dxf_ident_ent_type (current);
				/* starts the content sweep */
				current = current->obj.content->next;
				
				continue;
			}
		}
		else {
			if (ent_type != DXF_POLYLINE){
				/* get the vertex coordinate set */
				if (current->value.group == 10){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 20))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 30))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			if (ent_type == DXF_HATCH){
				/* hatch bondary path type */
				if (current->value.group == 72){ 
					if (current->value.i_data == 3)
						ellip = 1; /* ellipse */
				}
			}
			if (ent_type == DXF_LINE || ent_type == DXF_TEXT ||
			ent_type == DXF_HATCH || ent_type == DXF_ATTRIB){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_TRACE || ent_type == DXF_SOLID){
				/* get the vertex coordinate set */
				if (current->value.group == 11){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 21))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 31))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 12){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 22))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 32))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
			else if (ent_type == DXF_DIMENSION){
				/* get the vertex coordinate set */
				if (current->value.group == 13){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 23))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 33))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 14){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 24))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 34))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
				
				
				/* get the vertex coordinate set */
				if (current->value.group == 15){ /* x coordinate - start set */
					x = current->value.d_data;
					
					if ((current->next) && /* next should be the y coordinate */
						(current->next->type == DXF_ATTR) &&
						(current->next->value.group == 25))
					{
						current = current->next; /* update position in list */
						y = current->value.d_data;
						pt = 1; /* flag as valid point */
						
						/* get z coordinate - optional */
						z = 0.0;
						if ((current->next) && 
							(current->next->type == DXF_ATTR) &&
							(current->next->value.group == 35))
						{
							current = current->next; /* update position in list */
							z = current->value.d_data;
						}
					}
				}
			}
		}
		if (pt){
			pt = 0;
			/*
			if(vert_count == gui->vert_idx) 
				gui_draw_vert_rect(gui, img, x, y, dxf_colors[225]);
			else gui_draw_vert_rect(gui, img, x, y, dxf_colors[224]);
			*/
			vert_count++;
			
			lua_newtable(L); /* table to store point coordinates */
			lua_pushstring(L, "x");
			lua_pushnumber(L, x);
			lua_rawset(L, -3);
			
			lua_pushstring(L, "y");
			lua_pushnumber(L, y);
			lua_rawset(L, -3);
			
			lua_pushstring(L, "z");
			lua_pushnumber(L, z);
			lua_rawset(L, -3);
			
			lua_rawseti(L, -2, vert_count);  /* set table at key `vert_count' */
		}
		
		if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
			current = NULL;
			break;
		}
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			if ((prev == NULL) || (prev == stop)){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				if(prev == stop){
					current = NULL;
					break;
				}
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	return 1;
}

/* get extended data of an entity */
/* given parameters:
	- DXF entity, as userdata
	- APPID, as string
returns:
	- table with extended data
*/
int script_get_ext (lua_State *L) {
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
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "get_ext: invalid number of arguments");
		lua_error(L);
	}
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "get_ext: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "get_ext: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	
	lua_newtable(L); /* table to store data */
	
	int found = 0, type, num_data = 0;
	dxf_node *current = ent->obj.content->next;
	while (current){
		/* try to find the first entry, by matching the APPID */
		if (!found){
			if (current->type == DXF_ATTR){
				if(current->value.group == 1001){
					strncpy(name, current->value.s_data, DXF_MAX_CHARS); /* preserve original string */
					str_upp(name); /*upper case */
					if(strcmp(name, appid) == 0){
						found = 1; /* appid found */
					}
				}
			}
		}
		else{
			/* after the first entry, look by end */
			if (current->type == DXF_ATTR){
				/* breaks if is found a new APPID entry */
				if(current->value.group == 1001){
					break;
				}
				/* update the end mark */
				num_data++;
				/* identify the type of attrib, according DXF group specification */
				type = dxf_ident_attr_type(current->value.group);
				switch(type) {
					/* change the data */
					case DXF_FLOAT :
						lua_pushnumber(L, current->value.d_data);
						break;
					case DXF_INT :
						lua_pushinteger(L, current->value.i_data);
						break;
					case DXF_STR :
						lua_pushstring(L, current->value.s_data);
				}
				lua_rawseti(L, -2, num_data);  /* set table at key `num_data' */
			}
			/* breaks if is found a entity */
			else break;
		}
		current = current->next;
	}
	return 1;
}

/* get entities in BLOCK */
/* given parameters:
	- block name, as string
returns:
	- a table (array) with entities
*/
int script_get_blk_ents (lua_State *L) {
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
	if (n < 1){
		lua_pushliteral(L, "get_blk_ents: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "get_blk_ents: incorrect argument type");
		lua_error(L);
	}
	
	char blk_name[DXF_MAX_CHARS + 1] = "";
	strncpy(blk_name, lua_tostring(L, 1), DXF_MAX_CHARS); /* preserve original string */
	str_upp(blk_name); /*upper case */
	char *new_str;
	new_str = trimwhitespace(blk_name);
	
	lua_newtable(L); /* main returned table */
	
	dxf_node *block = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", new_str);
	dxf_node *current;
	if(block) { 
		current = block->obj.content;
	}
	int i = 1;
	struct ent_lua *ent = NULL;
	while (current){ /* sweep elements in block */
		if (current->type == DXF_ENT){ /* DXF entity */
			if (strcmp(current->obj.name, "ENDBLK") != 0){ /* skip ENDBLK elements */
				ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
				ent->curr_ent = NULL;
				ent->orig_ent = (dxf_node *) current;
				
				ent->drawing = gui->drawing;
				
				ent->sel = 0;
				
				luaL_getmetatable(L, "cz_ent_obj");
				lua_setmetatable(L, -2);
				lua_rawseti(L, -2, i);  /* set table at key `i' */
				i++;
			}
		}
		
		current = current->next;
	}
	return 1;
}

/* get all entities in drawing */
/* given parameters:
	- only visible, as boolean (optional)
returns:
	- a table (array) with entities
*/
int script_get_all (lua_State *L) {
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
	
	int visible = 0;
	
	/* verify if has passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n > 0){
		if (!lua_isboolean(L, 1)) {
			lua_pushliteral(L, "get_all: incorrect argument type");
			lua_error(L);
		}
		visible = lua_toboolean(L, 1);
	}
	
	int i = 1;
	lua_newtable(L); /* main returned table */
	
	struct ent_lua *ent = NULL;
	
	dxf_drawing *drawing = gui->drawing;
	
	/* sweep the drawing content */
	dxf_node *current = drawing->ents->obj.content->next;
	while (current != NULL){
		if (current->type == DXF_ENT){ /* found a DXF entity  */
			/*verify if entity layer is on and thaw */
			if (  ((!drawing->layers[current->obj.layer].off) &&
				(!drawing->layers[current->obj.layer].frozen) && visible) ||
				!visible )
			{
				ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
				ent->curr_ent = NULL;
				ent->orig_ent = current;
				
				ent->drawing = drawing;
				
				ent->sel = 0;
				luaL_getmetatable(L, "cz_ent_obj");
				lua_setmetatable(L, -2);
				
				lua_rawseti(L, -2, i);  /* set table at key `i' */
				i++;
			}
		}
		current = current->next;
	}
	
	return 1;
}

/* ========= entity modification functions =========== */

/* edit data (tag, value and hidden flag)  of  a ATTRIB in a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
	- ATTRIB index, as number (integer)
	- Tag and value as strings
	- hidden as boolean
returns:
	- success, as boolean
*/
int script_edit_attr (lua_State *L) {
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
	if (n < 5){
		lua_pushliteral(L, "edit_attr: invalid number of arguments");
		lua_error(L);
	}
	struct ent_lua *ent_obj;
	
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "edit_attr: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "edit_attr: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 3)) {
		lua_pushliteral(L, "edit_attr: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 4)) {
		lua_pushliteral(L, "edit_attr: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isboolean(L, 5)) {
		lua_pushliteral(L, "edit_attr: incorrect argument type");
		lua_error(L);
	}
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0);  /* return fail */
		return 1;
	}
	/* verify if it is a INSERT ent */
	if (strcmp(ent->obj.name, "INSERT") != 0) {
		lua_pushboolean(L, 0);  /* return fail */
		return 1;
	}
	
	int idx = lua_tointeger(L, 2) - 1;
	if (idx < 0){
		lua_pushliteral(L, "edit_attr: index is out of range");
		lua_error(L);
	}
	
	/* verify if INSERT ent has idx ATTRIB*/
	dxf_node *attr = dxf_find_obj_i(ent, "ATTRIB", idx);
	if (!attr){ 
		lua_pushboolean(L, 0);  /* return fail */
		return 1;
	}
	
	int hidden = lua_toboolean(L, 5);
	char tag[DXF_MAX_CHARS+1] = "";
	strncpy(tag, lua_tostring(L, 3), DXF_MAX_CHARS);
	char value[DXF_MAX_CHARS+1] = "";
	strncpy(value, lua_tostring(L, 4), DXF_MAX_CHARS);
	
	char *new_str;
	new_str = trimwhitespace(tag);
	/* verify if tags contain spaces */
	if (strchr(new_str, ' ')){
		lua_pushliteral(L, "edit_attr: No spaces allowed in tags");
		lua_error(L);
	}
	
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
		/* update other variables */
		ent = ent_obj->curr_ent; 
		attr = dxf_find_obj_i(ent, "ATTRIB", idx);
		if (!attr){ 
			lua_pushboolean(L, 0);  /* return fail */
			return 1;
		}
	}
	
	/* update tag */
	dxf_attr_change(attr, 2, new_str);
	/* update value */
	new_str = trimwhitespace(value);
	dxf_attr_change(attr, 1, new_str);
	/* update hide flag */
	dxf_attr_change(attr, 70, &hidden);
	
	lua_pushboolean(L, 1); /* return success */
	return 1; /* number of returned parrameters */
}


/* add extended data of an entity */
/* given parameters:
	- DXF entity, as userdata
	- APPID, as string
	- table with extended data
returns:
	- success as boolean
*/
int script_add_ext (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "add_ext: invalid number of arguments");
		lua_error(L);
	}
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "add_ext: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "add_ext: incorrect argument type");
		lua_error(L);
	}
	if (!lua_istable(L, 3)) {
		lua_pushliteral(L, "add_ext: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	char *new_str;
	new_str = trimwhitespace(appid);
	/* verify if  appid name contain spaces */
	if (strchr(new_str, ' ')){
		lua_pushliteral(L, "add_ext: No spaces allowed in APPID");
		lua_error(L);
	}
	/* verify if  appid is registred in drawing */
	if(!dxf_find_obj_descr2(ent_obj->drawing->t_appid, "APPID", new_str)){
		lua_pushliteral(L, "add_ext: APPID not registred");
		lua_error(L);
	}
	
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
		/* update other variables */
		ent = ent_obj->curr_ent; 
	}
	
	int found = 0, type;
	dxf_node *current = ent->obj.content->next;
	dxf_node *last = ent->obj.content;
	while (current){
		if (current->type == DXF_ATTR){
			/* try to find the first entry, by matching the APPID */
			if (!found){
				if(current->value.group == 1001){
					strncpy(name, current->value.s_data, DXF_MAX_CHARS); /* preserve original string */
					str_upp(name); /*upper case */
					if(strcmp(name, appid) == 0){
						found = 1; /* appid found */
					}
				}
			}
			else{
				/* after the first entry, look by end */
				/* breaks if is found a new APPID entry */
				if(current->value.group == 1001){
					break;
				}
			}
			/* update the end mark */
			last = current;
		}
		/* breaks if is found a entity */
		else break;
		
		current = current->next;
	}
	dxf_node *tmp;
	double num;
	if (!found){
		strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
		new_str = trimwhitespace(appid);
		
		if (tmp = dxf_attr_insert_after(last, 1001, new_str, FRAME_LIFE))
			last = tmp;
	}
	
	/* iterate over table */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, 3) != 0) { /* table index are shifted*/
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		
		int type = lua_type(L, -1); /*get  Lua type*/
		if (type == LUA_TSTRING){
			new_str = (char*)lua_tostring(L, -1);
			if (tmp = dxf_attr_insert_after(last, 1000, new_str, FRAME_LIFE))
				last = tmp;
		}
		else if (type == LUA_TNUMBER){
			num = lua_tonumber(L, -1);
			if (tmp = dxf_attr_insert_after(last, 1040, &num, FRAME_LIFE))
				last = tmp;
		}
			
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* edit an extended data of an entity */
/* given parameters:
	- DXF entity, as userdata
	- APPID, as string
	- index, as number (integer)
	- extended data
returns:
	- success as boolean
*/
int script_edit_ext_i (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 4){
		lua_pushliteral(L, "edit_ext_i: invalid number of arguments");
		lua_error(L);
	}
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "edit_ext_i: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "edit_ext_i: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 3)) {
		lua_pushliteral(L, "edit_ext_i: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	char *new_str;
	new_str = trimwhitespace(appid);
	
	int idx = lua_tointeger(L, 3) - 1;
	if (idx < 0){
		lua_pushliteral(L, "edit_ext_i: index is out of range");
		lua_error(L);
	}
	
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
		/* update other variables */
		ent = ent_obj->curr_ent; 
	}
	
	int found = 0, type, num_data = 0;
	dxf_node *current = ent->obj.content->next;
	
	while (current){
		if (current->type == DXF_ATTR){
			/* try to find the first entry, by matching the APPID */
			if (!found){
				if(current->value.group == 1001){
					strncpy(name, current->value.s_data, DXF_MAX_CHARS); /* preserve original string */
					str_upp(name); /*upper case */
					if(strcmp(name, appid) == 0){
						found = 1; /* appid found */
					}
				}
			}
			else{
				/* breaks if is found a new APPID entry */
				if(current->value.group == 1001){
					break;
				}
				/* after the first entry, look by index */
				if (idx == num_data) {
					dxf_node *new_ext;
					double num;
					int type = lua_type(L, 4); /*get  Lua type*/
					if (type == LUA_TSTRING){
						new_str = (char*)lua_tostring(L, 4);
						if (new_ext = dxf_attr_new (1000, DXF_STR, new_str, FRAME_LIFE)){
							dxf_obj_subst(current, new_ext);
							lua_pushboolean(L, 1); /* return success */
							return 1;
						}
					}
					else if (type == LUA_TNUMBER){
						num = lua_tonumber(L, 4);
						if (new_ext = dxf_attr_new (1040, DXF_FLOAT, &num, FRAME_LIFE)){
							dxf_obj_subst(current, new_ext);
							lua_pushboolean(L, 1); /* return success */
							return 1;
						}
					}
					lua_pushboolean(L, 0); /* return fail */
					return 1;
				}
				/* next index */
				num_data++;
			}
		}
		/* breaks if is found a entity */
		else break;
		
		current = current->next;
	}
	
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}

/* delete an extended data of an entity */
/* given parameters:
	- DXF entity, as userdata
	- APPID, as string
	- index, as number (integer)
returns:
	- success as boolean
*/
int script_del_ext_i (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "del_ext_i: invalid number of arguments");
		lua_error(L);
	}
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "del_ext_i: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "del_ext_i: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 3)) {
		lua_pushliteral(L, "del_ext_i: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	char *new_str;
	new_str = trimwhitespace(appid);
	
	int idx = lua_tointeger(L, 3) - 1;
	if (idx < 0){
		lua_pushliteral(L, "del_ext_i: index is out of range");
		lua_error(L);
	}
	
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
		/* update other variables */
		ent = ent_obj->curr_ent; 
	}
	
	int found = 0, num_data = 0;
	dxf_node *current = ent->obj.content->next;
	
	while (current){
		if (current->type == DXF_ATTR){
			/* try to find the first entry, by matching the APPID */
			if (!found){
				if(current->value.group == 1001){
					strncpy(name, current->value.s_data, DXF_MAX_CHARS); /* preserve original string */
					str_upp(name); /*upper case */
					if(strcmp(name, appid) == 0){
						found = 1; /* appid found */
					}
				}
			}
			else{
				/* breaks if is found a new APPID entry */
				if(current->value.group == 1001){
					break;
				}
				/* after the first entry, look by index */
				if (idx == num_data) {
					dxf_obj_subst(current, NULL); /* remove extended data*/
					lua_pushboolean(L, 1); /* return success */
					return 1;
				}
				
				/* next index */
				num_data++;
			}
		}
		/* breaks if is found a entity */
		else break;
		
		current = current->next;
	}
	
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}

/* delete all extended data of an entity */
/* given parameters:
	- DXF entity, as userdata
	- APPID, as string
returns:
	- success as boolean
*/
int script_del_ext_all (lua_State *L) {
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
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "edit_ext_all: invalid number of arguments");
		lua_error(L);
	}
	if (!( ent_obj =  luaL_checkudata(L, 1, "cz_ent_obj") )) { /* the entity is a Lua userdata type*/
		lua_pushliteral(L, "del_ext_all: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "del_ext_all: incorrect argument type");
		lua_error(L);
	}
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 2), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	char *new_str;
	new_str = trimwhitespace(appid);
	
	if(!ent_obj->curr_ent && ent_obj->orig_ent){
		/* copy the original entity to temporary memory pool*/
		ent_obj->curr_ent = dxf_ent_copy(ent_obj->orig_ent, FRAME_LIFE);
		/* update other variables */
		ent = ent_obj->curr_ent; 
	}
	
	int found = 0;
	dxf_node *current = ent->obj.content->next;
	
	while (current){
		if (current->type == DXF_ATTR){
			/* try to find the first entry, by matching the APPID */
			if (!found){
				if(current->value.group == 1001){
					strncpy(name, current->value.s_data, DXF_MAX_CHARS); /* preserve original string */
					str_upp(name); /*upper case */
					if(strcmp(name, appid) == 0){
						found = 1; /* appid found */
						dxf_obj_subst(current, NULL); /* remove head of extended data*/
					}
				}
			}
			else{
				/* breaks if is found a new APPID entry */
				if(current->value.group == 1001){
					break;
				}
				dxf_obj_subst(current, NULL); /* remove extended data*/
			}
		}
		/* breaks if is found a entity */
		else break;
		
		current = current->next;
	}
	if (found) lua_pushboolean(L, 1); /* return success */
	else lua_pushboolean(L, 0); /* return fail */
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
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	
	ent->drawing = gui->drawing;
	
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
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	
	ent->drawing = gui->drawing;
	
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
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	
	ent->drawing = gui->drawing;
	
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
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	
	ent->drawing = gui->drawing;
	
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
	struct ent_lua *ent = (struct ent_lua *) lua_newuserdatauv(L, sizeof(struct ent_lua), 0);  /* create a userdata object */
	ent->curr_ent = new_el;
	ent->orig_ent = NULL;
	ent->sel = 0;
	
	ent->drawing = gui->drawing;
	
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
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
		/* add to undo/redo list*/
		if (!script->do_init){
			char msg[DXF_MAX_CHARS];
			strncpy(msg, "SCRIPT:", DXF_MAX_CHARS - 1);
			lua_Debug ar;
			lua_getstack (L, 1, &ar);
			lua_getinfo(L, "S", &ar); /* get script file name */
			if (strlen (get_filename(ar.short_src)) > 0)
				strncat(msg, get_filename(ar.short_src), DXF_MAX_CHARS - 8);
			else
				strncat(msg, get_filename(script->path), DXF_MAX_CHARS - 8);
			do_add_entry(&gui->list_do, msg);
			script->do_init = 1;
		}
		
		do_add_item(gui->list_do.current, NULL, blkrec);
		do_add_item(gui->list_do.current, NULL, blk);
		lua_pushlightuserdata(L, (void *) blk); /* return success */
	}
	else lua_pushnil(L); /* return fail */
	return 1;
}

/* ========= get drawing's global parameters =========== */

/* get the APPIDs in drawing */
/* given parameters:
	- none
returns:
	- table with APPIDs (key as APPID's name, value as light user data)
*/
int script_get_dwg_appids (lua_State *L) {
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
	
	dxf_node *appid = NULL, *nxt_appid = NULL, *name_obj;
	char name[DXF_MAX_CHARS+1];
	
	lua_newtable(L); /* table to store APPIDs */
	
	while (appid = dxf_find_obj_nxt(gui->drawing->t_appid, &nxt_appid, "APPID")){
		name_obj = dxf_find_attr2(appid, 2);
		if (name_obj){
			strncpy(name, name_obj->value.s_data, DXF_MAX_CHARS); /* preserve original string */
			str_upp(name); /*upper case */
			
			lua_pushstring(L, name);
			lua_pushlightuserdata(L, appid);
			lua_rawset(L, -3);
		}
		if (!nxt_appid) break; /* end of APPIDs*/
	}
	
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

/* set gui modal */
/* given parameters:
	- modal, as string ( acceptable values = SELECT, LINE, ...)
returns:
	- boolean, success or fail
*/
int script_set_modal (lua_State *L) {
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
	if (n < 1){
		lua_pushliteral(L, "set_modal: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isstring(L, 1)) { /* arguments types */
		lua_pushliteral(L, "set_modal: incorrect argument type");
		lua_error(L);
	}
	/* ----------------switch the modal options --------------*/
	char modal[DXF_MAX_CHARS];
	strncpy(modal, lua_tostring(L, 1), DXF_MAX_CHARS - 1);
	str_upp(modal);
	char *new_modal = trimwhitespace(modal);
	
	if (strcmp(new_modal, "LINE") == 0){
		gui->modal = LINE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "POLYLINE") == 0){
		gui->modal = POLYLINE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "RECT") == 0){
		gui->modal = RECT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "TEXT") == 0){
		gui->modal = TEXT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "CIRCLE") == 0){
		gui->modal = CIRCLE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ELLIPSE") == 0){
		gui->modal = ELLIPSE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ARC") == 0){
		gui->modal = ARC;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "SPLINE") == 0){
		gui->modal = SPLINE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "HATCH") == 0){
		gui->modal = HATCH;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "MTEXT") == 0){
		gui->modal = MTEXT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "INSERT") == 0){
		gui->modal = INSERT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "IMAGE") == 0){
		gui->modal = IMAGE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "MOVE") == 0){
		gui->modal = MOVE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "DUPLICATE") == 0){
		gui->modal = DUPLI;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "SCALE") == 0){
		gui->modal = SCALE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ROTATE") == 0){
		gui->modal = ROTATE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "MIRROR") == 0){
		gui->modal = MIRROR;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "EXPLODE") == 0){
		gui->modal = EXPLODE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ADD_ATTRIB") == 0){
		gui->modal = ADD_ATTRIB;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ED_ATTR") == 0){
		gui->modal = ED_ATTR;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "ED_TEXT") == 0){
		gui->modal = ED_TEXT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "TXT_PROP") == 0){
		gui->modal = TXT_PROP;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "VERTEX") == 0){
		gui->modal = VERTEX;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "NEW_BLK") == 0){
		gui->modal = NEW_BLK;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "SELECT") == 0){
		gui->modal = SELECT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "FIND") == 0){
		gui->modal = FIND;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "MEASURE") == 0){
		gui->modal = MEASURE;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	else if (strcmp(new_modal, "PROP") == 0){
		gui->modal = PROP;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	if (strcmp(new_modal, "SCRIPT") == 0){
		gui->modal = SCRIPT;
		gui->step = 0;
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	
	lua_pushboolean(L, 0); /* return fail */
	return 1;
}


/* new APPID to the drawing */
/* given parameters:
	- APPID, as string
returns:
	- success, as boolean
*/
int script_new_appid (lua_State *L) {
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
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "new_appid: incorrect argument type");
		lua_error(L);
	}
	
	char appid [DXF_MAX_CHARS + 1];
	char name [DXF_MAX_CHARS + 1];
	
	strncpy(appid, lua_tostring(L, 1), DXF_MAX_CHARS); /* preserve original string */
	str_upp(appid); /*upper case */
	
	char *new_str;
	new_str = trimwhitespace(appid);
	/* verify if  appid name contain spaces */
	if (strchr(new_str, ' ')){
		lua_pushliteral(L, "new_appid: No spaces allowed in APPID");
		lua_error(L);
	}
	
	dxf_node *appid_obj = NULL, *nxt_appid_obj = NULL, *name_obj;
	
	/* check if exists an APPID with same name */
	
	while (appid_obj = dxf_find_obj_nxt(gui->drawing->t_appid, &nxt_appid_obj, "APPID")){
		name_obj = dxf_find_attr2(appid_obj, 2);
		if (name_obj){
			strncpy(name, name_obj->value.s_data, DXF_MAX_CHARS); /* preserve original string */
			str_upp(name); /*upper case */
			
			if (strcmp(name, new_str) == 0){
				lua_pushboolean(L, 0); /* return fail */
				return 1;
			}
		}
		if (!nxt_appid_obj) break; /* end of APPIDs*/
	}
	
	/* get APPID name*/
	strncpy(appid, lua_tostring(L, 1), DXF_MAX_CHARS); /* preserve original string */
	new_str = trimwhitespace(appid);
	
	/* create a new APPID object */
	const char *handle = "0";
	const char *dxf_class = "AcDbSymbolTableRecord";
	const char *dxf_subclass = "AcDbRegAppTableRecord";
	int ok = 1;
	dxf_node * new_appid = dxf_obj_new ("APPID", DWG_LIFE);
	ok &= dxf_attr_append(new_appid, 5, (void *) handle, DWG_LIFE);
	ok &= dxf_attr_append(new_appid, 100, (void *) dxf_class, DWG_LIFE);
	ok &= dxf_attr_append(new_appid, 100, (void *) dxf_subclass, DWG_LIFE);
	ok &= dxf_attr_append(new_appid, 2, (void *) new_str, DWG_LIFE);
	ok &= dxf_attr_append(new_appid, 70, (int[]){0}, DWG_LIFE);
	/* register and append on drawing */
	if (ok) ok &= ent_handle(gui->drawing, new_appid);
	
	if (ok){
		dxf_append(gui->drawing->t_appid, new_appid);
		lua_pushboolean(L, 1); /* return success */
		return 1;
	}
	
	lua_pushboolean(L, 0); /* return fail */
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
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
	
	gui_script_clear_dyn(gui);
	
	script->dynamic = 1;
	gui->modal = SCRIPT;
	gui_first_step(gui);
	strncpy(script->dyn_func, name, DXF_MAX_CHARS - 1);
	
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
		lua_error(L);
	}
	
	script->dynamic = 0;
	script->dyn_func[0] = 0;
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
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
	script->active = 1;
	strncpy(script->win, name, DXF_MAX_CHARS - 1);
	
	strncpy(script->win_title, lua_tostring(L, 2), DXF_MAX_CHARS - 1);
	script->win_x = lua_tonumber(L, 3);
	script->win_y = lua_tonumber(L, 4);
	script->win_w = lua_tonumber(L, 5);
	script->win_h = lua_tonumber(L, 6);
	
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
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
		lua_error(L);
	}
	
	script->active = 0;
	script->win[0] = 0;
	
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


/* ========= MINIZ ===============================================*/


struct script_miniz_arch {
	mz_zip_archive *archive;
};

int script_miniz_open (lua_State *L) {
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "open: invalid number of arguments");
		lua_error(L);
	}
	luaL_argcheck(L, lua_isstring(L, 1), 1, "string expected");
	
	struct script_miniz_arch * zip;
	
	/* create a userdata object */
	zip = (struct script_miniz_arch *) lua_newuserdatauv(L, sizeof(struct script_miniz_arch *), 0); 
	luaL_getmetatable(L, "Zip");
	lua_setmetatable(L, -2);
	
	/* init the structure */
	zip->archive = malloc(sizeof(mz_zip_archive));
	if(!zip->archive){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	memset(zip->archive, 0, sizeof(mz_zip_archive));
	
	/* try to open the archive. */
	if (!mz_zip_reader_init_file(zip->archive, lua_tostring(L, 1), 0) )  {
		free(zip->archive);
		lua_pop(L, 1);
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	return 1;
}

int script_miniz_close (lua_State *L) {
	
	struct script_miniz_arch * zip;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( zip =  luaL_checkudata(L, 1, "Zip") )) { /* the archive is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, zip->archive != NULL, 1, "archive is closed");
	
	/* Close the archive, freeing any resources it was using */
	mz_zip_reader_end(zip->archive);
	free(zip->archive);
	zip->archive = NULL;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_miniz_read (lua_State *L) {
	
	struct script_miniz_arch * zip;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "read: invalid number of arguments");
		lua_error(L);
	}
	if (!( zip =  luaL_checkudata(L, 1, "Zip") )) { /* the archive is a Lua userdata type*/
		lua_pushliteral(L, "read: incorrect argument type");
		lua_error(L);
	}
	/* check if it is not closed */
	luaL_argcheck(L, zip->archive != NULL, 1, "archive is closed");
	
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	/* Try to extract one files to the heap. */
	size_t size;
	char *p = mz_zip_reader_extract_file_to_heap(zip->archive, lua_tostring(L, 2), &size, 0);
	if (!p){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	luaL_Buffer b;
	char *buff = luaL_buffinitsize (L, &b, size);
	memcpy(buff, p, size);
	luaL_pushresultsize (&b, size);
	
	/* free resources */
	mz_free(p);
	
	/* return success */
	return 1;
}


/* ==================== YXML ===========================*/

#define BUFSIZE 4096

struct script_yxml_state {
	yxml_t *x;
};

int script_yxml_new (lua_State *L) {
	
	struct script_yxml_state * state;
	
	/* create a userdata object */
	state = (struct script_yxml_state *) lua_newuserdatauv(L, sizeof(struct script_yxml_state *), 0); 
	luaL_getmetatable(L, "Yxml");
	lua_setmetatable(L, -2);
	
	/* memory allocation */
	state->x = malloc(sizeof(yxml_t) + BUFSIZE);
	
	if(!state->x){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* init the parser */
	yxml_init(state->x, state->x+1, BUFSIZE);
	
	return 1;
}

int script_yxml_close (lua_State *L) {
	
	struct script_yxml_state * state;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( state =  luaL_checkudata(L, 1, "Yxml") )) { /* the archive is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, state->x != NULL, 1, "parser is closed");
	
	/* free any resources it was using */
	free(state->x);
	state->x = NULL;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_yxml_read (lua_State *L) {
	
	struct script_yxml_state * state;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "read: invalid number of arguments");
		lua_error(L);
	}
	if (!( state =  luaL_checkudata(L, 1, "Yxml") )) { /* the parser is a Lua userdata type*/
		lua_pushliteral(L, "read: incorrect argument type");
		lua_error(L);
	}
	/* check if it is not closed */
	luaL_argcheck(L, state->x != NULL, 1, "parser is closed");
	
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	luaL_Buffer b;  /* to store parcial strings */
	int content = 0, attr = 0, attrval = 0;
	
	char *doc = (char*) lua_tostring(L, 2);  /*get document to parse */
	for(; *doc; doc++) { /* sweep document, analysing each character */
		yxml_ret_t r = yxml_parse(state->x, *doc);
		if(r < 0){ /* parsing error */
			lua_pushliteral(L, "parse error");
			lua_error(L);
		}
		
		/* Handle any tokens we are interested in */
		switch(r) {
			case YXML_ELEMSTART: /* start a element */
				/*previous attr table not stored yet*/
				if (attr){
					attr = 0;
					if (lua_istable(L, -1) && lua_istable(L, -2)){
						/* store table with key "attr" in owner table */
						lua_pushstring(L, "attr");
						lua_pushvalue (L, -2);
						lua_remove (L, -3);
						lua_rawset(L, -3);
					}
				}
				/* element main table */
				lua_newtable(L);
				/* store element name with key "id" */
				lua_pushstring(L, "id");
				lua_pushstring(L, state->x->elem);
				lua_rawset(L, -3);
				/*reset flags */
				content = 0;
				attr = 0;
				attrval = 0;
				break;
			case YXML_ELEMEND:
				/* element attr table not stored yet*/
				if (attr){
					attr = 0;
					if (lua_istable(L, -1) && lua_istable(L, -2)){
						/* store table with key "attr" in owner table */
						lua_pushstring(L, "attr");
						lua_pushvalue (L, -2);
						lua_remove (L, -3);
						lua_rawset(L, -3);
					}
				}
				/* store content string with key "cont" in element owner table */
				if (content && lua_istable(L, -1)){
					lua_pushstring(L, "cont");
					luaL_pushresult(&b); /* finalize string and put on Lua stack */
					lua_rawset(L, -3);
				}
				/* store element in its owner table, if exists */
				if (lua_istable(L, -1) && lua_istable(L, -2)){
					int index = lua_rawlen (L, -2) + 1;
					lua_rawseti(L, -2, index);  /* set table at key `index' */
				}
				/*reset flags */
				content = 0;
				attr = 0;
				attrval = 0;
				break;
			case YXML_CONTENT:
				if (!content){ /*init content */
					content = 1;
					luaL_buffinit(L, &b); /* init the Lua buffer */
					/* store attr table in its owner */
					if (attr){
						attr = 0;
						if (lua_istable(L, -1) && lua_istable(L, -2)){
							/* store table with key "attr" in owner table */
							lua_pushstring(L, "attr");
							lua_pushvalue (L, -2);
							lua_remove (L, -3);
							lua_rawset(L, -3);
						}
					}
				}
				/* store parcial string */
				luaL_addstring(&b, state->x->data);
				break;
			case YXML_ATTRSTART:
				if (!attr){ /* init attributes */
					attr = 1;
					lua_newtable(L); /* element attr table */
				}
				break;
			case YXML_ATTRVAL:
				if (!attrval){ /*init attribute value */
					attrval = 1;
					luaL_buffinit(L, &b); /* init the Lua buffer */
				}
				/* store parcial string */
				luaL_addstring(&b, state->x->data);
				break;
			case YXML_ATTREND:
				/* Now we have a full attribute. Its name is in x->attr, 
				and its value is in Lua buffer "b". */
				if (attrval){
					attrval = 0;
					if (lua_istable(L, -1)){
						/* store in its owner table, where its name is the key */
						lua_pushstring(L, state->x->attr);
						luaL_pushresult(&b); /* finalize string and put on Lua stack */
						lua_rawset(L, -3);
					}
				}
				break;
		}
	}
	/* end parsing */
	yxml_eof(state->x);
	/* restart parser */
	yxml_init(state->x, state->x+1, BUFSIZE);
	
	if (!lua_istable(L, -1)) lua_pushnil(L); /* return fail */
	/* or return success */
	return 1;
}
