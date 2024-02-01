#include "script.h"
#include "miniz.h"
#include "yxml.h"
#include "gui_script.h"
#include "i_svg_media.h"

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

/* check if timeout is elapsed - for debug purposes */
/* given parameters:
	- none
returns:
	- elapsed, as boolean
*/
int check_timeout (lua_State *L) {
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	/* verify if script is valid */
	if (!script){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	clock_t end_t;
  double diff_t;
  /* get the elapsed time since script starts or continue */
  end_t = clock();
  diff_t = (double)(end_t - script->time) / CLOCKS_PER_SEC;
  
  /* verify if timeout is reachead */
  if (diff_t >= script->timeout){
    lua_pushboolean(L, 1); /* return elapsed */
    return 1;
  }
	lua_pushboolean(L, 0); /* return OK */
	return 1;
}

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

/* print to internal debug buffer */
void print_internal (void *data, char* str){
  gui_obj *gui = (gui_obj *) data;
  if (gui->debug_out_pos < DEBUG_OUT){
    gui->debug_out_pos += snprintf(gui->debug_out + gui->debug_out_pos,
      DEBUG_OUT - gui->debug_out_pos, str);
  }
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
		if (i > 1) {
      nk_str_append_str_char(&gui->debug_edit.string, "    ");
      print_internal (gui, "    ");
    }
    
		switch(type) {
			case LUA_TSTRING: {
        char *str = (char*) lua_tostring(L, i);
				snprintf(msg, DXF_MAX_CHARS - 1, "%s", str);
        print_internal (gui, str);
				break;
			}
			case LUA_TNUMBER: {
			/* LUA_NUMBER may be double or integer */
				snprintf(msg, DXF_MAX_CHARS - 1, "%.9g", lua_tonumber(L, i));
        print_internal (gui, msg);
				break;
			}
			case LUA_TTABLE: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
        print_internal (gui, msg);
				break;
			}
			case LUA_TFUNCTION: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
        print_internal (gui, msg);
				break;
      }
			case LUA_TUSERDATA: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_touserdata(L, i));
        print_internal (gui, msg);
				break;
			}
			case LUA_TLIGHTUSERDATA: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_touserdata(L, i));
        print_internal (gui, msg);
				break;
			}
			case LUA_TBOOLEAN: {
				snprintf(msg, DXF_MAX_CHARS - 1, "%s", lua_toboolean(L, i) ? "true" : "false");
        print_internal (gui, msg);
				break;
			}
			case LUA_TTHREAD: {
				snprintf(msg, DXF_MAX_CHARS - 1, "0x%08x", lua_topointer(L, i));
        print_internal (gui, msg);
				break;
			}
			case LUA_TNIL: {
				snprintf(msg, DXF_MAX_CHARS - 1, "nil");
        print_internal (gui, msg);
				break;
			}
		}
		nk_str_append_str_char(&gui->debug_edit.string, msg);
	}
	/*enter a new line*/
	nk_str_append_str_char(&gui->debug_edit.string, "\n");
  print_internal (gui, "\n");
	
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

/* clear selected entities in drawing */
/* given parameters:
	- none
returns:
	- none
*/
int script_clear_sel (lua_State *L) {
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
	
	sel_list_clear (gui);
	gui->draw = 1;
	gui->draw_phanton = 0;
	gui->element = NULL;
	
	return 0;
}

/* enable select mode */
/* given parameters:
	- none
returns:
	- none
*/
int script_enable_sel (lua_State *L) {
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
	
	gui->step = 0;
	gui->free_sel = 1;
	gui->en_distance = 0;
	gui->sel_ent_filter = ~DXF_NONE;
	gui_simple_select(gui);
	
	return 0;
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
	lua_pushstring (L, strpool_cstr2( &obj_pool, ent->obj.id));
	return 1;
}

/* get the parameters (center point, radius) of a CIRCLE entity */
/* given parameters:
	- DXF CIRCLE entity, as userdata
returns:
	- success, table with params
	- nil if not a CIRCLES
*/
int script_get_circle_data (lua_State *L) {
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
		lua_pushliteral(L, "get_circle_data: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* verify if it is a CIRCLE ent */
  if (dxf_ident_ent_type (ent) != DXF_CIRCLE) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	double x = 0.0, y = 0.0, z = 0.0, ex = 1.0, ey = 1.0, ez = 1.0, r = 0.0;
	
	dxf_node *current = ent->obj.content->next;
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 10:
					x = current->value.d_data;
					break;
				case 20:
					y = current->value.d_data;
					break;
				case 30:
					z = current->value.d_data;
					break;
				case 210:
					ex = current->value.d_data;
					break;
				case 220:
					ey = current->value.d_data;
					break;
				case 230:
					ez = current->value.d_data;
					break;
				case 40:
					r = current->value.d_data;
					break;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	lua_newtable(L); /*main returned table */
	lua_pushstring(L, "center"); /* center point */
	
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
	
	lua_rawset(L, -3);  /* set main table at key `center' */
	
	lua_pushstring(L, "extru"); /* extrusion vector */
	
	lua_newtable(L); /* table to store factors */
	lua_pushstring(L, "x");
	lua_pushnumber(L, ex);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "y");
	lua_pushnumber(L, ey);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "z");
	lua_pushnumber(L, ez);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `extru' */
	
	lua_pushstring(L, "radius");
	lua_pushnumber(L, r);
	lua_rawset(L, -3);  /* set main table at key `radius' */
	
	
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
  if (!(dxf_ident_ent_type (ent) == DXF_INSERT || dxf_ident_ent_type (ent) == DXF_DIMENSION)) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_node *tmp = NULL;
	if(tmp = dxf_find_attr2(ent, 2)) /* look for block name */
		lua_pushstring (L, strpool_cstr2( &name_pool, tmp->value.str));
	else lua_pushnil(L); /* return fail */
	return 1;
}

/* get the parameters (point, scales, rotation) of a INSERT entity */
/* given parameters:
	- DXF INSERT entity, as userdata
returns:
	- success, table with params
	- nil if not a INSERT
*/
int script_get_ins_data (lua_State *L) {
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
		lua_pushliteral(L, "get_ins_data: incorrect argument type");
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
  if (!(dxf_ident_ent_type (ent) == DXF_INSERT || dxf_ident_ent_type (ent) == DXF_DIMENSION)) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	double x = 0.0, y = 0.0, z = 0.0, sx = 1.0, sy = 1.0, sz = 1.0, rot = 0.0;
	
	dxf_node *current = ent->obj.content->next;
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 10:
					x = current->value.d_data;
					break;
				case 20:
					y = current->value.d_data;
					break;
				case 30:
					z = current->value.d_data;
					break;
				case 41:
					sx = current->value.d_data;
					break;
				case 42:
					sy = current->value.d_data;
					break;
				case 43:
					sz = current->value.d_data;
					break;
				case 50:
					rot = current->value.d_data;
					break;
			}
		}
		current = current->next; /* go to the next in the list */
	}
	lua_newtable(L); /*main returned table */
	lua_pushstring(L, "pt"); /* insert point */
	
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
	
	lua_rawset(L, -3);  /* set main table at key `pt' */
	
	lua_pushstring(L, "scale"); /* insert point */
	
	lua_newtable(L); /* table to store scale factors */
	lua_pushstring(L, "x");
	lua_pushnumber(L, sx);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "y");
	lua_pushnumber(L, sy);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "z");
	lua_pushnumber(L, sz);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `scale' */
	
	lua_pushstring(L, "rot");
	lua_pushnumber(L, rot);
	lua_rawset(L, -3);  /* set main table at key `rot' */
	
	
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
  if (dxf_ident_ent_type (ent) != DXF_INSERT){
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
  if (dxf_ident_ent_type (ent) != DXF_INSERT){
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
				strncpy(tag, strpool_cstr2( &name_pool, tmp->value.str), DXF_MAX_CHARS);
			if(tmp = dxf_find_attr2(attr, 1))
				strncpy(value, strpool_cstr2( &value_pool, tmp->value.str), DXF_MAX_CHARS);
			
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
  if (dxf_ident_ent_type (ent) != DXF_INSERT){
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
			strncpy(tag, strpool_cstr2( &name_pool, tmp->value.str), DXF_MAX_CHARS);
		if(tmp = dxf_find_attr2(attr, 1))
			strncpy(value, strpool_cstr2( &value_pool, tmp->value.str), DXF_MAX_CHARS);
		
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
	- DXF entity, as userdata
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

/* get rectangle boundary points of an entity */
/* given parameters:
	- DXF entity, as userdata
returns:
	- success, table with coordinates
	- nil if not a entity
*/
int script_get_bound (lua_State *L) {
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
		lua_pushliteral(L, "get_bound: incorrect argument type");
		lua_error(L);
	}
	
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	list_node *vec_graph = dxf_graph_parse(ent_obj->drawing, ent, 0, FRAME_LIFE);
	if (!vec_graph){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* extents parameters */
	int ei = 0; /*extents flag of current block */
	double x0, y0, z0, x1, y1, z1;
	
	/* get extents parameters of current block*/
	graph_list_ext(vec_graph, &ei, &x0, &y0, &z0, &x1, &y1, &z1);
	if (!ei) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	lua_newtable(L); /*main returned table */
	lua_pushstring(L, "low"); /* lower left corner */
	
	lua_newtable(L); /* table to store point coordinates */
	lua_pushstring(L, "x");
	lua_pushnumber(L, x0);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "y");
	lua_pushnumber(L, y0);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "z");
	lua_pushnumber(L, z0);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `low' */
	
	lua_pushstring(L, "up"); /* up right corner */
	
	lua_newtable(L); /* table to store point coordinates */
	lua_pushstring(L, "x");
	lua_pushnumber(L, x1);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "y");
	lua_pushnumber(L, y1);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "z");
	lua_pushnumber(L, z1);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `up' */
	
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
					strncpy(name, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
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
            if (current->value.group == 2 || (current->value.group > 5 && current->value.group < 9) ){
              lua_pushstring(L, strpool_cstr2( &name_pool, current->value.str));
            } else {
              lua_pushstring(L, strpool_cstr2( &value_pool, current->value.str));
            }
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
			/* skip ENDBLK elements */
      if (dxf_ident_ent_type (current) != DXF_ENDBLK){
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

/* get the parameters (text, point, scales, rotation) of a TEXT entity */
/* given parameters:
	- DXF TEXT entity, as userdata
returns:
	- success, table with params
	- nil if not a TEXT
*/
int script_get_text_data (lua_State *L) {
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
		lua_pushliteral(L, "get_text_data: incorrect argument type");
		lua_error(L);
	}
	/* get entity */
	dxf_node *ent = ent_obj->curr_ent;  /*try to get current entity */
	if (!ent) ent = ent_obj->orig_ent; /* if not current, try original entity */
	if (!ent) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	/* verify if it is a TEXT ent */
  if (!(dxf_ident_ent_type (ent) == DXF_TEXT
		//|| (strcmp(ent->obj.name, "MTEXT") == 0) 
	)) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	double x = 0.0, y = 0.0, z = 0.0, sx = 1.0, sy = 1.0, sz = 1.0, rot = 0.0, size = 1.0;
	char text[DXF_MAX_CHARS+1] = "";
	char t_style[DXF_MAX_CHARS+1] = "";
	char tmp_str[DXF_MAX_CHARS+1] = "";
	int alin_v = 0, alin_h = 0;
	
	dxf_node *current = ent->obj.content->next;
	while (current){
		if (current->type == DXF_ATTR){ /* DXF attibute */
			switch (current->value.group){
				case 1:
					strncpy(text, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
					break;
				case 7:
					strncpy(t_style, strpool_cstr2( &name_pool, current->value.str), DXF_MAX_CHARS);
					break;
				case 10:
					x = current->value.d_data;
					break;
				case 20:
					y = current->value.d_data;
					break;
				case 30:
					z = current->value.d_data;
					break;
				case 40:
					size = current->value.d_data;
					break;
				case 41:
					sx = current->value.d_data;
					break;
				case 50:
					rot = current->value.d_data;
					break;
				case 72:
					alin_h = current->value.i_data;
					break;
				case 73:
					alin_v = current->value.i_data;
					break;
				case 101:
					strncpy(tmp_str, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
					str_upp(tmp_str);
					char *tmp = trimwhitespace(tmp_str);
					if (strcmp (tmp, "EMBEDDED OBJECT") == 0 ){
						current = NULL;
						continue;
					}
			}
		}
		current = current->next; /* go to the next in the list */
	}
	lua_newtable(L); /*main returned table */
	
	lua_pushstring(L, "pt"); /* insert point */
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
	
	lua_rawset(L, -3);  /* set main table at key `pt' */
	
	lua_pushstring(L, "scale"); /* scale */
	lua_newtable(L); /* table to store scale factors */
	lua_pushstring(L, "x");
	lua_pushnumber(L, sx);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "y");
	lua_pushnumber(L, sy);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "z");
	lua_pushnumber(L, sz);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `scale' */
	
	lua_pushstring(L, "rot");
	lua_pushnumber(L, rot);
	lua_rawset(L, -3);  /* set main table at key `rot' */
	
	lua_pushstring(L, "size");
	lua_pushnumber(L, size);
	lua_rawset(L, -3);  /* set main table at key `size' */
	
	lua_pushstring(L, "text");
	lua_pushstring(L, text);
	lua_rawset(L, -3);  /* set main table at key `text' */
	
	lua_pushstring(L, "style");
	lua_pushstring(L, t_style);
	lua_rawset(L, -3);  /* set main table at key `style' */
	
	const char *al_v[] = {"BASE LINE", "BOTTOM", "MIDDLE", "TOP"};
	const char *al_h[] = {"LEFT", "CENTER", "RIGHT", "ALIGNED", "MIDDLE", "FIT"};
	
	lua_pushstring(L, "align"); /* alignment */
	lua_newtable(L); /* table to store alignment data */
	lua_pushstring(L, "h");
	lua_pushstring(L, al_h[alin_h]);
	lua_rawset(L, -3);
	
	lua_pushstring(L, "v");
	lua_pushstring(L, al_v[alin_v]);
	lua_rawset(L, -3);
	
	lua_rawset(L, -3);  /* set main table at key `scale' */
	
	
	return 1;
}


/* get the current drawing path (dir and filename) */
/* given parameters:
	- none
returns:
	- filename as string (blank if not a saved drawing)
	- dir as string
*/
int script_get_drwg_path (lua_State *L) {
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
	
	lua_pushstring (L, gui->dwg_file);
	lua_pushstring (L, gui->dwg_dir);
	
	return 2;
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
  if (dxf_ident_ent_type (ent) != DXF_INSERT){
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
					strncpy(name, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
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
					strncpy(name, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
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
					strncpy(name, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
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
					strncpy(name, strpool_cstr2( &value_pool, current->value.str), DXF_MAX_CHARS);
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
  - drawing parameters (layer, color, etc), as table (optional)
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
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,7)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 7); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
  }
	
	/* new LINE entity */
	dxf_node * new_el = (dxf_node *) dxf_new_line (
		lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), /* first point */
		lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), /* end point */
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, FRAME_LIFE); /* paper space */
	
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
  
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
  - drawing parameters (layer, color, etc), as table (optional)
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
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,7)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 7); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
  }
	
	/* new LWPOLYLINE entity */
	dxf_node *new_el = (dxf_node *) dxf_new_lwpolyline (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, /* pt1, */
		lua_tonumber(L, 3), /* bulge */
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, FRAME_LIFE); /* paper space */
	/* append second vertex to ensure entity's validity */
	dxf_lwpoly_append (new_el, lua_tonumber(L, 4), lua_tonumber(L, 5), 0.0, lua_tonumber(L, 6), FRAME_LIFE);
	
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
  
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
  - drawing parameters (layer, color, etc), as table (optional)
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
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,4)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 4); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
  }
  
	/* new CIRCLE entity */
	dxf_node * new_el = (dxf_node *) dxf_new_circle (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, lua_tonumber(L, 3), /* pt1, radius */
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, FRAME_LIFE); /* paper space */
	
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
  
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
  - drawing parameters (layer, color, etc), as table (optional)
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
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,3)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 3); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
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
	gui->color_idx, /* color, layer */
  (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
	/* line type, line weight */
  (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
  dxf_lw[gui->lw_idx],
	0, FRAME_LIFE); /* paper space */
  
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
	
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
  - drawing parameters (layer, color, etc), as table (optional)
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
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,7)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 7); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
  }
	
	/* create a new DXF TEXT */
	dxf_node * new_el = (dxf_node *) dxf_new_text (
		lua_tonumber(L, 1), lua_tonumber(L, 2), 0.0, txt_h, /* pt1, height */
		txt, /* text, */
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, DWG_LIFE); /* paper space */
	dxf_attr_change_i(new_el, 72, &t_al_h, -1);
	dxf_attr_change_i(new_el, 73, &t_al_v, -1);
	dxf_attr_change(new_el, 7,
    (char *) strpool_cstr2( &name_pool, gui->drawing->text_styles[gui->t_sty_idx].name));
  
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
	
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
	- description, as string (optional)
	- convert texts elements to tags, as boolean (optional) - default is "false"
	- mark for convert to tag, as string (optional) - default is "#"
	- mark for hidden tags, as string (optional) - default is "*"
	- mark for tag value, as string (optional) - default is "$"
	- default value for tags, as string (optional) - default is "?"
	- x, y, z, as number (optional) - reference point (zero) for block. If not given, minimal coordinates of elements is used.
returns:
	- success, as boolean
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
	
	char descr[DXF_MAX_CHARS+1];
	descr[0] = 0;
	double orig[3];
	int txt2attr = 0, ref_pt = 0;
	char mark[DXF_MAX_CHARS+1];
	mark[0] = '#'; mark[1] = 0;
	char hide_mark[DXF_MAX_CHARS+1];
	hide_mark[0] = '*'; hide_mark[1] = 0;
	char value_mark[DXF_MAX_CHARS+1];
	value_mark[0] = '$'; value_mark[1] = 0;
	char dflt_value[DXF_MAX_CHARS+1];
	dflt_value[0] = '?'; dflt_value[1] = 0;
	
	
	/* get name */
	char name[DXF_MAX_CHARS+1];
	name[0] = 0;
	strncpy(name, lua_tostring(L, 2), DXF_MAX_CHARS);
	if (strlen(name) <= 0) { /* invalid text */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	if (n > 2){
		if (!lua_isstring(L, 3)) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			strncpy(descr, lua_tostring(L, 3), DXF_MAX_CHARS);
		}
	}
	
	if (n > 3){
		if (!lua_isboolean(L, 4)) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			txt2attr = lua_toboolean(L, 4);
		}
	}
	
	if (txt2attr){
		if (n > 4){
			if (!lua_isstring(L, 5)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 5)) ){
				strncpy(mark, lua_tostring(L, 5), DXF_MAX_CHARS);
			}
		}
		if (n > 5){
			if (!lua_isstring(L, 6)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 6)) ){
				strncpy(hide_mark, lua_tostring(L, 6), DXF_MAX_CHARS);
			}
		}
		if (n > 6){
			if (!lua_isstring(L, 7)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 7)) ){
				strncpy(value_mark, lua_tostring(L, 7), DXF_MAX_CHARS);
			}
		}
		if (n > 7){
			if (!lua_isstring(L, 8)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 8)) ){
				strncpy(dflt_value, lua_tostring(L, 8), DXF_MAX_CHARS);
			}
		}
	}
	
	if (n > 10){
		if (!lua_isnumber(L, 9) || !lua_isnumber(L, 10) || !lua_isnumber(L, 11) ) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			ref_pt = 1;
			orig[0] = lua_tonumber(L, 9);
			orig[1] = lua_tonumber(L, 10);
			orig[2] = lua_tonumber(L, 11);
		}
	}
	
	dxf_drawing *drawing = gui->drawing;
	
	/* verify if block not exist */
	if (dxf_find_obj_descr2(drawing->blks, "BLOCK", name) != NULL){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_node *blkrec = NULL, *blk = NULL;
	dxf_node *obj;
	
	struct ent_lua *ent_obj;
	
	/* create the vector of returned values */
	list_node *list = list_new(NULL, FRAME_LIFE);
	
	/*create a list of entities, from passed lua table*/
	/* iterate over table */
	lua_pushnil(L);  /* first key */
	while (lua_next(L, 1) != 0) { /* table index are shifted*/
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if ( ent_obj =  luaL_checkudata(L, -1, "cz_ent_obj") ){
			/* get entity */
			obj = ent_obj->curr_ent;  /*try to get current entity */
			if (!obj) obj = ent_obj->orig_ent; /* if not current, try original entity */
			
			list_push(list, list_new((void *)obj, FRAME_LIFE));
			
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	int ok = 0;
	
	/* Create block*/	
	if (ref_pt) ok = dxf_new_block (drawing, name, descr, orig, txt2attr, 
		mark, hide_mark, value_mark, dflt_value,
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), list,
		&blkrec, &blk, DWG_LIFE);
	else ok = dxf_new_block (drawing, name, descr, NULL, txt2attr, 
		mark, hide_mark, value_mark, dflt_value,
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), list,
		&blkrec, &blk, DWG_LIFE);
	
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
		//lua_pushlightuserdata(L, (void *) blk); /* return success */
		lua_pushboolean(L, 1); /* return success */
	}
	else lua_pushboolean(L, 0);  /* return fail */
	return 1;
}

/* create a BLOCK from a DXF file*/
/* given parameters:
	- file path, as string
	- block name, as string
	- description, as string (optional)
	- convert texts elements to tags, as boolean (optional) - default is "false"
	- mark for convert to tag, as string (optional) - default is "#"
	- mark for hidden tags, as string (optional) - default is "*"
	- mark for tag value, as string (optional) - default is "$"
	- default value for tags, as string (optional) - default is "?"
	- x, y, z, as number (optional) - reference point (zero) for block. If not given, minimal coordinates of elements is used.
returns:
	- success, as boolean
Notes:
	- This function will append the created block to drawing's Block section.
	No actions are requiried after this.
*/
int script_new_block_file (lua_State *L) {
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
	
	if (!lua_isstring(L, 1)) { /* arguments types */
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
	
	char descr[DXF_MAX_CHARS+1];
	descr[0] = 0;
	double orig[3];
	int txt2attr = 0, ref_pt = 0;
	char mark[DXF_MAX_CHARS+1];
	mark[0] = '#'; mark[1] = 0;
	char hide_mark[DXF_MAX_CHARS+1];
	hide_mark[0] = '*'; hide_mark[1] = 0;
	char value_mark[DXF_MAX_CHARS+1];
	value_mark[0] = '$'; value_mark[1] = 0;
	char dflt_value[DXF_MAX_CHARS+1];
	dflt_value[0] = '?'; dflt_value[1] = 0;
	
	/* get path */
	char path[PATH_MAX_CHARS+1];
	path[0] = 0;
	strncpy(path, lua_tostring(L, 1), PATH_MAX_CHARS);
	if (strlen(path) <= 0) { /* invalid text */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* get name */
	char name[DXF_MAX_CHARS+1];
	name[0] = 0;
	strncpy(name, lua_tostring(L, 2), DXF_MAX_CHARS);
	if (strlen(name) <= 0) { /* invalid text */
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	if (n > 2){
		if (!lua_isstring(L, 3)) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			strncpy(descr, lua_tostring(L, 3), DXF_MAX_CHARS);
		}
	}
	
	if (n > 3){
		if (!lua_isboolean(L, 4)) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			txt2attr = lua_toboolean(L, 4);
		}
	}
	
	if (txt2attr){
		if (n > 4){
			if (!lua_isstring(L, 5)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 5)) ){
				strncpy(mark, lua_tostring(L, 5), DXF_MAX_CHARS);
			}
		}
		if (n > 5){
			if (!lua_isstring(L, 6)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 6)) ){
				strncpy(hide_mark, lua_tostring(L, 6), DXF_MAX_CHARS);
			}
		}
		if (n > 6){
			if (!lua_isstring(L, 7)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 7)) ){
				strncpy(value_mark, lua_tostring(L, 7), DXF_MAX_CHARS);
			}
		}
		if (n > 7){
			if (!lua_isstring(L, 8)) { /* arguments types */
				lua_pushliteral(L, "new_block: incorrect argument type");
				lua_error(L);
			}
			else if (strlen (lua_tostring(L, 8)) ){
				strncpy(dflt_value, lua_tostring(L, 8), DXF_MAX_CHARS);
			}
		}
	}
	
	if (n > 10){
		if (!lua_isnumber(L, 9) || !lua_isnumber(L, 10) || !lua_isnumber(L, 11) ) { /* arguments types */
			lua_pushliteral(L, "new_block: incorrect argument type");
			lua_error(L);
		}
		else{
			ref_pt = 1;
			orig[0] = lua_tonumber(L, 9);
			orig[1] = lua_tonumber(L, 10);
			orig[2] = lua_tonumber(L, 11);
		}
	}
	
	dxf_drawing *drawing = gui->drawing;
	
	/* verify if block not exist */
	if (dxf_find_obj_descr2(drawing->blks, "BLOCK", name) != NULL){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	dxf_node *blkrec = NULL, *blk = NULL;
	
	int ok = 0;
	
	/* Create block*/
	if (ref_pt) ok = dxf_new_blk_file (drawing, name, descr, orig, txt2attr, 
		mark, hide_mark, value_mark, dflt_value,
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), path,
		&blkrec, &blk, DWG_LIFE);
	else ok = dxf_new_blk_file (drawing, name, descr, NULL, txt2attr, 
		mark, hide_mark, value_mark, dflt_value,
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name), path,
		&blkrec, &blk, DWG_LIFE);
	
	if (ok) {
		gui_tstyle(gui); /* add additional fonts to main list, if required */
		
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
		//lua_pushlightuserdata(L, (void *) blk); /* return success */
		lua_pushboolean(L, 1); /* return success */
	}
	else lua_pushboolean(L, 0);  /* return fail */
	return 1;
}

/* create a INSERT DXF entity */
/* given parameters:
	- Block name, as string
	- placement x, y, as numbers
	- scale x, y, as numbers (optional)
	- rotation in degrees, as number (optional)
  - drawing parameters (layer, color, etc), as table (optional)
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
int script_new_insert (lua_State *L) {
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
		lua_pushliteral(L, "new_insert: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isstring(L, 1)) { /* arguments types */
		lua_pushliteral(L, "new_insert: incorrect argument type");
		lua_error(L);
	}
	int i;
	for (i = 2; i <= 3; i++) { /* arguments types */
		if (!lua_isnumber(L, i)) {
			lua_pushliteral(L, "new_insert: incorrect argument type");
			lua_error(L);
		}
	}
	
	char *name = (char *) lua_tostring(L, 1);
	
	/* try to find corresponding block, by name */
	if (!dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", name)){
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
  
  /* change drawing params, temporarily */
  int prev_color = gui->color_idx;
  int prev_layer = gui->layer_idx;
  int prev_ltype = gui->ltypes_idx;
  int prev_style = gui->t_sty_idx;
  int prev_lw = gui->lw_idx;
  if (lua_istable(L,7)){
    lua_getglobal(L, "cadzinho"); /* function to be called */
    lua_getfield(L, -1, "set_param");
    lua_pushvalue(L, 7); /* push table with param keys */
    lua_pcall(L, 1, 1, 0); /* call function (1 arguments, 1 result) */
    lua_pop(L, 2); /* pop returned value */
  }
	
	/* new INSERT entity */
	
	dxf_node * new_el = (dxf_node *) dxf_new_insert ( name,
		x, y, 0.0, /* placement point */
		gui->color_idx, /* color, layer */
    (char *) strpool_cstr2( &name_pool, gui->drawing->layers[gui->layer_idx].name),
		/* line type, line weight */
    (char *) strpool_cstr2( &name_pool, gui->drawing->ltypes[gui->ltypes_idx].name),
    dxf_lw[gui->lw_idx],
		0, FRAME_LIFE); /* paper space */
		
	if (!new_el) {
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	/* get x scale, if exist*/
	double scale_x = 1.0;
	if (lua_isnumber(L, 4)) {
		scale_x = lua_tonumber(L, 4);
	}
	/* get y scale, if exist*/
	double scale_y = 1.0;
	if (lua_isnumber(L, 5)) {
		scale_y = lua_tonumber(L, 5);
	}
	/* get rotation, if exist*/
	double rot = 0.0;
	if (lua_isnumber(L, 6)) {
		rot = lua_tonumber(L, 6);
	}
	
	/* update scale and rotation parameters in insert entity */
	dxf_attr_change(new_el, 41, &scale_x);
	dxf_attr_change(new_el, 42, &scale_y);
	dxf_attr_change(new_el, 43, &scale_x);//
	if (rot <= 0.0) rot = 360.0 - rot;
	rot = fmod(rot, 360.0);
	dxf_attr_change(new_el, 50, &rot);
	
	
	/* convert block's ATTDEF to ATTRIBUTES*/
	dxf_node *blk = dxf_find_obj_descr2(gui->drawing->blks, "BLOCK", name);
	dxf_node *attdef, *attrib, *nxt_attdef = NULL;
	/* get attdef */
	while (attdef = dxf_find_obj_nxt(blk, &nxt_attdef, "ATTDEF")){
		/* convert and append to insert with translate, rotation and scale */
		attrib = dxf_attrib_cpy(attdef, x, y, 0.0, scale_x, rot, FRAME_LIFE);
		ent_handle(gui->drawing, attrib);
		dxf_insert_append(gui->drawing, new_el, attrib, FRAME_LIFE);
		
		if (!nxt_attdef) break;
	}
	
  
  /* restore original drawing parameters */
  gui->color_idx = prev_color;
  gui->layer_idx = prev_layer;
  gui->ltypes_idx = prev_ltype;
  gui->t_sty_idx = prev_style;
  gui->lw_idx = prev_lw;
	
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
			strncpy(name, /* preserve original string */
        strpool_cstr2( &name_pool, name_obj->value.str), DXF_MAX_CHARS);
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
		int idx = dxf_lay_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ));
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
		/* change current ltype */
		gui->ltypes_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char *name = (char *) lua_tostring(L, 1);
		int idx = dxf_ltype_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ));
		/* change current ltype */
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
		/* change current style */
		gui->t_sty_idx = idx;
	}
	else if (lua_isstring(L, 1)) {
		char *name = (char *) lua_tostring(L, 1);
		int idx = dxf_tstyle_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ), 0);
		/* change current style */
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
		/* change current lw */
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
			/* change current lw */
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

int script_set_param (lua_State *L) {
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
  
  if (!lua_istable(L, 1)) {
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  int idx = 0;
  
  /* layer */
	int field_typ = lua_getfield(L, 1, "layer");
  if (field_typ == LUA_TNUMBER) {
    idx = lua_tonumber(L, -1);
    int num_layers = gui->drawing->num_layers;
    if (idx < num_layers && idx >= 0){
      /* change current layer */
      gui->layer_idx = idx;
    }
  }
  else if (field_typ == LUA_TSTRING) {
    char *name = (char *) lua_tostring(L, -1);
    idx = dxf_lay_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ));
    /* change current layer */
    gui->layer_idx = idx;
  }
  lua_pop(L, 1);

  /* color */
  field_typ = lua_getfield(L, 1, "color");
  if (field_typ == LUA_TNUMBER) {
    idx = lua_tonumber(L, -1);
    if (idx < 257 && idx >= 0){
      /* change current color */
      gui->color_idx = idx;
    }
  }
  else if (field_typ == LUA_TSTRING) {
    char name[DXF_MAX_CHARS + 1];
    strncpy(name, lua_tostring(L, -1), DXF_MAX_CHARS);
    str_upp(name);
    char *new_name = trimwhitespace(name);
    
    if (strcmp(new_name, "BY BLOCK") == 0){
      /* change current color */
      gui->color_idx = 0;
    }
    else if (strcmp(new_name, "BY LAYER") == 0){
      /* change current color */
      gui->color_idx = 256;
    }
  }
  lua_pop(L, 1);

  /* ltype */
  field_typ = lua_getfield(L, 1, "ltype");
  if (field_typ == LUA_TNUMBER) {
    idx = lua_tonumber(L, -1);
    int num_ltypes = gui->drawing->num_ltypes;
    if (idx < num_ltypes && idx >= 0){
      /* change current line type */
      gui->ltypes_idx = idx;
    }
  
  }
  else if (field_typ == LUA_TSTRING) {
    char *name = (char *) lua_tostring(L, -1);
    idx = dxf_ltype_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ));
    /* change current line type */
    gui->ltypes_idx = idx;
  }
  lua_pop(L, 1);

  /* style */
  field_typ = lua_getfield(L, 1, "style");
  if (field_typ == LUA_TNUMBER) {
    idx = lua_tonumber(L, -1);
    int num_tstyles = gui->drawing->num_tstyles;
    if (idx < num_tstyles && idx >= 0){
      /* change current style */
      gui->t_sty_idx = idx;
    }
  }
  else if (field_typ == LUA_TSTRING) {
    char *name = (char *) lua_tostring(L, -1);
    idx = dxf_tstyle_idx (gui->drawing,
      strpool_inject( &name_pool, (char const*) name, strlen(name) ), 0);
    /* change current style*/
    gui->t_sty_idx = idx;
  }
  lua_pop(L, 1);

  /* lw */
  field_typ = lua_getfield(L, 1, "lw");
  if (field_typ == LUA_TNUMBER) {
    idx = lua_tonumber(L, -1);
    if (idx < DXF_LW_LEN + 2 && idx >= 0){
      /* change current lw */
      gui->lw_idx = idx;
    }
  }
  else if (field_typ == LUA_TSTRING) {
    char name[DXF_MAX_CHARS + 1];
    strncpy(name, lua_tostring(L, -1), DXF_MAX_CHARS);
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
  }
  lua_pop(L, 1);
  
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
		gui->modal = RECTANGLE;
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
	else if (strcmp(new_modal, "POINT") == 0){
		gui->modal = SINGLE_POINT;
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
			strncpy(name, /* preserve original string */
        strpool_cstr2( &name_pool, name_obj->value.str), DXF_MAX_CHARS);
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


int script_new_drwg_k (lua_State *L, int status, lua_KContext ctx) {
	/* continuation function for new drawing */
	
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
	
	/*verify if action is finished with errors */
	if (gui->script_wait_t.wait_gui_resume > 0){
		lua_pushboolean(L, 1); /* return success */
	}
	else {
		lua_pushboolean(L, 0); /* return fail */
	}
	
	gui->script_wait_t.wait_gui_resume = 0; /*clear current script flag */
	return 1;
}

/* new drawing */
/* given parameters:
	- none
returns:
	- boolean, success or fail

Warning: Unsaved changes in current drawing will be lost.
*/
int script_new_drwg (lua_State *L) {
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
	
	if (gui->script_resume_reason != YIELD_NONE || 
		gui->script_resume ||
		!lua_isyieldable(L)
	){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* send new drawing command to main loop */
	gui->script_resume_reason = YIELD_DRWG_NEW;
	gui->action = FILE_NEW;
	
	gui->hist_new = 0;
		
	/* current script flag, indicating wait for response */
	gui->script_wait_t.wait_gui_resume = 1;
	gui->script_wait_t.T = L;
	
	/* pause script until gui response */
	lua_yieldk(L, 0, 0, &script_new_drwg_k);
	return 0;
	
}

int script_open_drwg_k (lua_State *L, int status, lua_KContext ctx) {
	/* continuation function for open drawing */
	
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
	
	/*verify if action is finished with errors */
	if (gui->script_wait_t.wait_gui_resume > 0){
		lua_pushboolean(L, 1); /* return success */
	}
	else {
		lua_pushboolean(L, 0); /* return fail */
	}
	
	gui->script_wait_t.wait_gui_resume = 0; /*clear current script flag */
	return 1;
}

/* open drawing */
/* given parameters:
	- file path, as string
	- don't add file to history, as boolean (optional)
returns:
	- boolean, success or fail

Warning: Unsaved changes in current drawing will be lost.
*/
int script_open_drwg (lua_State *L) {
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
		lua_pushliteral(L, "open_drwg: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isstring(L, 1)) { /* arguments types */
		lua_pushliteral(L, "open_drwg: incorrect argument type");
		lua_error(L);
	}
	
	if (gui->script_resume_reason != YIELD_NONE || 
		gui->script_resume ||
		!lua_isyieldable(L)
	){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char curr_path[PATH_MAX_CHARS+1];
	strncpy(curr_path, lua_tostring(L, 1), PATH_MAX_CHARS); /* preserve original string */
	
	/* send open drawing command to main loop */
	gui->script_resume_reason = YIELD_DRWG_OPEN;
	gui->action = FILE_OPEN;
	gui->path_ok = 1;
	strncpy(gui->curr_path, curr_path, PATH_MAX_CHARS);
	
	gui->hist_new = !lua_toboolean(L, 2);
	
	/* current script flag, indicating wait for response */
	gui->script_wait_t.wait_gui_resume = 1;
	gui->script_wait_t.T = L;
	
	/* pause script until gui response */
	lua_yieldk(L, 0, 0, &script_open_drwg_k);
	return 0;
	
}

int script_save_drwg_k (lua_State *L, int status, lua_KContext ctx) {
	/* continuation function for save drawing */
	
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
	
	/*verify if action is finished with errors */
	if (gui->script_wait_t.wait_gui_resume > 0){
		lua_pushboolean(L, 1); /* return success */
	}
	else {
		lua_pushboolean(L, 0); /* return fail */
	}
	
	gui->script_wait_t.wait_gui_resume = 0; /*clear current script flag */
	return 1;
}

/* save drawing */
/* given parameters:
	- file path, as string
	- don't add file to history, as boolean (optional)
returns:
	- boolean, success or fail

Warning: If the file exists, it will be overwritten
*/
int script_save_drwg (lua_State *L) {
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
		lua_pushliteral(L, "save_drwg: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isstring(L, 1)) { /* arguments types */
		lua_pushliteral(L, "save_drwg: incorrect argument type");
		lua_error(L);
	}
	
	if (gui->script_resume_reason != YIELD_NONE || 
		gui->script_resume ||
		!lua_isyieldable(L)
	){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	char curr_path[PATH_MAX_CHARS+1];
	strncpy(curr_path, lua_tostring(L, 1), PATH_MAX_CHARS); /* preserve original string */
	
	/* send save drawing command to main loop */
	gui->script_resume_reason = YIELD_DRWG_SAVE;
	gui->action = FILE_SAVE;
	gui->path_ok = 1;
	strncpy(gui->curr_path, curr_path, PATH_MAX_CHARS);
	
	gui->hist_new = !lua_toboolean(L, 2);
	
	/* current script flag, indicating wait for response */
	gui->script_wait_t.wait_gui_resume = 1;
	gui->script_wait_t.T = L;
	
	/* pause script until gui response */
	lua_yieldk(L, 0, 0, &script_save_drwg_k);
	return 0;
	
}

/* generate a printable file from current drawing */
/* given parameters:
	- output path, as string (file extension will determine the format)
	- page width, as number
	- page height, as number
	- units, as string ("mm", "in", "px", optional - default="mm")
	- drawing scale, as number (optional - default=fit all)
	- drawing offset x, as number (optional - default=centralize)
	- drawing offset y, as number (optional - default=centralize)
returns:
	- Success/fail, as boolean 
*/
int script_print_drwg (lua_State *L) {
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
		lua_pushliteral(L, "print_drwg: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "print_drwg: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "print_drwg: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 3)) {
		lua_pushliteral(L, "print_drwg: incorrect argument type");
		lua_error(L);
	}
	
	static struct print_param param;
	
	char path[PATH_MAX_CHARS+1];
	strncpy(path, lua_tostring(L, 1), PATH_MAX_CHARS); /* preserve original string */
	
	param.out_fmt = PRT_NONE;
	
	char *fmt = get_ext(path);
	str_upp(fmt);
	
	if(strcmp(fmt, "PDF") == 0) param.out_fmt = PRT_PDF;
	else if(strcmp(fmt, "SVG") == 0) param.out_fmt = PRT_SVG;
	else if(strcmp(fmt, "PS") == 0) param.out_fmt = PRT_PS;
	else if(strcmp(fmt, "PNG") == 0) param.out_fmt = PRT_PNG;
	else if(strcmp(fmt, "JPG") == 0) param.out_fmt = PRT_JPG;
	else if(strcmp(fmt, "BMP") == 0) param.out_fmt = PRT_BMP;
	else {
		lua_pushboolean (L, 0); /* fail */
		return 1;
	}
	
	double page_w = lua_tonumber(L, 2);
	double page_h = lua_tonumber(L, 3);
	
	double ofs_x, ofs_y, scale;
	double min_x, min_y, min_z, max_x, max_y, max_z;
	double zoom_x, zoom_y;
	
	int unit = PRT_MM;
	/* get units, if exist*/
	if (lua_isstring(L, 4)) {
		char un[10];
		strncpy(un, lua_tostring(L, 4), 9);
		str_upp(un);
		char *new_un = trimwhitespace(un);
		
		if (strcmp(new_un, "MM") == 0){
			unit = PRT_MM;
		}
		else if (strcmp(new_un, "IN") == 0){
			unit = PRT_IN;
		}
		else if (strcmp(new_un, "PX") == 0){
			unit = PRT_PX;
		}
	}
	
	/* get scale, if exist*/
	if (lua_isnumber(L, 5)) {
		scale = fabs(lua_tonumber(L, 5));
		/* get ofset x, if exist*/
		if (lua_isnumber(L, 6)) {
			ofs_x = lua_tonumber(L, 6);
			/* get ofset y, if exist*/
			if (lua_isnumber(L, 7)) {
				ofs_y = lua_tonumber(L, 7);
			}
			else {
				/* get drawing extents */
				dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
				ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
			}
		}
		else{
			/* get drawing extents */
			dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
			ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
			ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
		}
	}
	else {
		/* get drawing extents */
		dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
		
		/* get the better scale to fit in width or height */
		zoom_x = fabs(max_x - min_x)/page_w;
		zoom_y = fabs(max_y - min_y)/page_h;
		scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
		scale = 1/scale;
		
		/* get origin */
		ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
		ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
	}
	
	param.w = page_w;
	param.h = page_h;
	param.scale = scale;
	param.ofs_x = ofs_x;
	param.ofs_y = ofs_y;
	param.mono = 0;
	param.unit = unit;
	
	/* basic colors */
	bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
	bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
	/* default substitution list (display white -> print black) */
	bmp_color list[] = { white, };
	bmp_color subst[] = { black, };
	param.list = list;
	param.subst = subst;
	param.len = 1;
	
	int ret = 0;
	if (param.out_fmt == PRT_PDF)
		ret = print_pdf(gui->drawing, param, path);
	else if (param.out_fmt == PRT_SVG)
		ret = print_svg(gui->drawing, param, path);
	else if (param.out_fmt == PRT_PNG ||
		param.out_fmt == PRT_JPG ||
		param.out_fmt == PRT_BMP)
		ret = print_img(gui->drawing, param, path);
	else if (param.out_fmt == PRT_PS)
		ret = print_ps(gui->drawing, param, path);
	/* verify success or fail*/
	lua_pushboolean (L, ret);
	
	return 1;
}

struct script_pdf{
	struct pdf_doc *pdf;
	struct print_param param;
};

/* start a new pdf file */
/* given parameters:
	- page width, as number
	- page height, as number
	- units, as string ("mm", "in", "px", optional - default="mm")
	- drawing scale, as number (optional - default=fit all)
	- drawing offset x, as number (optional - default=centralize)
	- drawing offset y, as number (optional - default=centralize)
returns:
	- a pdf object, or nil if fail
*/
int script_pdf_new(lua_State *L){
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "pdf_new: invalid number of arguments");
		lua_error(L);
	}
	if (!lua_isnumber(L, 1)) {
		lua_pushliteral(L, "pdf_new: incorrect argument type");
		lua_error(L);
	}
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "pdf_new: incorrect argument type");
		lua_error(L);
	}
	
	struct script_pdf *pdf;
	
	/* create a userdata object */
	pdf = (struct script_pdf *) lua_newuserdatauv(L, sizeof(struct script_pdf), 0); 
	luaL_getmetatable(L, "cz_pdf_obj");
	lua_setmetatable(L, -2);
	
	/* init */
	/* resolution -> multiplier factor over integer units in pdf */
	pdf->param.resolution = 20;
	pdf->param.out_fmt = PRT_PDF;
	pdf->param.w = lua_tonumber(L, 1);
	pdf->param.h = lua_tonumber(L, 2);
		
	pdf->param.unit = PRT_MM;
	double mul = 72.0 / 25.4; /* multiplier to fit pdf parameters in final output units */
	/* get units, if exist*/
	if (lua_isstring(L, 3)) {
		char un[10];
		strncpy(un, lua_tostring(L, 3), 9);
		str_upp(un);
		char *new_un = trimwhitespace(un);
		
		if (strcmp(new_un, "MM") == 0){
			pdf->param.unit = PRT_MM;
			mul = 72.0 / 25.4;
		}
		else if (strcmp(new_un, "IN") == 0){
			pdf->param.unit = PRT_IN;
			mul = 72.0;
		}
		else if (strcmp(new_un, "PX") == 0){
			pdf->param.unit = PRT_PX;
			mul = 72.0/96.0;
		}
	}
	
	pdf->param.scale = -1;
	pdf->param.ofs_x = NAN;
	pdf->param.ofs_y = NAN;
	/* get scale, if exist*/
	if (lua_isnumber(L, 4)) {
		pdf->param.scale = fabs(lua_tonumber(L, 4));
		/* get ofset x, if exist*/
		if (lua_isnumber(L, 5)) {
			pdf->param.ofs_x = lua_tonumber(L, 5);
			/* get ofset y, if exist*/
			if (lua_isnumber(L, 6)) {
				pdf->param.ofs_y = lua_tonumber(L, 6);
			}
		}
	}
	pdf->param.mono = 0;
	
	/*--------------- assemble the pdf structure to a file */
	/* pdf info header */
	struct pdf_info info = {
		.creator = "CadZinho",
		.producer = "CadZinho",
		.title = "Batch Print",
		.author = "",
		.subject = "Batch Print",
		.date = ""
	};
	/* pdf main struct */
	pdf->pdf = pdf_create((int)(mul * pdf->param.w + 0.5), (int)(mul * pdf->param.h + 0.5), &info);
	
	return 1;
}

/* print current drawing and add page to a pdf objet */
/* given parameters:
	- a pdf object (this function is called as method inside object)
	optional parameters (default = params in pdf object)
	- page width, as number
	- page height, as number
	- units, as string 
	- drawing scale, as number
	- drawing offset x, as number
	- drawing offset y, as number
returns:
	- a boolean indicating success or fail
*/
int script_pdf_page(lua_State *L){
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
	
	struct script_pdf *pdf;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "page: invalid number of arguments");
		lua_error(L);
	}
	if (!( pdf =  luaL_checkudata(L, 1, "cz_pdf_obj") )) { /* the pdf object is a Lua userdata type*/
		lua_pushliteral(L, "page: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, pdf->pdf != NULL, 1, "pdf is closed");

	
	
	/* buffer to hold the pdf commands from drawing convertion */
	static struct txt_buf buf;
	struct Mem_buffer *mem1 = manage_buffer(PDF_BUF_SIZE + 1, BUF_GET, 2);
	if (!mem1) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	buf.data = mem1->buffer;
	
	buf.pos = 0; /* init buffer */
	
	static struct print_param param;
	double page_w = pdf->param.w;
	double page_h = pdf->param.h;
	
	double ofs_x, ofs_y, scale;
	double min_x, min_y, min_z, max_x, max_y, max_z;
	double zoom_x, zoom_y;
	
	/* get page size, if exist*/
	if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
		page_w = lua_tonumber(L, 2);
		page_h = lua_tonumber(L, 3);
	}
		
	int unit = pdf->param.unit;
	double mul = 72.0 / 25.4; /* multiplier to fit pdf parameters in final output units */
	/* get units, if exist*/
	if (lua_isstring(L, 4)) {
		char un[10];
		strncpy(un, lua_tostring(L, 4), 9);
		str_upp(un);
		char *new_un = trimwhitespace(un);
		
		if (strcmp(new_un, "MM") == 0){
			unit = PRT_MM;
			mul = 72.0 / 25.4;
		}
		else if (strcmp(new_un, "IN") == 0){
			unit = PRT_IN;
			mul = 72.0;
		}
		else if (strcmp(new_un, "PX") == 0){
			unit = PRT_PX;
			mul = 72.0/96.0;
		}
	}
	
	scale = pdf->param.scale;
	ofs_x = pdf->param.ofs_x;
	ofs_y = pdf->param.ofs_y;
	/* get scale, if exist*/
	if (lua_isnumber(L, 5)) {
		scale = fabs(lua_tonumber(L, 5));
		/* get ofset x, if exist*/
		if (lua_isnumber(L, 6)) {
			ofs_x = lua_tonumber(L, 6);
			/* get ofset y, if exist*/
			if (lua_isnumber(L, 7)) {
				ofs_y = lua_tonumber(L, 7);
			}
		}
	}
	
	/* get scale, if exist*/
	if (scale > 0) {
		if (!isnan(ofs_x)) {
			if (isnan(ofs_y)){
				/* get drawing extents */
				dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
				ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
			}
		}
		else{
			/* get drawing extents */
			dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
			ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
			ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
		}
	}
	else {
		/* get drawing extents */
		dxf_ents_ext(gui->drawing, &min_x, &min_y, &min_z, &max_x, &max_y, &max_z);
		
		/* get the better scale to fit in width or height */
		zoom_x = fabs(max_x - min_x)/page_w;
		zoom_y = fabs(max_y - min_y)/page_h;
		scale = (zoom_x > zoom_y) ? zoom_x : zoom_y;
		scale = 1/scale;
		
		/* get origin */
		ofs_x = min_x - (fabs((max_x - min_x) * scale - page_w)/2) / scale;
		ofs_y = min_y - (fabs((max_y - min_y) * scale - page_h)/2) / scale;
	}
	
	param.w = page_w;
	param.h = page_h;
	param.scale = scale;
	param.ofs_x = ofs_x;
	param.ofs_y = ofs_y;
	param.mono = pdf->param.mono;
	param.unit = unit;
	
	
	/* basic colors */
	bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
	bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
	/* default substitution list (display white -> print black) */
	bmp_color list[] = { white, };
	bmp_color subst[] = { black, };
	param.list = list;
	param.subst = subst;
	param.len = 1;
	
	/* resolution -> multiplier factor over integer units in pdf */
	param.resolution = 20;
	
	/* multiplier to fit pdf parameters in final output units */
	param.w = mul * param.w + 0.5;
	param.h = mul * param.h + 0.5;
	param.scale *= mul;
	
	
	/* fill buffer with pdf drawing commands */
	print_ents_pdf(gui->drawing, &buf, param);
	
	/* -------------- compress the command buffer stream (deflate algorithm)*/
	int cmp_status;
	long src_len = strlen(buf.data);
	long cmp_len = compressBound(src_len);
	/* Allocate buffers to hold compressed and uncompressed data. */
	struct Mem_buffer *mem2 = manage_buffer(cmp_len, BUF_GET, 3);
	if (!mem2) {
		manage_buffer(0, BUF_RELEASE, 2);
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	mz_uint8 *pCmp = (mz_uint8 *) mem2->buffer;
	
	/* Compress buffer string. */
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf.data, src_len);
	if (cmp_status != Z_OK){
		manage_buffer(0, BUF_RELEASE, 3);
		manage_buffer(0, BUF_RELEASE, 2);
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	/*-------------------------*/
	
	/* add  the drawing images in pdf */
	if (gui->drawing->img_list != NULL){
		
		list_node *list = gui->drawing->img_list;
		
		struct dxf_img_def * img_def;
		bmp_img * img;
		
		list_node *current = list->next;
		while (current != NULL){ /* sweep the image list */
			if (current->data){
				img_def = (struct dxf_img_def *)current->data;
				img = img_def->img;
				if (img){
					/* convert and append image data*/
					print_img_pdf(pdf->pdf, img);
				}
			}
			current = current->next;
		}
	}
	/*---------------------*/
	
	/* add a page to pdf file */
	struct pdf_object *page = pdf_append_page(pdf->pdf);
	
	pdf_page_set_size(pdf->pdf, page, param.w, param.h);
	
	/* add the print object to pdf page */
	//pdf_add_stream(pdf, page, buf->data); /* non compressed stream */
	pdf_add_stream_zip(pdf->pdf, page, pCmp, cmp_len); /* compressed stream */
	
	manage_buffer(0, BUF_RELEASE, 3);
	manage_buffer(0, BUF_RELEASE, 2);
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* save a pdf object to file */
/* given parameters:
	- a pdf object (this function is called as method inside object)
	- path to file, as string
returns:
	- a boolean indicating success or fail
*/
int script_pdf_save (lua_State *L) {
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "save: invalid number of arguments");
		lua_error(L);
	}
	
	struct script_pdf *pdf;
	
	if (!( pdf =  luaL_checkudata(L, 1, "cz_pdf_obj") )) { /* the pdf object is a Lua userdata type*/
		lua_pushliteral(L, "save: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, pdf->pdf != NULL, 1, "pdf is closed");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	/* save pdf file */ 
	int e = pdf_save(pdf->pdf, lua_tostring(L, 2));
	
	lua_pushboolean(L, !e); /* return success or fail */
	return 1;
}

/* close a previouly opened pdf object */
/* given parameters:
	- a pdf object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int script_pdf_close(lua_State *L){
	
	struct script_pdf *pdf;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( pdf =  luaL_checkudata(L, 1, "cz_pdf_obj") )) { /* the pdf object is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, pdf->pdf != NULL, 1, "pdf is closed");

	/* close pdf */
	pdf_destroy(pdf->pdf);
	pdf->pdf = NULL;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

int script_gui_refresh_k (lua_State *L, int status, lua_KContext ctx) {
	/* continuation function for gui refresh */
	
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
	
	/*verify if action is finished with errors */
	if (gui->script_wait_t.wait_gui_resume > 0){
		lua_pushboolean(L, 1); /* return success */
	}
	else {
		lua_pushboolean(L, 0); /* return fail */
	}
	
	gui->script_wait_t.wait_gui_resume = 0; /*clear current script flag */
	return 1;
}

/* refresh gui */
/* given parameters:
	- none
returns:
	- boolean, success or fail
*/
int script_gui_refresh (lua_State *L) {
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
	
	if (gui->script_resume_reason != YIELD_NONE || 
		gui->script_resume ||
		!lua_isyieldable(L)
	){
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	/* current script flag, indicating wait for response */
	gui->script_wait_t.wait_gui_resume = 1;
	gui->script_wait_t.T = L;
	
	/* pause script until gui response */
	gui->script_resume_reason = YIELD_GUI_REFRESH;
	lua_yieldk(L, 0, 0, &script_gui_refresh_k);
	return 0;
	
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
/* using Nuklear immediate GUI functions */


/* show a user window */
/* given parameters:
	- Lua function name (to call in main loop), as string
	- window title, as string
	- x, y position (left up corner) as numbers
	- width, height as numbers
returns:
	- boolean, success or fail
*/
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

/* close a user window */
/* given parameters:
	- none (this call close the window inside current thread)
returns:
	- none
*/
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

/* layout the GUI objects in window */
/* given parameters:
	- line height as number
	- number of columns as number
returns:
	- none
*/
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	nk_layout_row_dynamic(gui->ctx, lua_tonumber(L, 1), lua_tonumber(L, 2));
	
	return 0;
}

/* GUI button object */
/* given parameters:
	- Button text, as string
returns:
	- button pressed, as boolean,
*/
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int ret = nk_button_label(gui->ctx, lua_tostring(L, 1));
	
	lua_pushboolean(L, ret); /* return button status */
	
	return 1;
}

/* GUI image button object */
/* given parameters:
	- Button image, as user value
returns:
	- button pressed, as boolean,
*/
int script_nk_button_img (lua_State *L) {
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
		lua_pushliteral(L, "nk_button_img: invalid number of arguments");
		lua_error(L);
	}
	
  struct script_rast_image * img_obj;
  
  if (!( img_obj = luaL_checkudata(L, 1, "Rast_img") )) { /* the Rast_img object is a Lua userdata type*/
		lua_pushliteral(L, "nk_button_img: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int ret = nk_button_image_styled(gui->ctx, &gui->b_icon, nk_image_ptr(img_obj->img));
	
	lua_pushboolean(L, ret); /* return button status */
	
	return 1;
}

/* GUI label object */
/* given parameters:
	- label text, as string
returns:
	- none
*/
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	nk_label(gui->ctx, lua_tostring(L, 1), NK_TEXT_LEFT);
	return 0;
}

/* GUI edit object (text entry) */
/* given parameters:
	- Text to modify (by reference), as table with a "value" named field
returns:
	- Enter signal, as boolean (text is updated in passed table "value" key)
*/
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	const char *value = lua_tostring(L, -1);
	lua_pop(L, 1);
	char buff[DXF_MAX_CHARS+1];
	strncpy(buff, value, DXF_MAX_CHARS);
	nk_flags res = nk_edit_string_zero_terminated(gui->ctx, 
		NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD,
		buff, DXF_MAX_CHARS, nk_filter_default);
	lua_pushstring(L, buff);
	lua_setfield(L, -2, "value");
	lua_pop(L, 1);
	
	lua_pushboolean(L, res & NK_EDIT_COMMITED); /* return ENTER signal */
	return 1;
}

/* GUI integer property object (number entry) */
/* given parameters:
	- name, as string
	- number to modify (by reference), as table with a "value" named field (integer)
	- min, as integer (optional - default=negative large number)
	- max, as integer (optional - default=positive large number)
	- step, as integer (optional - default=1)
returns:
	- none (number is updated in passed table "value" key)
*/
int script_nk_propertyi (lua_State *L) {
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
		lua_pushliteral(L, "nk_propertyi: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_propertyi: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_istable(L, 2)) {
		lua_pushliteral(L, "nk_propertyi: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 2, "value");
	if (!lua_isnumber(L, -1)) {
		lua_pushliteral(L, "nk_propertyi: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int val = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* get min, if exist*/
	int min = INT_MIN;
	if (lua_isinteger(L, 3)) {
		min = lua_tointeger(L, 3);
	}
	
	/* get max, if exist*/
	int max = INT_MAX;
	if (lua_isinteger(L, 4)) {
		max = lua_tointeger(L, 4);
	}
	
	/* get step, if exist*/
	int step = 1;
	if (lua_isinteger(L, 5)) {
		step = lua_tointeger(L, 5);
	}
	
	int ret = nk_propertyi(gui->ctx, lua_tostring(L, 1), min, val, max, step, (float) step);
	
	lua_pushinteger(L, ret); /* return value */
	lua_setfield(L, 2, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI property object (number entry - double) */
/* given parameters:
	- name, as string
	- number to modify (by reference), as table with a "value" named field (number)
	- min, as number (optional - default=negative large number)
	- max, as number (optional - default=positive large number)
	- step, as number (optional - default=proportional to current value)
returns:
	- none (number is updated in passed table "value" key)
*/
int script_nk_propertyd (lua_State *L) {
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
		lua_pushliteral(L, "nk_propertyd: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_propertyd: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_istable(L, 2)) {
		lua_pushliteral(L, "nk_propertyd: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 2, "value");
	if (!lua_isnumber(L, -1)) {
		lua_pushliteral(L, "nk_propertyd: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	double val = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	/* get min, if exist*/
	double min = LONG_MIN;
	if (lua_isnumber(L, 3)) {
		min = lua_tonumber(L, 3);
	}
	
	/* get max, if exist*/
	double max = LONG_MAX;
	if (lua_isnumber(L, 4)) {
		max = lua_tonumber(L, 4);
	}
	
	/* get step, if exist*/
	double step = SMART_STEP(val);
	if (lua_isnumber(L, 5)) {
		step = lua_tonumber(L, 5);
	}
	
	double ret = nk_propertyd(gui->ctx, lua_tostring(L, 1), min, val, max, step, (float) step);
	
	lua_pushnumber(L, ret); /* return value */
	lua_setfield(L, 2, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI combo object (list selectable) */
/* given parameters:
	- item list (by reference), as table with one named field "value"
	- box width, as number (optional - default=150)
	- box height, as number (optional - default=150)
returns:
	- none (current selected item index is updated in passed table "value" key)
*/
int script_nk_combo (lua_State *L) {
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
		lua_pushliteral(L, "nk_combo: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) {
		lua_pushliteral(L, "nk_combo: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 1, "value");
	if (!lua_isinteger(L, -1)){
		lua_pushliteral(L, "nk_combo: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int idx = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	int items = lua_rawlen(L, 1); /* get number of numbered items in table */
	
	if (items < 1){ /* no items */
		return 0;
	}
	
	if (idx > items) idx = 1; /* assert select item */
	
	/* get box width, if exist*/
	int bw = 150;
	if (lua_isnumber(L, 2)) {
		bw = lua_tonumber(L, 2);
	}
	
	/* get box heigth, if exist*/
	int bh = 150;
	if (lua_isnumber(L, 3)) {
		bh = lua_tonumber(L, 3);
	}
	
	int i;
	/* assert height of combo box */
	int h = items * 25 + 5;
	h = (h < bh) ? h : bh;
	
	lua_geti(L, 1, idx); /* current item selected */
	if(nk_combo_begin_label(gui->ctx, lua_tostring(L, -1), nk_vec2(bw, h))){
		/* show each item */
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		for (i = 1; i<= items; i++){
			lua_geti(L, 1, i);
			if (nk_combo_item_label(gui->ctx, lua_tostring(L, -1), NK_TEXT_LEFT)){
				idx = i; /* select item */
			}
			lua_pop(L, 1);
		}
		
		nk_combo_end(gui->ctx);
	}
	lua_pop(L, 1);
	
	/* update "value" in table */
	lua_pushinteger(L, idx);
	lua_setfield(L, 1, "value");
	lua_pop(L, 1);
	
	return 0;
}


/* GUI integer slide object (kind of number entry) */
/* given parameters:
	- number to modify (by reference), as table with a "value" named field (integer)
	- min, as integer
	- max, as integer
	- step, as integer (optional - default=1)
returns:
	- none (number is updated in passed table "value" key)
*/
int script_nk_slide_i (lua_State *L) {
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
		lua_pushliteral(L, "nk_slide_i: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) {
		lua_pushliteral(L, "nk_slide_i: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 1, "value");
	if (!lua_isnumber(L, -1)) {
		lua_pushliteral(L, "nk_slide_i: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "nk_slide_i: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 3)) {
		lua_pushliteral(L, "nk_slide_i: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int val = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* get min, max*/
	int min = lua_tointeger(L, 2);
	int max = lua_tointeger(L, 3);
	
	/* get step, if exist*/
	int step = 1;
	if (lua_isinteger(L, 4)) {
		step = lua_tointeger(L, 4);
	}
	
	int ret = nk_slide_int(gui->ctx, min, val, max, step);
	
	lua_pushinteger(L, ret); /* return value */
	lua_setfield(L, 1, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI float slide object (kind of number entry) */
/* given parameters:
	- number to modify (by reference), as table with a "value" named field (number)
	- min, as number
	- max, as number
	- step, as number (optional - default=1)
returns:
	- none (number is updated in passed table "value" key)
*/
int script_nk_slide_f (lua_State *L) {
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
		lua_pushliteral(L, "nk_slide_f: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) {
		lua_pushliteral(L, "nk_slide_f: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 1, "value");
	if (!lua_isnumber(L, -1)) {
		lua_pushliteral(L, "nk_slide_f: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "nk_slide_f: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 3)) {
		lua_pushliteral(L, "nk_slide_f: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	float val = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	/* get min, max*/
	float min = lua_tonumber(L, 2);
	float max = lua_tonumber(L, 3);
	
	/* get step, if exist*/
	float step = 1;
	if (lua_isnumber(L, 4)) {
		step = lua_tonumber(L, 4);
	}
	
	float ret = nk_slide_float(gui->ctx, min, val, max, step);
	
	lua_pushnumber(L, ret); /* return value */
	lua_setfield(L, 1, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI option object (list selectable) */
/* given parameters:
	- item list (by reference), as table with one named field "value"
returns:
	- none (current selected item index is updated in passed table "value" key)
*/
int script_nk_option (lua_State *L) {
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
		lua_pushliteral(L, "nk_option: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_istable(L, 1)) {
		lua_pushliteral(L, "nk_option: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 1, "value");
	if (!lua_isinteger(L, -1)){
		lua_pushliteral(L, "nk_option: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int idx = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	int items = lua_rawlen(L, 1); /* get number of numbered items in table */
	
	if (items < 1){ /* no items */
		return 0;
	}
	
	if (idx > items) idx = 1; /* assert select item */
	
	int i;
	
	/* show each item */
	for (i = 1; i<= items; i++){
		lua_geti(L, 1, i);
		if (nk_option_label(gui->ctx, lua_tostring(L, -1), idx == i)) idx = i; /* select item */
		lua_pop(L, 1);
	}
	
	/* update "value" in table */
	lua_pushinteger(L, idx);
	lua_setfield(L, 1, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI integer check object (boolean entry) */
/* given parameters:
	- name, as string
	- value to modify (by reference), as table with a "value" named field (boolean)
returns:
	- none (boolean is updated in passed table "value" key)
*/
int script_nk_check (lua_State *L) {
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
		lua_pushliteral(L, "nk_check: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_check: incorrect argument type");
		lua_error(L);
	}
	
	
	if (!lua_istable(L, 2)) {
		lua_pushliteral(L, "nk_check: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 2, "value");
	if (!lua_isboolean(L, -1)) {
		lua_pushliteral(L, "nk_check: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int val = lua_toboolean(L, -1);
	lua_pop(L, 1);
	
	val = nk_check_label(gui->ctx, lua_tostring(L, 1), val);
	
	lua_pushboolean(L, val); /* return value */
	lua_setfield(L, 2, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI selectable object */
/* given parameters:
	- Label, as string
	- value to modify (by reference), as table with a "value" named field (boolean)
returns:
	- none (boolean is updated in passed table "value" key)
*/
int script_nk_selectable (lua_State *L) {
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
		lua_pushliteral(L, "nk_selectable: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_selectable: incorrect argument type");
		lua_error(L);
	}
	
	
	if (!lua_istable(L, 2)) {
		lua_pushliteral(L, "nk_selectable: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 2, "value");
	if (!lua_isboolean(L, -1)) {
		lua_pushliteral(L, "nk_selectable: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int val = lua_toboolean(L, -1);
	lua_pop(L, 1);
	
	val = nk_select_label(gui->ctx, lua_tostring(L, 1), NK_TEXT_LEFT, val);
	
	lua_pushboolean(L, val); /* return value */
	lua_setfield(L, 2, "value");
	lua_pop(L, 1);
	
	return 0;
}

/* GUI progress bar object */
/* given parameters:
	- current value, as integer
	- max, as integer
returns:
	- none 
*/
int script_nk_progress (lua_State *L) {
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
		lua_pushliteral(L, "nk_progress: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 1)) {
		lua_pushliteral(L, "nk_progress: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_isnumber(L, 2)) {
		lua_pushliteral(L, "nk_progress: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	int val = lua_tointeger(L, 1);
	
	/* get max*/
	int max = lua_tointeger(L, 2);
	
	nk_prog(gui->ctx, val, max, 0);
	
	return 0;
}

/* begin a GUI group */
/* given parameters:
	- name, as string
	- show title flag (optional, default=false)
	- show border flag (optional, default=false)
	- scrollbars flag (optional, default=false)
returns:
	- boolean, `true` if visible and fillable with widgets or `false` otherwise
*/
int script_nk_group_begin (lua_State *L) {
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
		lua_pushliteral(L, "nk_group: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_group: incorrect argument type");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	//char id[32];
	//snprintf(id, 31, "script_gr_%d%d", script->groups, rand() % 100);
	//snprintf(id, 31, "script_gr_%d", script->groups);
	
	nk_flags flags =  0;
	if (lua_toboolean(L, 2)) flags |= NK_WINDOW_TITLE;
	if (lua_toboolean(L, 3)) flags |= NK_WINDOW_BORDER;
	if (!lua_toboolean(L, 4)) flags |= NK_WINDOW_NO_SCROLLBAR;
	
	//int ret = nk_group_begin_titled(gui->ctx, (const char *)id, (const char *)lua_tostring(L, 1), flags);
	int ret = nk_group_begin(gui->ctx, (const char *)lua_tostring(L, 1), flags);
	
	
	if (ret) script->groups ++;
	
	lua_pushboolean(L, ret); /* return value */
	
	return 1;
}

/* end a GUI group */
/* given parameters:
	- none (this call ends current group)
returns:
	- none
*/
int script_nk_group_end (lua_State *L) {
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	if (script->groups > 0){ /* check if exists pending nk_group */
		nk_group_end(gui->ctx);
		script->groups --;
	}
	return 0;
}

/* GUI tab obj object (kind of container with tabs id) */
/* given parameters:
	- name, as string
	- tab list (by reference), as table with one named field "value"
returns:
	- boolean, `true` if visible and fillable with widgets or `false` otherwise
	  (current selected tab index is updated in passed table "value" key)
*/
int script_nk_tab_begin (lua_State *L) {
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
		lua_pushliteral(L, "nk_tab: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "nk_tab: incorrect argument type");
		lua_error(L);
	}
	
	if (!lua_istable(L, 2)) {
		lua_pushliteral(L, "nk_tab: incorrect argument type");
		lua_error(L);
	}
	lua_getfield(L, 2, "value");
	if (!lua_isinteger(L, -1)){
		lua_pushliteral(L, "nk_tab: incorrect argument type");
		lua_error(L);
	}
	int idx = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (!script){ /* error in script object access */
		lua_pushstring(L, "Auto check: no access to CadZinho script object");
		lua_error(L);
	}
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	
	
	int items = lua_rawlen(L, 2); /* get number of numbered items in table */
	
	if (items < 1){ /* no items */
		return 0;
	}
	
	if (idx > items) idx = 1; /* assert select item */
	
	int i;
	
	float h = nk_widget_height(gui->ctx);
	
	if (h < 40.0){
		/* there is not enough space to draw*/
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	nk_flags flags =  NK_WINDOW_NO_SCROLLBAR;
	
	char id[DXF_MAX_CHARS+1];
	snprintf(id, DXF_MAX_CHARS, "%s%d", lua_tostring(L, 1), script->groups);
	int ret = nk_group_begin(gui->ctx, (const char *)id, flags);
	if(!ret){
		/* not able to get widgets*/
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	script->groups ++;
	
	/* show tabs - header */
	nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
	nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 100); /* until 100 tabs */
	for (i = 1; i<= items; i++){
		lua_geti(L, 2, i);
		if (gui_tab (gui, lua_tostring(L, -1), idx == i)) idx = i; /* select item */
		lua_pop(L, 1);
	}
	nk_style_pop_vec2(gui->ctx);
	nk_layout_row_end(gui->ctx);
	
	/* update "value" in table */
	lua_pushinteger(L, idx);
	lua_setfield(L, 2, "value");
	lua_pop(L, 1);
	
	nk_layout_row_dynamic(gui->ctx, h - 26, 1); /* space to widgets*/
	flags =  NK_WINDOW_BORDER;
	snprintf(id, DXF_MAX_CHARS, "%s%d", lua_tostring(L, 1), script->groups);
	ret = nk_group_begin(gui->ctx, (const char *)id, flags);
	
	if (ret) script->groups ++;
	
	lua_pushboolean(L, ret); /* return value */
	
	return 1;
}

/* end a GUI tab */
/* given parameters:
	- none (this call ends current tab)
returns:
	- none
*/
int script_nk_tab_end (lua_State *L) {
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
	
	if (!gui->ctx->current) return 0; /* verifiy if has current window - prevent crash in nuklear */
	
	if (script->groups > 0){ /* check if exists pending nk_group */
		nk_group_end(gui->ctx);
		script->groups --;
	}
	
	if (script->groups > 0){ /* check if exists pending nk_group - yes: twice*/
		nk_group_end(gui->ctx);
		script->groups --;
	}
	return 0;
}


/* ========= MINIZ ===============================================*/


struct script_miniz_arch {
	mz_zip_archive *archive;
};

/* open a compressed file (zip) to read content */
/* given parameters:
	- path to compressed file, as string
returns:
	- a Zip object (use methods to read and close), or nil if fail
*/
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

/* close a previouly opened compressed file (zip) */
/* given parameters:
	- a Zip object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int script_miniz_close (lua_State *L) {
	
	struct script_miniz_arch * zip;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( zip =  luaL_checkudata(L, 1, "Zip") )) { /* the Zip object is a Lua userdata type*/
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

/* read an archive inside a compressed file (zip) */
/* given parameters:
	- a Zip object (this function is called as method inside object)
	- name of archive, as string
returns:
	-  a buffer with archive contents (string), or nil if fail
*/
int script_miniz_read (lua_State *L) {
	struct script_miniz_arch * zip;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "read: invalid number of arguments");
		lua_error(L);
	}
	if (!( zip =  luaL_checkudata(L, 1, "Zip") )) { /* the Zip object is a Lua userdata type*/
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

/* write to an archive inside a compressed file (zip) */
/* given parameters:
	- path to compressed file, as string
	- name of archive, as string
	- buffer with archive contents, as string
	- a comment, as string (optional)
returns:
	- a boolean indicating success or fail
*/
int script_miniz_write(lua_State *L) {
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 3){
		lua_pushliteral(L, "write: invalid number of arguments");
		lua_error(L);
	}
	int i;
	for (i = 1; i <= n; i++){
		if (!lua_isstring(L, i)) {
			lua_pushliteral(L, "write: incorrect argument type");
			lua_error(L);
		}
	}
	
	mz_bool ok = 0;
	
	const char *zip = lua_tostring(L, 1);
	const char *arch = lua_tostring(L, 2);
	const char *buf = lua_tostring(L, 3);
	const char *comment = "";
	
	if (n > 3) comment = lua_tostring(L, 4);
	
	int buf_sz = strlen(buf);
	int com_sz = strlen(comment);
	
	ok =  mz_zip_add_mem_to_archive_file_in_place(zip, arch, buf, buf_sz, comment, com_sz, MZ_DEFAULT_COMPRESSION);
	
	if (ok) lua_pushboolean(L, 1); /* return success */
	else lua_pushboolean(L, 0); /* return fail */
	
	/* return success */
	return 1;
}


/* ==================== YXML ===========================*/

#define BUFSIZE 4096

struct script_yxml_state {
	yxml_t *x;
};

/* Creates a XML parser object */
/* given parameters:
	- none
returns:
	- a Yxml object (use methods to read and close), or nil if fail
*/
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

/* close a XML parser object */
/* given parameters:
	- a Yxml object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int script_yxml_close (lua_State *L) {
	
	struct script_yxml_state * state;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( state =  luaL_checkudata(L, 1, "Yxml") )) { /* the Yxml object is a Lua userdata type*/
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

/* parse a XML stream*/
/* given parameters:
	- a Yxml parser object (this function is called as method inside object)
	- XML stream, as string
returns:
	-  a table, or nil if fail
NOTES: inside main table are created sub-tables for each XML element, with:
	- element name string, with key "id"
	- atttribute table, with key "attr" (each atrribute's value string stored in table, with its name as key)
	- contents table, with key "cont" (can store strings or element's tables, as see above)
*/
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
	
  /* buffer to store parcial strings */
	static struct txt_buf buf;
	struct Mem_buffer *mem1 = manage_buffer(PDF_BUF_SIZE + 1, BUF_GET, 1);
	if (!mem1) {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	buf.data = mem1->buffer;
	buf.pos = 0; buf.data[0] = 0; /* init buffer */
  
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
				/* reset flags */
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
				if (content){
          lua_pushstring(L, buf.data);
					if (lua_istable(L, -2)){
						lua_pushstring(L, "cont");
						lua_insert (L, lua_gettop(L) - 1); /* setup Lua stack to next operation */
						lua_rawset(L, -3);
					}
					else {
						lua_pop(L, 1);
					}
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
					
          buf.pos = 0; buf.data[0] = 0; /* zero buffer */
				}
				/* store parcial string */
        buf.pos +=snprintf(buf.data + buf.pos,
          PDF_BUF_SIZE - buf.pos, state->x->data);
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
					
          buf.pos = 0; buf.data[0] = 0; /* zero buffer */
				}
				/* store parcial string */
        buf.pos +=snprintf(buf.data + buf.pos,
          PDF_BUF_SIZE - buf.pos, state->x->data);
				break;
			case YXML_ATTREND:
				/* Now we have a full attribute. Its name is in x->attr, 
				and its value is in Lua buffer "b". */
				if (attrval){
					attrval = 0;
          lua_pushstring(L, buf.data);
					if (lua_istable(L, -2)){
						/* store in its owner table, where its name is the key */
						lua_pushstring(L, state->x->attr);
						lua_insert (L, lua_gettop(L) - 1); /* setup Lua stack to next operation */
						lua_rawset(L, -3);
					}
				}
				break;
		}
	}
  manage_buffer(0, BUF_RELEASE, 1);
  
	/* end parsing */
	yxml_eof(state->x);
	/* restart parser */
	yxml_init(state->x, state->x+1, BUFSIZE);
	
	if (!lua_istable(L, -1)) lua_pushnil(L); /* return fail */
	/* or return success */
	return 1;
}


/* ================== File system ======================== */

/* List a directory */
/* given parameters:
	- path, as string (optional)
returns:
	- table with directory contents
*/
int script_fs_dir (lua_State *L) {
	
	char path[PATH_MAX_CHARS+1];
	path[0] = '.'; path[1] = 0;
  char sub_path[PATH_MAX_CHARS+1];
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n > 0){
		if (!lua_isstring(L, 1)) { /* arguments types */
			lua_pushliteral(L, "dir: incorrect argument type");
			lua_error(L);
		}
		else if (strlen (lua_tostring(L, 1)) ){
			strncpy(path, lua_tostring(L, 1), PATH_MAX_CHARS);
		}
	}
	
	DIR *d, *subdir;
	d = opendir(path);
	if (d == NULL)
	{
		lua_pushnil(L); /* return fail */
		return 1;
	}

  /* make sure directory path terminates with separator character */
  if (path[strlen(path) - 1] != DIR_SEPARATOR){
    snprintf(path, PATH_MAX_CHARS, "%s%c", lua_tostring(L, 1), DIR_SEPARATOR);
  }
	
	int i, is_dir = 0;
	struct dirent *entry;
  struct stat filestat;
	i = 0;
	lua_newtable(L);
	while (entry = readdir(d)) {
		/* verify if current item is a subdir */
    snprintf(sub_path, PATH_MAX_CHARS, "%s%s", path, entry->d_name);
    
    //date = filestat.st_mtime; /* modification time */
    //size = filestat.st_size; /* file size */
    
		subdir = opendir(sub_path);
    
		if (subdir != NULL){ /* a subdir */
			if (!(strcmp(entry->d_name, ".") == 0) &&
			    !(strcmp(entry->d_name, "..") == 0)){ /* don't show current and parent dir information */
				i++;
        stat(sub_path, &filestat); /* get storage information */
        
				lua_newtable(L); /* table to store data */
				lua_pushstring(L, "name");
				lua_pushstring(L, entry->d_name);
				lua_rawset(L, -3);
				
				lua_pushstring(L, "is_dir");
				lua_pushboolean(L, 1);
				lua_rawset(L, -3);
        
        lua_pushstring(L, "created");
        lua_pushinteger(L, filestat.st_ctime);
        lua_rawset(L, -3);
        
        lua_pushstring(L, "modified");
        lua_pushinteger(L, filestat.st_mtime);
        lua_rawset(L, -3);
        
        lua_pushstring(L, "size");
        lua_pushinteger(L, filestat.st_size);
        lua_rawset(L, -3);
        
        lua_pushstring(L, "r");
        lua_pushboolean(L, filestat.st_mode & R_OK);
        lua_rawset(L, -3);
        
        lua_pushstring(L, "w");
        lua_pushboolean(L, filestat.st_mode & W_OK);
        lua_rawset(L, -3);
        
        lua_pushstring(L, "x");
        lua_pushboolean(L, filestat.st_mode & X_OK);
        lua_rawset(L, -3);
				
				lua_rawseti(L, -2, i); /*store in main table */
			}
		} else {
			i++;
      stat(sub_path, &filestat); /* get storage information */
      
			lua_newtable(L); /* table to store data */
			lua_pushstring(L, "name");
			lua_pushstring(L, entry->d_name);
			lua_rawset(L, -3);
			
			lua_pushstring(L, "is_dir");
			lua_pushboolean(L, 0);
			lua_rawset(L, -3);
      
      lua_pushstring(L, "created");
      lua_pushinteger(L, filestat.st_ctime);
			lua_rawset(L, -3);
			
      lua_pushstring(L, "modified");
      lua_pushinteger(L, filestat.st_mtime);
			lua_rawset(L, -3);
      
      lua_pushstring(L, "size");
      lua_pushinteger(L, filestat.st_size);
			lua_rawset(L, -3);
      
      lua_pushstring(L, "r");
			lua_pushboolean(L, filestat.st_mode & R_OK);
			lua_rawset(L, -3);
      
      lua_pushstring(L, "w");
			lua_pushboolean(L, filestat.st_mode & W_OK);
			lua_rawset(L, -3);
      
      lua_pushstring(L, "x");
			lua_pushboolean(L, filestat.st_mode & X_OK);
			lua_rawset(L, -3);
      
			lua_rawseti(L, -2, i); /*store in main table */
		}
	}
	closedir(d);
	return 1;
}

/* Change current directory */
/* given parameters:
	- path, as string
returns:
	- a boolean indicating success or fail
*/
int script_fs_chdir (lua_State *L) {
  
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
	
	char path[PATH_MAX_CHARS+1];
	path[0] = '.'; path[1] = 0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "chdir: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) { /* arguments types */
		lua_pushliteral(L, "chdir: incorrect argument type");
		lua_error(L);
	}
	else if (strlen (lua_tostring(L, 1)) ){
		strncpy(path, lua_tostring(L, 1), PATH_MAX_CHARS);
	}
	else {
		lua_pushboolean(L, 0); /* return fail */
		return 1;
	}
	
	int ret = chdir(path);
  
  if (!ret) strncpy (gui->dwg_dir, path, PATH_MAX_CHARS);
  char last_char = strlen(gui->dwg_dir) - 1;
  if(last_char > 1 && last_char < PATH_MAX_CHARS - 2) {
    if (gui->dwg_dir[last_char] != DIR_SEPARATOR) {
      gui->dwg_dir[last_char + 1] = DIR_SEPARATOR;
    }
  }
	
	lua_pushboolean(L, ret != 0); /* return success or fail */
	return 1;
}

/* Get current directory */
/* given parameters:
	- none
returns:
	- path, as string
*/
int script_fs_cwd (lua_State *L) {
	
	char curr_path[PATH_MAX_CHARS+1];
	getcwd(curr_path, PATH_MAX_CHARS);
	
	lua_pushstring(L, curr_path); /* return success or fail */
	return 1;
}

/* Get current script path */
/* given parameters:
	- none
returns:
	- path, as string
*/
int script_fs_script_path(lua_State *L) {
	
	/* get script object from Lua instance */
	lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	struct script_obj *script = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	char curr_path[PATH_MAX_CHARS+1];
	lua_Debug ar;
	lua_getstack (L, 1, &ar);
	lua_getinfo(L, "S", &ar); /* get script file name */
	if (strlen (ar.short_src) > 0)
		strncpy(curr_path, ar.short_src, PATH_MAX_CHARS);
	else
		strncpy(curr_path, script->path, PATH_MAX_CHARS);
	
	lua_pushstring(L, curr_path); /* return success or fail */
	return 1;
}

/*========== lazy sqlite =====================*/
/* Adapted from https://github.com/katlogic/lsqlite */

struct script_sqlite_db { /* sqlite db user object */
	sqlite3 *db;
	int changes;
};

/* -------------- Auxiliary functions --------------- */

/* Push one row column value at `idx` to the stack. */
static void push_field(lua_State *L, struct sqlite3_stmt *row, int idx){
	switch (sqlite3_column_type(row, idx)) {
		case SQLITE_INTEGER:
			lua_pushinteger(L, sqlite3_column_int64(row, idx));
			return;
		case SQLITE_FLOAT:
			lua_pushnumber(L, sqlite3_column_double(row, idx));
			return;
		case SQLITE_TEXT:
		case SQLITE_BLOB: {
			const char *p = sqlite3_column_blob(row, idx);
			if (!p) lua_pushnil(L); else
				lua_pushlstring(L, p, sqlite3_column_bytes(row, idx));
			return;
		}
		case SQLITE_NULL:
			lua_pushnil(L);
			return;
		default:
			lua_pushnil(L);
			return;
	}
}

/* Push all columns of a row on the stack. Returns number of columns. */
static int push_fields(lua_State *L, sqlite3_stmt *row){
	int i, n = sqlite3_data_count(row);
	for (i = 0; i < n; i++)
		push_field(L, row, i);
	return n;
}

/* Set columns as named keys/values in table `tab`. */
static void set_fields(lua_State *L, sqlite3_stmt *row){
	int i, n = sqlite3_data_count(row);
	for (i = 0; i < n; i++) {
		push_field(L, row, i);
		lua_setfield(L, -2, sqlite3_column_name(row, i));
	}
}

/* Perform a single step in db statement.
Return a table with pairs key/values (column name is key) */
static int col_iter(lua_State *L){
	/* get current statement (upvalue) */
	sqlite3_stmt *stmt = *(sqlite3_stmt **)lua_touserdata(L, lua_upvalueindex(1));
	
	int err = sqlite3_step(stmt);
	if (err == SQLITE_ROW) {
		lua_newtable(L);
		set_fields(L, stmt);
		return 1;
	}
	
	/* end - no more steps to perform */
	return 0;
	
}

/* Perform a single step in db statement.
Return columns values */
static int row_iter(lua_State *L){
	/* get current statement (upvalue) */
	sqlite3_stmt *stmt = *(sqlite3_stmt **)lua_touserdata(L, lua_upvalueindex(1));
	
	int err = sqlite3_step(stmt);
	if (err == SQLITE_ROW) {
		return push_fields(L, stmt);
	}
	
	/* end - no more steps to perform */
	return 0;
	
}

/*------------ binding functions to library ---------- */

/* full execution of a sqlite statement in database (usually SQL modify operations that do not return values)*/
/* given parameters:
	- a Sqlite_db object (this function is called as method inside object)
	- a SQL statement, as text string
returns:
	- current number of changes in db, as number
*/
int script_sqlite_exec(lua_State *L){
	
	struct script_sqlite_db * db;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "exec: invalid number of arguments");
		lua_error(L);
	}
	if (!( db =  luaL_checkudata(L, 1, "Sqlite_db") )) { /* the Sqlite_db object is a Lua userdata type*/
		lua_pushliteral(L, "exec: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, db->db != NULL, 1, "database is closed");
	
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	sqlite3_stmt *stmt;
	
	/* sqlite full statement cycle - prepare/step(s)/finalize */
	sqlite3_prepare_v2(db->db, lua_tostring(L, 2), -1, &stmt, NULL);
	
	int err;
	
	while (err = sqlite3_step(stmt) != SQLITE_DONE) {
		if (err ==  SQLITE_ERROR) break;
	}
	
	sqlite3_finalize(stmt);
	
	if (err == SQLITE_DONE || err == SQLITE_OK) {
		/* return number of current changes */
		int prev = db->changes;
		db->changes = sqlite3_total_changes(db->db);
		lua_pushinteger(L, db->changes - prev);
		return 1;
		
	} else {
		luaL_error(L, "Error in Sql: %s", sqlite3_errmsg(db->db));
	}
	
	lua_pushnil(L); /* return fail */
	return 1;
}

/* garbage colector function for statements in iterators */
int script_sqlite_stmt_gc(lua_State *L){
	sqlite3_stmt *stmt = *(sqlite3_stmt **)lua_touserdata(L, 1);
	sqlite3_finalize(stmt);
	return 0;
}

/* return total changes in database */
/* given parameters:
	- a Sqlite_db object (this function is called as method inside object)
returns:
	- total number of changes in db, as number
*/
int script_sqlite_changes(lua_State *L){
	struct script_sqlite_db * db;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "changes: invalid number of arguments");
		lua_error(L);
	}
	if (!( db =  luaL_checkudata(L, 1, "Sqlite_db") )) { /* the Sqlite_db object is a Lua userdata type*/
		lua_pushliteral(L, "changes: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, db->db != NULL, 1, "database is closed");
	
	db->changes = sqlite3_total_changes(db->db);
	lua_pushinteger(L, db->changes);
	return 1;
}

/* for tab in db:cols()    -    iterator producer */
/* given parameters:
	- a Sqlite_db object (this function is called as method inside object)
	- a SQL statement, as text string
returns:
	- table with key/values (column name is key), per row
*/
int script_sqlite_cols(lua_State *L){
	struct script_sqlite_db * db;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( db =  luaL_checkudata(L, 1, "Sqlite_db") )) { /* the Sqlite_db object is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, db->db != NULL, 1, "database is closed");
	
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	/* create a Sqlite_stmt object */
	sqlite3_stmt **stmt = (sqlite3_stmt **) lua_newuserdata(L, sizeof(sqlite3_stmt *));
	luaL_getmetatable(L, "Sqlite_stmt");
	lua_setmetatable(L, -2);
	
	/* init statement */
	*stmt = NULL;
	sqlite3_prepare_v2(db->db, lua_tostring(L, 2), -1, stmt, NULL);
	if (*stmt == NULL) luaL_error(L, "SQL error in statement");
	
	/* return iterator */
	lua_pushcclosure(L, col_iter, 1);
	return 1;
}

/* for col1,col2.. in db:rows()    -  iterator producer*/
/* given parameters:
	- a Sqlite_db object (this function is called as method inside object)
	- a SQL statement, as text string
returns:
	- independent columns values, per row
*/
int script_sqlite_rows(lua_State *L){
	struct script_sqlite_db * db;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( db =  luaL_checkudata(L, 1, "Sqlite_db") )) { /* the Sqlite_db object is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, db->db != NULL, 1, "database is closed");
	
	luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
	
	/* create a Sqlite_stmt object */
	sqlite3_stmt **stmt = (sqlite3_stmt **) lua_newuserdata(L, sizeof(sqlite3_stmt *));
	luaL_getmetatable(L, "Sqlite_stmt");
	lua_setmetatable(L, -2);
	
	/* init statement */
	*stmt = NULL;
	sqlite3_prepare_v2(db->db, lua_tostring(L, 2), -1, stmt, NULL);
	if (*stmt == NULL) luaL_error(L, "SQL error in statement");
	
	/* return iterator */
	lua_pushcclosure(L, row_iter, 1);
	return 1;
}

/* open a sqlite database */
/* given parameters:
	- sqlite database (file path), as string
returns:
	- a Sqlite_db object (use methods to manipulate), or nil if fail
*/
int script_sqlite_open(lua_State *L){
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "open: invalid number of arguments");
		lua_error(L);
	}
	luaL_argcheck(L, lua_isstring(L, 1), 1, "string expected");
	
	struct script_sqlite_db * db;
	
	/* create a userdata object */
	db = (struct script_sqlite_db *) lua_newuserdatauv(L, sizeof(struct script_sqlite_db), 0); 
	luaL_getmetatable(L, "Sqlite_db");
	lua_setmetatable(L, -2);
	
	/* init database */
	sqlite3_open(lua_tostring(L, 1), &(db->db));
	if (db->db == NULL){
		lua_pop(L, 1);
		lua_pushnil(L); /* return fail */
		return 1;
	}
	db->changes = 0;
	
	return 1;
}

/* close a previouly opened sqlite database */
/* given parameters:
	- a Sqlite_db object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int script_sqlite_close(lua_State *L){
	
	struct script_sqlite_db * db;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "close: invalid number of arguments");
		lua_error(L);
	}
	if (!( db =  luaL_checkudata(L, 1, "Sqlite_db") )) { /* the Sqlite_db object is a Lua userdata type*/
		lua_pushliteral(L, "close: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not closed */
	luaL_argcheck(L, db->db != NULL, 1, "database is closed");

	/* close database */
	sqlite3_close_v2(db->db);
	db->db = NULL;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* ------------ SVG Image ------------- */


/* Get raw data of SVG image */
/* given parameters:
	- SVG data, as string
returns:
	- a table with SVG data, or nil if fail
*/
int script_svg_curves(lua_State *L){
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "image: invalid number of arguments");
		lua_error(L);
	}
	luaL_argcheck(L, lua_isstring(L, 1), 1, "string expected");
	
  size_t len = 0;
  const char *data = lua_tolstring(L, 1, &len);
  char *svg_data = malloc(len + 1);
  
  /* init image */
  NSVGimage *curves =NULL;
  if (svg_data){  /* get vectorized data from SVG */
    strncpy(svg_data, data, len); /* copy string to allow modification */
    curves = nsvgParse(svg_data, "px", 96.0f);
    free(svg_data);
  }
  if (curves){ /* success on parse */
    if (curves->shapes){
    
      lua_newtable(L); /* main table to store data */
      
      /* default width and height from SVG */
      lua_pushstring(L, "width");
      lua_pushinteger(L, curves->width);
      lua_rawset(L, -3);
      
      lua_pushstring(L, "height");
      lua_pushinteger(L, curves->height);
      lua_rawset(L, -3);
      
      NSVGshape* shape;
      NSVGpath* path;
      
      int n_shapes = 1, n_paths;
      
      for (shape = curves->shapes; shape != NULL; shape = shape->next) {
        lua_newtable(L); /* table to store shape data */
        lua_pushstring(L, "strokeWidth");
        lua_pushnumber(L, shape->strokeWidth);
        lua_rawset(L, -3);
        
        n_paths = 1;
        for (path = shape->paths; path != NULL; path = path->next) {
          lua_newtable(L); /* table to store path data */
          lua_pushstring(L, "closed");
          lua_pushboolean(L, path->closed);
          lua_rawset(L, -3);
          /* control points */
          int i;
          for (i = 0; i < path->npts; i ++){
            lua_newtable(L); /* table to store control point */
            lua_pushstring(L, "x");
            lua_pushnumber(L, path->pts[i*2]);
            lua_rawset(L, -3);
            lua_pushstring(L, "y");
            lua_pushnumber(L, curves->height - path->pts[i*2+1]);
            lua_rawset(L, -3);
            lua_rawseti(L, -2, i+1); /*store point in path table */
          }
          lua_rawseti(L, -2, n_paths); /*store in shape table */
          n_paths++;
        }
        lua_rawseti(L, -2, n_shapes); /*store in main table */
        n_shapes++;
      }
      
      
      nsvgDelete(curves);
    }
  }
	else{
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	return 1;
}

/* Rasterize a SVG image */
/* given parameters:
	- SVG data, as string
  - forced width and height, as numbers (optional, default: use SVG param)
returns:
	- a Image object (user value), or nil if fail
*/
int script_svg_image(lua_State *L){
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "image: invalid number of arguments");
		lua_error(L);
	}
	luaL_argcheck(L, lua_isstring(L, 1), 1, "string expected");
	
	struct script_rast_image * img_obj;
  
	/* create a userdata object */
	img_obj = (struct script_rast_image *) lua_newuserdatauv(L, sizeof(struct script_rast_image), 0); 
	luaL_getmetatable(L, "Rast_img");
	lua_setmetatable(L, -2);
	
  img_obj->img = NULL;
  size_t len = 0;
  const char *data = lua_tolstring(L, 1, &len);
  char *svg_data = malloc(len + 1);
  
  /* init image */
  NSVGimage *curves =NULL;
  if (svg_data){  /* get vectorized data from SVG */
    strncpy(svg_data, data, len); /* copy string to allow modification */
    curves = nsvgParse(svg_data, "px", 96.0f);
    free(svg_data);
  }
  if (curves){ /* rasterize image */
    if (curves->shapes){
      int w = curves->width; /* default width and height from SVG */
      int h = curves->height;
    
      if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) { /* force size - optional*/
        w = lua_tonumber(L, 2);
        h = lua_tonumber(L, 3);
      }
      
      img_obj->img = i_svg_bmp(curves, w, h);
      nsvgDelete(curves);
    }
  }
  
	if (img_obj->img == NULL){
		lua_pop(L, 1);
		lua_pushnil(L); /* return fail */
		return 1;
	}
	
	return 1;
}

/* garbage colector function for raster images */
int script_rast_image_gc(lua_State *L){
	struct script_rast_image * img_obj = (struct script_rast_image *)lua_touserdata(L, 1);
	bmp_free(img_obj->img);
	return 0;
}

/* ------------- Misc ------*/
/* Get a unique ID */
/* given parameters:
	- none
returns:
	- ID, as string
*/
int script_unique_id (lua_State *L) {
	
	int n = rand(); /* random number */
	static int c = 0; /* internal counter */
	time_t t;
	time(&t); /* get current time */
	
	/* compose a ID with current time, a internal counter and a random number */
	long long id;
	id = (long long) (t << 32) | (c << 16) | n;
	
	c++; /* increment counter for next call */
	
	char out[21];
	snprintf(out, 20, "%016llX", id); /* string showing a 64-bit hexadecimal number */
	
	lua_pushstring(L, out); /* return success or fail */
	return 1;
}

/* try to locate the last numbered block, match name starting with mark chars */
/* given parameters:
	- mark, as string
returns:
	- last available number
*/
int script_last_blk (lua_State *L) {
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
		lua_pushliteral(L, "last_blk: invalid number of arguments");
		lua_error(L);
	}
	
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "last_blk: incorrect argument type");
		lua_error(L);
	}
	
	int last = dxf_find_last_blk (gui->drawing, (char *) lua_tostring(L, 1));
	
	lua_pushinteger(L, last);
	return 1;
}
