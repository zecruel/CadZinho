#include "dxf_3d.h"

extern struct script_obj dxf_3d_engine;

struct manifold_obj {
	ManifoldManifold *obj;
};

void *manifold_buffer() { return malloc(manifold_manifold_size()); }
void *meshgl64_buffer() { return malloc(manifold_meshgl64_size()); }


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
int dxf_new_sphere (lua_State *L) {
	
	/* verify passed arguments */
	int n = lua_gettop(L);    /* number of arguments */
	if (n = 0){
		lua_pushliteral(L, "sphere: invalid number of arguments");
		lua_error(L);
	}
	
	int i;
	/* arguments types */
	/*if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "sphere: incorrect argument type");
		lua_error(L);
	}*/
	
	
	/* create a userdata object */
	struct manifold_obj *sphere;
	
	sphere = (struct manifold_obj *) lua_newuserdatauv(L, sizeof(struct manifold_obj *), 0); 
	luaL_getmetatable(L, "Manifold");
	lua_setmetatable(L, -2);
	
	sphere->obj = manifold_sphere(manifold_buffer(), 1.0, 4 * 25);
	
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
	
	lua_pushcfunction(T, dxf_new_sphere);
	lua_setglobal(T, "sphere"); 
	
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