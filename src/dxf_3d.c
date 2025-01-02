#include "dxf_3d.h"

extern struct script_obj dxf_3d_engine;

struct manifold_obj {
	ManifoldManifold *obj;
};

void *manifold_buffer() { return malloc(manifold_manifold_size()); }
void *meshgl64_buffer() { return malloc(manifold_meshgl64_size()); }


/* create a sphere manifold */
/* given parameters:
	- radius, as number (optional, dflt = 1)
  - center x,y,z coordinates, as numbers (dflt = 0,0,0)
	- circ segments, as number factor of 4 (dflt = 20)
	- Manifold object, as userdata
*/
int dxf_3d_sphere (lua_State *L) {
  double r = 1.0, x = 0.0, y = 0.0, z = 0.0;
  int c_seg = 20;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) r = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) x = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) y = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) z = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) c_seg = lua_tointeger(L, 5);
  
  if (r <= 0.0) r = 1.0;
  if (c_seg <= 0) c_seg = 20;
	
	/* create a userdata object */
	struct manifold_obj *sphere;
	
	sphere = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  ManifoldManifold *sph = manifold_sphere(manifold_buffer(), r, c_seg);
  ManifoldManifold *trans = manifold_translate(manifold_buffer(), sph, x, y, z);
  manifold_destruct_manifold(sph);
	free(sph);
  
	sphere->obj = trans;
	
	return 1;
}

/* create a slab manifold */
/* given parameters:
	- edge lengths w,h,p, as numbers (dflt = 1,1,1)
  - origin x,y,z coordinates, as numbers (dflt = 0,0,0)
  - rotation angles x,y,z degrees, as numbers (dflt = 0,0,0)
	- Manifold object, as userdata
*/
int dxf_3d_slab (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0, x = 0.0, y = 0.0, z = 0.0;
  double ax = 0.0, ay = 0.0, az = 0.0;
  int c_seg = 20;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) x = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) y = lua_tonumber(L, 5);
  if (lua_isnumber(L, 6)) z = lua_tonumber(L, 6);
  if (lua_isnumber(L, 7)) ax = lua_tonumber(L, 7);
  if (lua_isnumber(L, 8)) ay = lua_tonumber(L, 8);
  if (lua_isnumber(L, 9)) az = lua_tonumber(L, 9);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
	
	/* create a userdata object */
	struct manifold_obj *slab;
	
	slab = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  ManifoldManifold *cube = manifold_cube(manifold_buffer(), w, h, p, 0);
  ManifoldManifold *rot = manifold_rotate(manifold_buffer(), cube, ax, ay, az);
  ManifoldManifold *trans = manifold_translate(manifold_buffer(), rot, x, y, z);
  manifold_destruct_manifold(cube);
	free(cube);
  manifold_destruct_manifold(rot);
	free(rot);
  
	slab->obj = trans;
	
	return 1;
}

/* create a cylinder manifold */
/* given parameters:
  - heigth, as number (optional, dflt = 1)
  - bottom radius, as number (optional, dflt = 1)
	- top radius, as number (optional, dflt = bottom radius)
  - origin x,y,z coordinates, as numbers (dflt = 0,0,0)
  - rotation angles x,y,z degrees, as numbers (dflt = 0,0,0)
  - circ segments, as number factor of 4 (dflt = 20)
	- Manifold object, as userdata
*/
int dxf_3d_cylinder (lua_State *L) {
  double h = 1.0, r1 = 1.0, r2 = 1.0, x = 0.0, y = 0.0, z = 0.0;
  double ax = 0.0, ay = 0.0, az = 0.0;
  int c_seg = 20;
	
	/* verify passed arguments */
	
  if (lua_isnumber(L, 1)) h = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) r1 = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) r2 = lua_tonumber(L, 3);
  else r2 = r1;
  if (lua_isnumber(L, 4)) x = lua_tonumber(L, 4);
  if (lua_isnumber(L, 5)) y = lua_tonumber(L, 5);
  if (lua_isnumber(L, 6)) z = lua_tonumber(L, 6);
  if (lua_isnumber(L, 7)) ax = lua_tonumber(L, 7);
  if (lua_isnumber(L, 8)) ay = lua_tonumber(L, 8);
  if (lua_isnumber(L, 9)) az = lua_tonumber(L, 9);
  if (lua_isnumber(L, 10)) c_seg = lua_tointeger(L, 10);
  
  if (r1 <= 0.0) r1 = 1.0;
  if (h <= 0.0) h = 1.0;
  if (r2 <= 0.0) r2 = 1.0;
  if (c_seg <= 0) c_seg = 20;
	
	/* create a userdata object */
	struct manifold_obj *cylinder;
	
	cylinder = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  ManifoldManifold *cil = manifold_cylinder(manifold_buffer(), h, r1, r2, c_seg, 0);
  ManifoldManifold *rot = manifold_rotate(manifold_buffer(), cil, ax, ay, az);
  ManifoldManifold *trans = manifold_translate(manifold_buffer(), rot, x, y, z);
  manifold_destruct_manifold(cil);
	free(cil);
  manifold_destruct_manifold(rot);
	free(rot);
  
	cylinder->obj = trans;
	
	return 1;
}

/* Manifold union operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a+b", as userdata
*/
int dxf_3d_union (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *uni;
	
	uni = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  uni->obj = manifold_union(manifold_buffer(), a->obj, b->obj);
  
	
	return 1;
}

/* Manifold difference operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a-b", as userdata
*/
int dxf_3d_difference (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *uni;
	
	uni = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  uni->obj = manifold_difference(manifold_buffer(), a->obj, b->obj);
  
	
	return 1;
}

/* Manifold difference operation */
/* given parameters:
	- Manifold object "a", as userdata
  - Manifold object "b", as userdata
returns:
	- Manifold object "a-b", as userdata
*/
int dxf_3d_intersection (lua_State *L) {
	
	struct manifold_obj *a, *b;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 2){
		lua_pushnil(L); /* return error */
    return 1;
	}
	if (!( a = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  if (!( b = udata_check(L, 2, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (a->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
  
  if (b->obj == NULL){
		lua_pushnil(L); /* return error */
    return 1;
	}
	
	/* create a userdata object */
	struct manifold_obj *uni;
	
	uni = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
  uni->obj = manifold_intersection(manifold_buffer(), a->obj, b->obj);
  
	
	return 1;
}

/* destroy a previouly created manifold object */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
returns:
	- a boolean indicating success or fail
*/
int manifold_destroy (lua_State *L) {
	
	struct manifold_obj * manifold;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushliteral(L, "destroy: invalid number of arguments");
		lua_error(L);
	}
	if (!( manifold =  udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushliteral(L, "destroy: incorrect argument type");
		lua_error(L);
	}
	
	/* check if it is not destroyed */
	luaL_argcheck(L, manifold->obj != NULL, 1, "invalid manifold");
	
	/* destroy the archive, freeing any resources it was using */
	manifold_destruct_manifold(manifold->obj);
	free(manifold->obj);
	manifold->obj = NULL;
	
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* create a mesh entity */
/* given parameters:
	- manifold commands, as string
	- drawing parameters (layer, color, etc), as table (optional)
returns:
	- DXF entity, as userdata
Notes:
	- The returned data is for one shot use in Lua script, because
	the alocated memory is valid in single iteration of main loop.
	It is assumed that soon afterwards it will be appended or drawn.
*/
dxf_node * dxf_new_mesh  (dxf_drawing *drawing, char *chunk, int color, char *layer, int pool){
	lua_State *L = dxf_3d_engine.T;
	dxf_3d_engine.status = luaL_dostring(L, chunk);
	if (dxf_3d_engine.status != LUA_OK){
		//dxf_3d_engine.active = 0; /* error */
		return NULL;
	}
	
	struct manifold_obj * manifold;
	lua_getglobal(L, "manifold");
	/* verify passed arguments */
	//int n = lua_gettop(L);    /* number of arguments */
	//if (n < 1){
		//return NULL;
	//}
	if (!( manifold =  udata_check(L, -1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		return NULL;
	}
	
	
	ManifoldManifold *sphere = manifold->obj;
	ManifoldMeshGL64 *mesh = manifold_get_meshgl64(meshgl64_buffer(), sphere);
	
	
	
	/* new mesh entity */
	dxf_node * new_el = (dxf_node *) dxf_new_face_mesh (drawing, mesh, chunk, 
		color, layer, pool); 

	
	
	manifold_destruct_meshgl64(mesh);
	free(mesh);
	
	if (!new_el) {
		return NULL;
	}
	/* return success */
	return new_el;
}

int dxf_3d_init (){
	const char * chunk = "manifold = {}";
	/* close previous Lua state */
	if(dxf_3d_engine.L) lua_close(dxf_3d_engine.L);
  
	/* initialize script object */
	if(!(dxf_3d_engine.L = luaL_newstate())) return 0; /* opens Lua */
	dxf_3d_engine.T = NULL;
	dxf_3d_engine.status = LUA_OK;
	dxf_3d_engine.active = 0;
	dxf_3d_engine.dynamic = 0;
	dxf_3d_engine.do_init = 0;
	//dxf_3d_engine.wait_gui_resume = 0;
	dxf_3d_engine.groups = 0;
	dxf_3d_engine.path[0] = 0;
	
	dxf_3d_engine.timeout = 10.0; /* default timeout value */
	
	luaL_openlibs(dxf_3d_engine.L); /* opens the standard libraries */
	
	/* create a new lua thread, allowing yield */
	lua_State *T = lua_newthread(dxf_3d_engine.L);
	if(!T) {
		lua_close(dxf_3d_engine.L);
		return 0;
	}
	dxf_3d_engine.T = T;
	
	/* put the gui structure in lua global registry */
	//lua_pushstring(T, "cz_gui");
	//lua_pushlightuserdata(T, (void *)gui);
	//lua_settable(T, LUA_REGISTRYINDEX);
	
	
	
	
	
	
	static const struct luaL_Reg manifold_meths[] = {
		{"__gc", manifold_destroy},
		{NULL, NULL}
	};
	
	/* create a new type of lua userdata to represent a ZIP archive */
	/* create metatable */
	luaL_newmetatable(T, "Manifold");
	/* metatable.__index = metatable */
	lua_pushvalue(T, -1);
	lua_setfield(T, -2, "__index");
	/* register methods */
	luaL_setfuncs(T, manifold_meths, 0);
	lua_pop( T, 1);
	
	lua_pushcfunction(T, dxf_3d_sphere);
	lua_setglobal(T, "sphere");
  lua_pushcfunction(T, dxf_3d_slab);
	lua_setglobal(T, "slab");
  lua_pushcfunction(T, dxf_3d_cylinder);
	lua_setglobal(T, "cylinder");
  
  lua_pushcfunction(T, dxf_3d_union);
	lua_setglobal(T, "union");
  lua_pushcfunction(T, dxf_3d_difference);
	lua_setglobal(T, "difference");
  lua_pushcfunction(T, dxf_3d_intersection);
	lua_setglobal(T, "intersection");
	
	lua_sethook(T, script_check, LUA_MASKCOUNT, 10000);
	
	//dxf_3d_engine.status = luaL_loadstring(T, chunk);
	
	
	//if ( dxf_3d_engine.status == LUA_OK)  {
	//	lua_setglobal(T, "cz_main_func"); /* store main function in global variable */
	//	return 1;
	//}
	
	//return -1;
	
	#if(0)
	/* try to run init file */
	if (gui_script_init (gui, &dxf_3d_engine, init_path, NULL) == 1){
		dxf_3d_engine.time = clock();
		dxf_3d_engine.timeout = 1.0; /* default timeout value */
		dxf_3d_engine.do_init = 0;
		
		lua_getglobal(dxf_3d_engine.T, "cz_main_func");
		int n_results = 0; /* for Lua 5.4*/
		dxf_3d_engine.status = lua_resume(dxf_3d_engine.T, NULL, 0, &n_results); /* start thread */
		if (dxf_3d_engine.status != LUA_OK){
			dxf_3d_engine.active = 0; /* error */			
		}
		/* finaly get states from global variables in Lua instance */
		gui_get_ini (dxf_3d_engine.T);
		
		/* close script and clean instance*/
		lua_close(dxf_3d_engine.L);
		dxf_3d_engine.L = NULL;
		dxf_3d_engine.T = NULL;
		dxf_3d_engine.active = 0;
		dxf_3d_engine.dynamic = 0;
	}
	#endif
}