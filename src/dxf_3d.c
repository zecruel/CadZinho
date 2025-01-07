#include "dxf_3d.h"

extern struct script_obj dxf_3d_engine;

struct manifold_obj {
	ManifoldManifold *obj;
  ManifoldManifold *prev;
};

void *manifold_buffer() { return malloc(manifold_manifold_size()); }
void *meshgl64_buffer() { return malloc(manifold_meshgl64_size()); }
void *simple_polyg_buffer() { return malloc(manifold_simple_polygon_size()); }
void *polygons_buffer() { return malloc(manifold_polygons_size()); }


/* create a sphere manifold */
/* given parameters:
	- radius, as number (optional, dflt = 1)
  - center x,y,z coordinates, as numbers (dflt = 0,0,0)
	- Manifold object, as userdata
*/
int dxf_3d_sphere (lua_State *L) {
  double r = 1.0;
  int c_seg = 32;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) r = lua_tonumber(L, 1);
  
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
  if (r <= 0.0) r = 1.0;
  if (c_seg <= 0) c_seg = 32;
	
	/* create a userdata object */
	struct manifold_obj *sphere;
	
	sphere = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  sphere->obj = NULL;
  sphere->prev = NULL;
	
  sphere->obj = manifold_sphere(manifold_buffer(), r, c_seg);
  sphere->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* create a slab manifold */
/* given parameters:
	- edge lengths w,h,p, as numbers (dflt = 1,1,1)
	- Manifold object, as userdata
*/
int dxf_3d_slab (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0;
  
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
	
	/* create a userdata object */
	struct manifold_obj *slab;
	
	slab = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  slab->obj = NULL;
  slab->prev = NULL;
	
  slab->obj = manifold_cube(manifold_buffer(), w, h, p, 0);
  slab->prev = manifold_empty(manifold_buffer());
  
	
	return 1;
}

/* create a cylinder manifold */
/* given parameters:
  - heigth, as number (optional, dflt = 1)
  - bottom radius, as number (optional, dflt = 1)
	- top radius, as number (optional, dflt = bottom radius)
	- Manifold object, as userdata
*/
int dxf_3d_cylinder (lua_State *L) {
  double h = 1.0, r1 = 1.0, r2 = 1.0;
  int c_seg = 32;
	
	/* verify passed arguments */
	
  if (lua_isnumber(L, 1)) h = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) r1 = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) r2 = lua_tonumber(L, 3);
  else r2 = r1;
  lua_getglobal(L, "circular_segments");
  if (lua_isnumber(L, -1)) c_seg = lua_tointeger(L, -1);
  lua_pop (L, 1);
  
  if (r1 <= 0.0) r1 = 1.0;
  if (h <= 0.0) h = 1.0;
  if (r2 <= 0.0) r2 = 1.0;
  if (c_seg <= 0) c_seg = 32;
	
	/* create a userdata object */
	struct manifold_obj *cylinder;
	
	cylinder = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  cylinder->obj = NULL;
  cylinder->prev = NULL;
	
  cylinder->obj = manifold_cylinder(manifold_buffer(), h, r1, r2, c_seg, 0);
  cylinder->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* create a pyramid manifold */
/* given parameters:
	- base edge lengths w,h, as numbers (dflt = 1,1)
  - pyramid total heigth p, as number (dflt = 1)
  - top scale factor relative to base, as number (dflt = 0 - complete pyramid)
	- Manifold object, as userdata
*/
int dxf_3d_pyramid (lua_State *L) {
  double w = 1.0, h = 1.0, p = 1.0, s = 0.0;
	
	/* verify passed arguments */
	
	if (lua_isnumber(L, 1)) w = lua_tonumber(L, 1);
  if (lua_isnumber(L, 2)) h = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) p = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) s = lua_tonumber(L, 4);
  
  if (w <= 0.0) w = 1.0;
  if (h <= 0.0) h = 1.0;
  if (p <= 0.0) p = 1.0;
  if (s < 0.0) p = 0.0;
	
	/* create a userdata object */
	struct manifold_obj *pyr;
	
	pyr = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  pyr->obj = NULL;
  pyr->prev = NULL;
	
  
  //ManifoldVec2 pts[] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
  ManifoldVec2 pts[] = {{-w/2.0, -h/2.0}, {w/2.0, -h/2.0}, {w/2.0, h/2.0}, {-w/2.0, h/2.0}};
  ManifoldSimplePolygon *sq[] = {
      manifold_simple_polygon(simple_polyg_buffer(), &pts[0], 4)};
  ManifoldPolygons *polys = manifold_polygons(polygons_buffer(), sq, 1);

  pyr->obj = manifold_extrude(manifold_buffer(), polys, p, 0, 0, s, s);
  pyr->prev = manifold_empty(manifold_buffer());
  
  manifold_destruct_simple_polygon(sq[0]);
  manifold_destruct_polygons(polys);
  free(sq[0]);
  free(polys);
	
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
	
	uni = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  uni->obj = NULL;
  uni->prev = NULL;
	
  uni->obj = manifold_union(manifold_buffer(), a->obj, b->obj);
  uni->prev = manifold_empty(manifold_buffer());
  
	
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
	struct manifold_obj *dif;
	
	dif = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  dif->obj = NULL;
  dif->prev = NULL;
	
  dif->obj = manifold_difference(manifold_buffer(), a->obj, b->obj);
  dif->prev = manifold_empty(manifold_buffer());
  
	
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
	struct manifold_obj *inter;
	
	inter = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
  
  inter->obj = NULL;
  inter->prev = NULL;
	
  inter->obj = manifold_intersection(manifold_buffer(), a->obj, b->obj);
  inter->prev = manifold_empty(manifold_buffer());
	
	return 1;
}

/* modify a manifold object - translate */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z values to move, as numbers (dflt = 0,0,0)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_translate (lua_State *L) {
	
	struct manifold_obj * manifold;
  double x = 0.0, y = 0.0, z = 0.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) x = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) y = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) z = lua_tonumber(L, 4);
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_translate(manifold->prev, manifold->obj, x, y, z);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - rotate */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z angle values in degrees, as numbers (dflt = 0,0,0)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_rotate (lua_State *L) {
	
	struct manifold_obj * manifold;
  double ax = 0.0, ay = 0.0, az = 0.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) ax = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) ay = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) az = lua_tonumber(L, 4);
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_rotate(manifold->prev, manifold->obj, ax, ay, az);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
	return 1;
}

/* modify a manifold object - scale */
/* given parameters:
	- a Manifold object (this function is called as method inside object)
  - x,y,z values factors, as numbers (dflt = 0,0,0)
returns:
	- a boolean indicating success or fail
*/
int dxf_3d_scale (lua_State *L) {
	
	struct manifold_obj * manifold;
  double sx = 0.0, sy = 0.0, sz = 0.0;
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n < 1){
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
  
  if (lua_isnumber(L, 2)) sx = lua_tonumber(L, 2);
  if (lua_isnumber(L, 3)) sy = lua_tonumber(L, 3);
  if (lua_isnumber(L, 4)) sz = lua_tonumber(L, 4);
	
	/* check if it is not destroyed */
	if (manifold->obj == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
  /* check if it is not destroyed */
	if (manifold->prev == NULL){
    lua_pushboolean(L, 0); /* return fail */
    return 1;
  }
	
  ManifoldManifold *tmp = manifold->obj;
  manifold->prev = manifold_scale(manifold->prev, manifold->obj, sx, sy, sz);
  manifold->obj = manifold->prev;
  manifold->prev = tmp;
  
	lua_pushboolean(L, 1); /* return success */
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
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	if (!( manifold = udata_check(L, 1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
		lua_pushboolean(L, 0); /* return fail */
    return 1;
	}
	
	/* check if it is not destroyed */
	if (manifold->obj != NULL){
    /* destroy the object, freeing any resources it was using */
    manifold_destruct_manifold(manifold->obj);
    free(manifold->obj);
    manifold->obj = NULL;
  }
  /* check if it is not destroyed */
	if (manifold->prev != NULL){
    /* destroy the prevect, freeing any resources it was using */
    manifold_destruct_manifold(manifold->prev);
    free(manifold->prev);
    manifold->prev = NULL;
  }
	
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
  dxf_3d_engine.time = clock();
  
  lua_newtable(L);
	lua_setglobal(L, "manifold");
  
  lua_pushinteger(L, 32);
	lua_setglobal(L, "circular_segments");
  
  
	dxf_3d_engine.status = luaL_loadbuffer(L, chunk, strlen(chunk), "engine3d");
	if (dxf_3d_engine.status != LUA_OK){
		//dxf_3d_engine.active = 0; /* error */
    printf("3D engine error  - chunk\n" );
		return NULL;
	}
  int n_res = 0; /* for Lua 5.4*/
  dxf_3d_engine.status = lua_resume(L, NULL, 0, &n_res); /* start thread */
	if (dxf_3d_engine.status != LUA_OK){
		//dxf_3d_engine.active = 0; /* error */
    int n = lua_gettop(L);    /* number of arguments */
    printf("3D engine error  - resume, %d , %d\n", n_res, n );
		return NULL;
	}
	
	struct manifold_obj * manifold;
	lua_getglobal(L, "manifold");
	/* verify passed arguments */
	if(!lua_istable(L, -1)){
    printf("3D engine error  - global\n" );
    return NULL;
  }
  
  int last = lua_rawlen(L, -1);
  
  lua_rawgeti (L, -1, last);
  
	if (!( manifold = udata_check(L, -1, "Manifold") )) { /* the Manifold object is a Lua userdata type*/
    printf("3D engine error  - metatable\n" );
		return NULL;
	}
	lua_pop(L, 2);
	
	ManifoldMeshGL64 *mesh = manifold_get_meshgl64(meshgl64_buffer(), manifold->obj);
	
	
	
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
	
	/* put the engine script structure in lua global registry */
	lua_pushstring(T, "cz_script");
	lua_pushlightuserdata(T, (void *) &dxf_3d_engine);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	static const struct luaL_Reg manifold_meths[] = {
    {"translate", dxf_3d_translate},
    {"rotate", dxf_3d_rotate},
    {"scale", dxf_3d_scale},
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
  
  lua_pushinteger(T, 32);
	lua_setglobal(T, "circular_segments");
	
	lua_pushcfunction(T, dxf_3d_sphere);
	lua_setglobal(T, "sphere");
  lua_pushcfunction(T, dxf_3d_slab);
	lua_setglobal(T, "slab");
  lua_pushcfunction(T, dxf_3d_cylinder);
	lua_setglobal(T, "cylinder");
  lua_pushcfunction(T, dxf_3d_pyramid);
	lua_setglobal(T, "pyramid");
  
  lua_pushcfunction(T, dxf_3d_union);
	lua_setglobal(T, "union");
  lua_pushcfunction(T, dxf_3d_difference);
	lua_setglobal(T, "difference");
  lua_pushcfunction(T, dxf_3d_intersection);
	lua_setglobal(T, "intersection");
	
	lua_sethook(T, script_check, LUA_MASKCOUNT, 10000);
	
	}