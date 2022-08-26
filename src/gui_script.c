#include "gui_script.h"

static int print_lua_var(char * value, lua_State * L){
	int type = lua_type(L, -1);

	switch(type) {
		case LUA_TSTRING: {
			snprintf(value, DXF_MAX_CHARS - 1, "s: %s", lua_tostring(L, -1));
			break;
		}
		case LUA_TNUMBER: {
		/* LUA_NUMBER may be double or integer */
			snprintf(value, DXF_MAX_CHARS - 1, "n: %.9g", lua_tonumber(L, -1));
			break;
		}
		case LUA_TTABLE: {
			snprintf(value, DXF_MAX_CHARS - 1, "t: 0x%08x", lua_topointer(L, -1));
			break;
		}
		case LUA_TFUNCTION: {
			snprintf(value, DXF_MAX_CHARS - 1, "f: 0x%08x", lua_topointer(L, -1));
			break;		}
		case LUA_TUSERDATA: {
			snprintf(value, DXF_MAX_CHARS - 1, "u: 0x%08x", lua_touserdata(L, -1));
			break;
		}
		case LUA_TLIGHTUSERDATA: {
			snprintf(value, DXF_MAX_CHARS - 1, "U: 0x%08x", lua_touserdata(L, -1));
			break;
		}
		case LUA_TBOOLEAN: {
			snprintf(value, DXF_MAX_CHARS - 1, "b: %d", lua_toboolean(L, -1) ? 1 : 0);
			break;
		}
		case LUA_TTHREAD: {
			snprintf(value, DXF_MAX_CHARS - 1, "d: 0x%08x", lua_topointer(L, -1));
			break;
		}
		case LUA_TNIL: {
			snprintf(value, DXF_MAX_CHARS - 1, "nil");
			break;
		}
	}
}

/* Routine to check break points and script execution time ( timeout in stuck scripts)*/
void script_check(lua_State *L, lua_Debug *ar){
	
	/* listen to "Hook Lines" events to verify debug breakpoints */
	if(ar->event == LUA_HOOKLINE){
		/* get gui object from Lua instance */
		lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
		lua_gettable(L, LUA_REGISTRYINDEX); 
		gui_obj *gui = lua_touserdata (L, -1);
		lua_pop(L, 1);
		
		if (!gui){ /* error in gui object access */
			//lua_pushstring(L, "Auto check: no access to CadZinho enviroment");
			//lua_error(L);
			return;
		}
		
		int i;
		/* sweep the breakpoints list */
		for (i = 0; i < gui->num_brk_pts; i++){
			/* verify if break conditions matchs with current line */
			if ((ar->currentline == gui->brk_pts[i].line) && gui->brk_pts[i].enable){	
				/* get the source name */
				char msg[DXF_MAX_CHARS];
				char source[DXF_MAX_CHARS];
				lua_getinfo(L, "Sl", ar); /* fill debug informations */
				strncpy(source, get_filename(ar->short_src), DXF_MAX_CHARS - 1);
				
				if (strcmp(source, gui->brk_pts[i].source) == 0){
					/* pause execution*/
					snprintf(msg, DXF_MAX_CHARS-1, "db: Thread paused at: %s-line %d\n", source, ar->currentline);
					nk_str_append_str_char(&gui->debug_edit.string, msg);
					lua_yield (L, 0);
					return;
				}
			}
		}
	}
	
	/* listen to "Hook Count" events to verify execution time and timeout */
	else if(ar->event == LUA_HOOKCOUNT){
		/* get script object from Lua instance */
		lua_pushstring(L, "cz_script"); /* is indexed as  "cz_script" */
		lua_gettable(L, LUA_REGISTRYINDEX); 
		struct script_obj *script = lua_touserdata (L, -1);
		lua_pop(L, 1);
		
		if (!script){ /* error in gui object access */
			lua_pushstring(L, "Auto check: no access to CadZinho script object");
			lua_error(L);
			return;
		}
		
		clock_t end_t;
		double diff_t;
		/* get the elapsed time since script starts or continue */
		end_t = clock();
		diff_t = (double)(end_t - script->time) / CLOCKS_PER_SEC;
		
		/* verify if timeout is reachead. Its made to prevent user script stuck main program*/
		if (diff_t >= script->timeout){
			char msg[DXF_MAX_CHARS];
			lua_getinfo(L, "Sl", ar); /* fill debug informations */
			
			/* stop script execution */
			snprintf(msg, DXF_MAX_CHARS-1, "script timeout exceeded in %s, line %d, exec time %f s\n", ar->source, ar->currentline, diff_t);
			//nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			script->active = 0;
			script->dynamic = 0;
			
			lua_pushstring(L, msg);
			lua_error(L);
			return;
		}
	}
}

/* init script from file or alternative string chunk */
int gui_script_init (gui_obj *gui, struct script_obj *script, char *fname, char *alt_chunk) {
	if(!gui) return 0;
	if(!script) return 0;
	if (!fname && !alt_chunk) return 0;
	
	/* initialize script object */ 
	if(!(script->L = luaL_newstate())) return 0; /* opens Lua */
	script->T = NULL;
	script->status = LUA_OK;
	script->active = 0;
	script->dynamic = 0;
	script->do_init = 0;
	//script->wait_gui_resume = 0;
	script->groups = 0;
	strncpy(script->path, fname, DXF_MAX_CHARS - 1);
	
	script->timeout = 10.0; /* default timeout value */
	
	luaL_openlibs(script->L); /* opens the standard libraries */
	
	/* create a new lua thread, allowing yield */
	lua_State *T = lua_newthread(script->L);
	if(!T) {
		lua_close(script->L);
		return 0;
	}
	script->T = T;
	
	/* put the gui structure in lua global registry */
	lua_pushstring(T, "cz_gui");
	lua_pushlightuserdata(T, (void *)gui);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	/* put the current script structure in lua global registry */
	lua_pushstring(T, "cz_script");
	lua_pushlightuserdata(T, (void *)script);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	/* add functions in cadzinho object*/
	static const luaL_Reg cz_lib[] = {
		{"exec_file", gui_script_exec_file},
		{"db_print",   debug_print},
		{"set_timeout", set_timeout},
		{"get_sel", script_get_sel},
		{"clear_sel", script_clear_sel},
		{"enable_sel", script_enable_sel},
		{"get_ent_typ", script_get_ent_typ},
		{"get_circle_data", script_get_circle_data},
		{"get_blk_name", script_get_blk_name},
		{"get_ins_data", script_get_ins_data},
		{"count_attrib", script_count_attrib},
		{"get_attrib_i", script_get_attrib_i},
		{"get_attribs", script_get_attribs},
		{"get_points", script_get_points},
		{"get_bound", script_get_bound},
		{"get_ext", script_get_ext},
		{"get_blk_ents", script_get_blk_ents},
		{"get_all", script_get_all},
		{"get_text_data", script_get_text_data},
		{"get_drwg_path", script_get_drwg_path},
		
		{"edit_attr", script_edit_attr},
		{"add_ext", script_add_ext},
		{"edit_ext_i", script_edit_ext_i},
		{"del_ext_i", script_del_ext_i},
		{"del_ext_all", script_del_ext_all},
		
		{"new_line", script_new_line},
		{"new_pline", script_new_pline},
		{"pline_append", script_pline_append},
		{"pline_close", script_pline_close},
		{"new_circle", script_new_circle},
		{"new_hatch", script_new_hatch},
		{"new_text", script_new_text},
		{"new_block", script_new_block},
		{"new_block_file", script_new_block_file},
		{"new_insert", script_new_insert},
		
		{"get_dwg_appids", script_get_dwg_appids},
		
		{"set_layer", script_set_layer},
		{"set_color", script_set_color},
		{"set_ltype", script_set_ltype},
		{"set_style", script_set_style},
		{"set_lw", script_set_lw},
		{"set_modal", script_set_modal},
		{"new_appid", script_new_appid},
		
		{"new_drwg", script_new_drwg},
		{"open_drwg", script_open_drwg},
		{"save_drwg", script_save_drwg},
		{"print_drwg", script_print_drwg},
		
		{"gui_refresh", script_gui_refresh},
		
		{"win_show", script_win_show},
		{"win_close", script_win_close},
		{"nk_layout", script_nk_layout},
		{"nk_button", script_nk_button},
		{"nk_label", script_nk_label},
		{"nk_edit", script_nk_edit},
		{"nk_propertyi", script_nk_propertyi},
		{"nk_propertyd", script_nk_propertyd},
		{"nk_combo", script_nk_combo},
		{"nk_slide_i", script_nk_slide_i},
		{"nk_slide_f", script_nk_slide_f},
		{"nk_option", script_nk_option},
		{"nk_check", script_nk_check},
		{"nk_selectable", script_nk_selectable},
		{"nk_progress", script_nk_progress},
		{"nk_group_begin", script_nk_group_begin},
		{"nk_group_end", script_nk_group_end},
		{"nk_tab_begin", script_nk_tab_begin},
		{"nk_tab_end", script_nk_tab_end},
		
		{"start_dynamic", script_start_dynamic},
		{"stop_dynamic", script_stop_dynamic},
		{"ent_draw", script_ent_draw},
		{"unique_id", script_unique_id},
		{"last_blk", script_last_blk},
		{"pdf_new", script_pdf_new},
		{NULL, NULL}
	};
	luaL_newlib(T, cz_lib);
	lua_setglobal(T, "cadzinho");
	
	/* create a new type of lua userdata to represent a DXF entity */
	static const struct luaL_Reg methods [] = {
		{"write", script_ent_write},
		{NULL, NULL}
	};
	luaL_newmetatable(T, "cz_ent_obj");
	lua_pushvalue(T, -1); /*  */
	lua_setfield(T, -2, "__index");
	luaL_setfuncs(T, methods, 0);
	lua_pop( T, 1);
	
	static const struct luaL_Reg pdf_meths[] = {
		{"close", script_pdf_close},
		{"page", script_pdf_page},
		{"save", script_pdf_save},
		{"__gc", script_pdf_close},
		{NULL, NULL}
	};
	luaL_newmetatable(T, "cz_pdf_obj");
	lua_pushvalue(T, -1); /*  */
	lua_setfield(T, -2, "__index");
	luaL_setfuncs(T, pdf_meths, 0);
	lua_pop( T, 1);
	
	static const struct luaL_Reg miniz_meths[] = {
		{"read", script_miniz_read},
		{"close", script_miniz_close},
		{"__gc", script_miniz_close},
		{NULL, NULL}
	};
	static const struct luaL_Reg miniz_funcs[] = {
		{"open", script_miniz_open},
		{"write",  script_miniz_write},
		{NULL, NULL}
	};
	luaL_newlib(T, miniz_funcs);
	lua_setglobal(T, "miniz");
	
	/* create a new type of lua userdata to represent a ZIP archive */
	/* create metatable */
	luaL_newmetatable(T, "Zip");
	/* metatable.__index = metatable */
	lua_pushvalue(T, -1);
	lua_setfield(T, -2, "__index");
	/* register methods */
	luaL_setfuncs(T, miniz_meths, 0);
	lua_pop( T, 1);
	
	static const struct luaL_Reg yxml_meths[] = {
		{"read", script_yxml_read},
		{"close", script_yxml_close},
		{"__gc", script_yxml_close},
		{NULL, NULL}
	};
	static const struct luaL_Reg yxml_funcs[] = {
		{"new", script_yxml_new},
		{NULL, NULL}
	};
	luaL_newlib(T, yxml_funcs);
	lua_setglobal(T, "yxml");
	
	/* create a new type of lua userdata to represent a XML parser */
	/* create metatable */
	luaL_newmetatable(T, "Yxml");
	/* metatable.__index = metatable */
	lua_pushvalue(T, -1);
	lua_setfield(T, -2, "__index");
	/* register methods */
	luaL_setfuncs(T, yxml_meths, 0);
	lua_pop( T, 1);
	
	/* add functions in cadzinho object*/
	static const luaL_Reg fs_lib[] = {
		{"dir", script_fs_dir },
		{"chdir", script_fs_chdir },
		{"cwd", script_fs_cwd },
		{"script_path", script_fs_script_path},
		{NULL, NULL}
	};
	luaL_newlib(T, fs_lib);
	lua_setglobal(T, "fs");
	
	static const struct luaL_Reg sqlite_meths[] = {
		{"exec", script_sqlite_exec},
		{"rows", script_sqlite_rows},
		{"cols", script_sqlite_cols},
		{"changes", script_sqlite_changes},
		{"close",  script_sqlite_close},
		{"__gc", script_sqlite_close},
		{NULL, NULL}
	};
	static const struct luaL_Reg sqlite_funcs[] = {
		{"open", script_sqlite_open},
		{NULL, NULL}
	};
	luaL_newlib(T, sqlite_funcs);
	lua_setglobal(T, "sqlite");
	
	/* create a new type of lua userdata to represent a Sqlite database */
	/* create metatable */
	luaL_newmetatable(T, "Sqlite_db");
	/* metatable.__index = metatable */
	lua_pushvalue(T, -1);
	lua_setfield(T, -2, "__index");
	/* register methods */
	luaL_setfuncs(T, sqlite_meths, 0);
	lua_pop( T, 1);
	
	/* create a new type of lua userdata to represent a Sqlite statement */
	/* create metatable */
	luaL_newmetatable(T, "Sqlite_stmt");
	lua_pushcfunction(T, script_sqlite_stmt_gc);
	lua_setfield(T, -2, "__gc");
	lua_pop( T, 1);
	
	/* adjust package path for "require" in script file*/
	luaL_Buffer b;  /* to store parcial strings */
	luaL_buffinit(T, &b); /* init the Lua buffer */
	luaL_addstring(&b, gui->base_dir);
	luaL_addstring(&b, "script");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "?.lua;");
	luaL_addstring(&b, gui->base_dir);
	luaL_addstring(&b, "script");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "?");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "init.lua;");
	
	if (strcmp (gui->base_dir, gui->pref_path) != 0){
		luaL_addstring(&b, gui->pref_path);
		luaL_addstring(&b, "script");
		luaL_addchar(&b, DIR_SEPARATOR);
		luaL_addstring(&b, "?.lua;");
		luaL_addstring(&b, gui->pref_path);
		luaL_addstring(&b, "script");
		luaL_addchar(&b, DIR_SEPARATOR);
		luaL_addstring(&b, "?");
		luaL_addchar(&b, DIR_SEPARATOR);
		luaL_addstring(&b, "init.lua;");
	}
	
	luaL_addstring(&b, ".");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "?.lua;");
	luaL_addstring(&b, ".");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "?");
	luaL_addchar(&b, DIR_SEPARATOR);
	luaL_addstring(&b, "init.lua;");
	
	luaL_pushresult(&b); /* finalize string and put on Lua stack  - new package path */
	
	lua_getglobal( T, "package");
	lua_insert( T, 1 ); /* setup stack  for next operation*/
	lua_setfield( T, -2, "path"); 
	lua_pop( T, 1); /* get rid of package table from top of stack */
	
	/* hook function to breakpoints and  timeout verification*/
	lua_sethook(T, script_check, LUA_MASKCOUNT|LUA_MASKLINE, 500);
	
	/* load lua script file */
	if (fname){
		script->status = luaL_loadfile(T, (const char *) fname);
		if ( script->status == LUA_ERRFILE ) { /* try to look in pref folder */
			char new_path[PATH_MAX_CHARS+1] = "";
			snprintf(new_path, PATH_MAX_CHARS, "%sscript%c%s", gui->pref_path, DIR_SEPARATOR, fname);
			script->status = luaL_loadfile(T, (const char *) new_path);
		}
		if ( script->status == LUA_ERRFILE && alt_chunk) {
			lua_pop(T, 1); /* pop error message from Lua stack */
			script->status = luaL_loadstring(T, (const char *) alt_chunk);
		}
	}
	else {
		script->status = luaL_loadstring(T, (const char *) alt_chunk);
	}
	
	if ( script->status == LUA_OK)  {
		lua_setglobal(T, "cz_main_func"); /* store main function in global variable */
		return 1;
	}
	
	return -1;
	
}

/* run script from file */
int gui_script_run (gui_obj *gui, struct script_obj *script, char *fname) {
	if(!gui) return 0;
	if(!script) return 0;
	char msg[DXF_MAX_CHARS];
	
	/* load lua script file */
	int st = gui_script_init (gui, script, fname, NULL);
	
	if (st == -1){
		/* error on loading */
		snprintf(msg, DXF_MAX_CHARS-1, "cannot run script file: %s", lua_tostring(script->T, -1));
		nk_str_append_str_char(&gui->debug_edit.string, msg);
		
		lua_pop(script->T, 1); /* pop error message from Lua stack */
		
		lua_close(script->L);
		script->L = NULL;
		script->T = NULL;
		script->active = 0;
		script->dynamic = 0;
	}
	
	/* run Lua script*/
	else if (st){
		
		/* set start time of script execution */
		script->time = clock();
		script->timeout = 10.0; /* default timeout value */
		script->do_init = 0;
		
		/* add main entry to do/redo list */
		//do_add_entry(&gui->list_do, "SCRIPT");
		
		lua_getglobal(script->T, "cz_main_func");
		script->n_results = 0; /* for Lua 5.4*/
		script->status = lua_resume(script->T, NULL, 0, &script->n_results); /* start thread */
		if (script->status != LUA_OK && script->status != LUA_YIELD){
			/* execution error */
			snprintf(msg, DXF_MAX_CHARS-1, "error: %s", lua_tostring(script->T, -1));
			nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			lua_pop(script->T, 1); /* pop error message from Lua stack */
		}
		/* clear variable if thread is no yielded*/
		if ((script->status != LUA_YIELD && script->active == 0 && script->dynamic == 0) ||
			(script->status != LUA_YIELD && script->status != LUA_OK)) {
			lua_close(script->L);
			script->L = NULL;
			script->T = NULL;
			script->active = 0;
			script->dynamic = 0;
		}
	}
	
	return 1;
	
}

int gui_script_exec_file_slot (gui_obj *gui, char *path) {
	int i;
	struct script_obj *gui_script = NULL;
	
	/*verify if same script file is already running */
	for (i = 0; i < MAX_SCRIPTS; i++){
		if (gui->lua_script[i].L != NULL && gui->lua_script[i].T != NULL ){
			if (strcmp (gui->lua_script[i].path, path) == 0){
				gui->lua_script[i].time = clock();
				//lua_getglobal(script->T, "cz_main_func");
				gui->lua_script[i].n_results = 0; /* for Lua 5.4*/
				gui->lua_script[i].status = lua_resume(gui->lua_script[i].T, NULL, 0, &gui->lua_script[i].n_results);
				/* return success */
				return 1;
			}
		}
	}
	
	/*try to find a available gui script slot */
	for (i = 1; i < MAX_SCRIPTS; i++){ /* start from 1 index (0 index is reserved) */
		if (gui->lua_script[i].L == NULL && gui->lua_script[i].T == NULL ){
			/* success */
			gui_script = &gui->lua_script[i];
			break;
		}
	}
	if (!gui_script){
		/* return fail */
		return 0;
	}
	
	/* run script from file */
	gui_script_run (gui, gui_script, path);
	return 1;
}

/* execute a lua script file */
/* A new Lua state is created and apended in main execution list 
given parameters:
	- file path
returns:
	- success, as boolean
*/
int gui_script_exec_file (lua_State *L) {
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
	if (!lua_isstring(L, 1)) {
		lua_pushliteral(L, "exec_file: incorrect argument type");
		lua_error(L);
	}
	
	char path[DXF_MAX_CHARS + 1];
	strncpy(path, lua_tostring(L, 1), DXF_MAX_CHARS);
	
	lua_pushboolean(L, /* return success or fail*/
		gui_script_exec_file_slot (gui, path) );
	return 1;
}

/* GUI window for scripts */
int script_win (gui_obj *gui){
	int show_script = 1;
	int i = 0;
	static int init = 0;
	static char source[DXF_MAX_CHARS], line[DXF_MAX_CHARS];
	static char glob[DXF_MAX_CHARS], loc[DXF_MAX_CHARS];
	char str_tmp[DXF_MAX_CHARS];
	
	
	enum Script_tab {
		EXECUTE,
		BREAKS,
		VARS
	} static script_tab = EXECUTE;
	
	if (!init){ /* initialize static vars */
		nk_str_clear(&gui->debug_edit.string);
		source[0] = 0;
		line[0] = 0;
		glob[0] = 0;
		loc[0] = 0;
		
		init = 1;
	}
	
	if (nk_begin(gui->ctx, "Script", nk_rect(215, 88, 400, 380),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		struct nk_style_button *sel_type;
		
		/* Tabs for select three options:
			- Load and run scripts;
			- Manage breakpoints in code;
			- View set variables; */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,0));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 3);
		if (gui_tab (gui, "Execute", script_tab == EXECUTE)) script_tab = EXECUTE;
		if (gui_tab (gui, "Breakpoints", script_tab == BREAKS)) script_tab = BREAKS;
		if (gui_tab (gui, "Variables", script_tab == VARS)) script_tab = VARS;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		/* body of tab control */
		nk_layout_row_dynamic(gui->ctx, 180, 1);
		if (nk_group_begin(gui->ctx, "Script_controls", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
			/* run script tab*/
			if (script_tab == EXECUTE){
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				nk_label(gui->ctx, "Script file:", NK_TEXT_LEFT);
				
				static int show_app_file = 0;

				/* supported image formats */
				static const char *ext_type[] = {
					"LUA",
					"*"
				};
				static const char *ext_descr[] = {
					"Lua Script (.lua)",
					"All files (*)"
				};
				#define FILTER_COUNT 2
				
				if (nk_button_label(gui->ctx, "Browse")){/* call file browser */
					show_app_file = 1;
					/* set filter for suported output formats */
					for (i = 0; i < FILTER_COUNT; i++){
						gui->file_filter_types[i] = ext_type[i];
						gui->file_filter_descr[i] = ext_descr[i];
					}
					gui->file_filter_count = FILTER_COUNT;
					gui->filter_idx = 0;
					
					gui->show_file_br = 1;
					gui->curr_path[0] = 0;
				}
				if (show_app_file){ /* running file browser */
					if (gui->show_file_br == 2){ /* return file OK */
						/* close browser window*/
						gui->show_file_br = 0;
						show_app_file = 0;
						/* update output path */
						strncpy(gui->curr_script, gui->curr_path, PATH_MAX_CHARS - 1);
					}
				}
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				
				/* user can type the file name/path, or paste text, or drop from system navigator */
				//nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_script, PATH_MAX_CHARS, nk_filter_default);
				
				nk_layout_row_static(gui->ctx, 28, 28, 6);
				if (nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_RIGHT)){
					if (gui->lua_script[0].status == LUA_YIELD){
						gui->lua_script[0].time = clock();
						gui->lua_script[0].n_results = 0; /* for Lua 5.4*/
						gui->lua_script[0].status = lua_resume(gui->lua_script[0].T, NULL, 0, &gui->lua_script[0].n_results);
						if (gui->lua_script[0].status != LUA_YIELD && gui->lua_script[0].status != LUA_OK){
							/* execution error */
							char msg[DXF_MAX_CHARS];
							snprintf(msg, DXF_MAX_CHARS-1, "error: %s", lua_tostring(gui->lua_script[0].T, -1));
							nk_str_append_str_char(&gui->debug_edit.string, msg);
							
							lua_pop(gui->lua_script[0].T, 1); /* pop error message from Lua stack */
						}
						/* clear variable if thread is no yielded*/
						if ((gui->lua_script[0].status != LUA_YIELD && gui->lua_script[0].active == 0 && gui->lua_script[0].dynamic == 0) ||
							(gui->lua_script[0].status != LUA_YIELD && gui->lua_script[0].status != LUA_OK)) {
							lua_close(gui->lua_script[0].L);
							gui->lua_script[0].L = NULL;
							gui->lua_script[0].T = NULL;
							gui->lua_script[0].active = 0;
							gui->lua_script[0].dynamic = 0;
						}
					}
					else if (gui->lua_script[0].active == 0 && gui->lua_script[0].dynamic == 0){
						gui_script_run (gui, &gui->lua_script[0], gui->curr_script);
					}
				}
				if (gui->lua_script[0].status == LUA_YIELD || gui->lua_script[0].active || gui->lua_script[0].dynamic){
					if(nk_button_symbol(gui->ctx, NK_SYMBOL_RECT_SOLID)){
						lua_close(gui->lua_script[0].L);
						gui->lua_script[0].L = NULL;
						gui->lua_script[0].T = NULL;
						
						if (gui->lua_script[0].active || gui->lua_script[0].dynamic)
							gui_default_modal(gui);
						
						gui->lua_script[0].active = 0;
						gui->lua_script[0].dynamic = 0;
					}
				}
				
			}
			/* breakpoints tab */
			else if (script_tab == BREAKS){
				static int sel_brk = -1;
				
				nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 4, (float[]){0.18f, 0.45f, 0.12f, 0.25f});
				nk_label(gui->ctx, "Source:", NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, source, DXF_MAX_CHARS - 1, nk_filter_default);
				nk_label(gui->ctx, "Line:", NK_TEXT_RIGHT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, line, DXF_MAX_CHARS - 1, nk_filter_decimal);
				
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				if (nk_button_label(gui->ctx, "Add")){
					long i_line = strtol(line, NULL, 10);
					if (i_line && strlen(source) && gui->num_brk_pts < BRK_PTS_MAX){
						gui->brk_pts[gui->num_brk_pts].line = i_line;
						strncpy(gui->brk_pts[gui->num_brk_pts].source, source, DXF_MAX_CHARS - 1);
						gui->brk_pts[gui->num_brk_pts].enable = 1;
						
						gui->num_brk_pts++;
					}
				}
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				nk_label(gui->ctx, "Breakpoints:", NK_TEXT_LEFT);
				if (nk_button_label(gui->ctx, "Remove")){
					if (sel_brk >= 0 && gui->num_brk_pts > 0){
						for (i = sel_brk; i < gui->num_brk_pts - 1; i++){
							gui->brk_pts[i] = gui->brk_pts[i + 1];
						}
						gui->num_brk_pts--;
						if (sel_brk >= gui->num_brk_pts) sel_brk = gui->num_brk_pts - 1;
						
					}
				}
				//if (nk_button_label(gui->ctx, "On/Off")){
					
				//}
				nk_layout_row_dynamic(gui->ctx, 95, 1);
				if (nk_group_begin(gui->ctx, "Breaks", NK_WINDOW_BORDER)) {
					nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 3, (float[]){0.1f, 0.7f, 0.2f});
					for (i = 0; i < gui->num_brk_pts; i++){
						
						sel_type = &gui->b_icon_unsel;
						if (i == sel_brk) sel_type = &gui->b_icon_sel;
						
						snprintf(str_tmp, DXF_MAX_CHARS-1, "%d.", i + 1);
						nk_label(gui->ctx, str_tmp, NK_TEXT_LEFT);
						
						snprintf(str_tmp, DXF_MAX_CHARS-1, "%s : %d", gui->brk_pts[i].source, gui->brk_pts[i].line);
						if (nk_button_label_styled(gui->ctx, sel_type, str_tmp)){
							sel_brk = i; /* select current text style */
						}
						if (gui->brk_pts[i].enable) snprintf(str_tmp, DXF_MAX_CHARS-1, "On");
						else snprintf(str_tmp, DXF_MAX_CHARS-1, "Off");
						if (nk_button_label_styled(gui->ctx, sel_type, str_tmp)){
							sel_brk = i; /* select current text style */
							gui->brk_pts[i].enable = !gui->brk_pts[i].enable;
						}
						
					}
					nk_group_end(gui->ctx);
				}
			}
			/* view variables tabs */
			else if (script_tab == VARS && gui->lua_script[0].status == LUA_YIELD){
				static int num_vars = 0;
				int ok = 0;
				lua_Debug ar;
				static char vars[50][DXF_MAX_CHARS];
				static char values[50][DXF_MAX_CHARS];
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "All Globals")){
					lua_pushglobaltable(gui->lua_script[0].T);
					lua_pushnil(gui->lua_script[0].T);
					i = 0;
					while (lua_next(gui->lua_script[0].T, -2) != 0) {
						snprintf(vars[i], DXF_MAX_CHARS-1, "%s", lua_tostring(gui->lua_script[0].T, -2));
						lua_getglobal(gui->lua_script[0].T, vars[i]);
						print_lua_var(values[i], gui->lua_script[0].T);
						lua_pop(gui->lua_script[0].T, 1);
						
						
						//snprintf(values[i], DXF_MAX_CHARS-1, "-");
						lua_pop(gui->lua_script[0].T, 1);
						i++;
					}
					lua_pop(gui->lua_script[0].T, 1);
					num_vars = i;
				}
				if (nk_button_label(gui->ctx, "All Locals")){
					ok = lua_getstack(gui->lua_script[0].T, 0, &ar);
					if (ok){
						i = 0;
						const char * name;

						while ((name = lua_getlocal(gui->lua_script[0].T, &ar, i+1))) {
							strncpy(vars[i], name, DXF_MAX_CHARS - 1);
							//snprintf(values[i], DXF_MAX_CHARS-1, "%s", lua_tostring(gui->lua_script[0], -1));
							print_lua_var(values[i], gui->lua_script[0].T);
							lua_pop(gui->lua_script[0].T, 1);
							i++;
						}
						num_vars = i;
					}
				}
				
				nk_layout_row(gui->ctx, NK_DYNAMIC, 145, 2, (float[]){0.3f, 0.7f});
				//nk_layout_row_dynamic(gui->ctx, 170, 2);
				if (nk_group_begin(gui->ctx, "vars", NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
					nk_layout_row_dynamic(gui->ctx, 19, 1);
					
					nk_label(gui->ctx, "Global:", NK_TEXT_LEFT);
					nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, glob, DXF_MAX_CHARS - 1, nk_filter_default);
					if (nk_button_label(gui->ctx, "Print")){
						char msg[DXF_MAX_CHARS];
						lua_getglobal(gui->lua_script[0].T, glob);
						print_lua_var(str_tmp, gui->lua_script[0].T);
						lua_pop(gui->lua_script[0].T, 1);
						
						snprintf(msg, DXF_MAX_CHARS-1, "Global %s - %s\n", glob, str_tmp);
						nk_str_append_str_char(&gui->debug_edit.string, msg);
					}
					
					nk_label(gui->ctx, "Local:", NK_TEXT_LEFT);
					nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, loc, DXF_MAX_CHARS - 1, nk_filter_decimal);
					if (nk_button_label(gui->ctx, "Print")){
						long i_loc = strtol(loc, NULL, 10);
						char msg[DXF_MAX_CHARS];
						ok = lua_getstack(gui->lua_script[0].T, 0, &ar);
						if (ok){
							const char * name;

							if (name = lua_getlocal(gui->lua_script[0].T, &ar, i_loc)) {
								
								print_lua_var(str_tmp, gui->lua_script[0].T);
								snprintf(msg, DXF_MAX_CHARS-1, "Local %s - %s\n", name, str_tmp);
								nk_str_append_str_char(&gui->debug_edit.string, msg);
								lua_pop(gui->lua_script[0].T, 1);
								
							}
						}
						
						
					}
					
					nk_group_end(gui->ctx);
				}
				if (nk_group_begin(gui->ctx, "list_vars", NK_WINDOW_BORDER)) {
					nk_layout_row_dynamic(gui->ctx, 20, 2);
					
					
					
					for (i = 0; i < num_vars; i++){
						
						sel_type = &gui->b_icon_unsel;
						//if (i == sel_brk) sel_type = &gui->b_icon_sel;
						
						
						if (nk_button_label_styled(gui->ctx, sel_type, vars[i])){
							
						}
						if (nk_button_label_styled(gui->ctx, sel_type, values[i])){
							
						}
						
					}
					nk_group_end(gui->ctx);
				}
			}
			
			nk_group_end(gui->ctx);
		}
		
		/* text edit control - emulate stdout, showing script "print" outputs */ 
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_label(gui->ctx, "Output:", NK_TEXT_LEFT);
		if (nk_button_label(gui->ctx, "Clear")){ /* clear text */
			nk_str_clear(&gui->debug_edit.string);
		}
		nk_layout_row_dynamic(gui->ctx, 100, 1);
		nk_edit_buffer_wrap(gui->ctx, NK_EDIT_EDITOR|NK_EDIT_GOTO_END_ON_ACTIVATE, &(gui->debug_edit), nk_filter_default);
		
		
		
		
	} else {
		show_script = 0;
		//init = 0;
	}
	nk_end(gui->ctx);
	
	return show_script;
}

/*int script_win_k (lua_State *L, int status, lua_KContext ctx) {
	return status;
}*/

int gui_script_interactive(gui_obj *gui){
	static int i, j;
	
	for (i = 0; i < MAX_SCRIPTS; i++){ /* sweep script slots and execute each valid */
		/* window functions */
		if (gui->lua_script[i].L != NULL && gui->lua_script[i].T != NULL){
			if (strlen(gui->lua_script[i].win) > 0 && gui->lua_script[i].active &&
				gui->lua_script[i].status != LUA_YIELD)
			{
				int win;
				
				/* different windows names, according its index, to prevent crashes in nuklear */
				char win_id[32];
				snprintf(win_id, 31, "script_win_%d", i);
				/* create window */
				if (win = nk_begin_titled (gui->ctx, win_id, gui->lua_script[i].win_title,
					nk_rect(gui->lua_script[i].win_x, gui->lua_script[i].win_y,
						gui->lua_script[i].win_w, gui->lua_script[i].win_h),
					NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
					NK_WINDOW_SCALABLE|
					NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
				{
					int n = lua_gettop(gui->lua_script[i].T);
					if (n){
						lua_pop(gui->lua_script[i].T, n);
					}
					lua_getglobal(gui->lua_script[i].T, gui->lua_script[i].win);
					gui->lua_script[i].time = clock();
					//gui->lua_script[i].status = lua_pcallk(gui->lua_script[i].T, 0, 0, 0, &gui->lua_script[i], &script_win_k);
					
					gui->lua_script[i].n_results = 0; /* for Lua 5.4*/
					gui->lua_script[i].status = lua_resume(gui->lua_script[i].T, NULL, 0, &gui->lua_script[i].n_results); /* start thread */
					
					/* close pending nk_groups, to prevent nuklear crashes */
					for (j = 0; j < gui->lua_script[i].groups; j++){
						nk_group_end(gui->ctx);
					}
					gui->lua_script[i].groups = 0;
				}
				nk_end(gui->ctx); /* not allow user to ends windows, to prevent nuklear crashes */
				
				if (!win){
					gui->lua_script[i].active = 0;
					gui->lua_script[i].win[0] = 0;
				}
			}
			if (gui->lua_script[i].status != LUA_YIELD && gui->lua_script[i].status != LUA_OK){
				/* execution error */
				char msg[DXF_MAX_CHARS];
				snprintf(msg, DXF_MAX_CHARS-1, "error: %s", lua_tostring(gui->lua_script[i].T, -1));
				
				if ( i == 0 ){
					nk_str_append_str_char(&gui->debug_edit.string, msg);
				} else {
					snprintf(gui->log_msg, 63, "Script %s", msg);
				}
				
				lua_pop(gui->lua_script[i].T, 1); /* pop error message from Lua stack */
				
				gui_default_modal(gui); /* back to default modal */
			}
			
			if (gui->lua_script[i].status != LUA_YIELD) {
				gui->lua_script[i].do_init = 0; /* reinit script do list */
			}
			
			if((gui->lua_script[i].status != LUA_YIELD && gui->lua_script[i].active == 0 && gui->lua_script[i].dynamic == 0) ||
				(gui->lua_script[i].status != LUA_YIELD && gui->lua_script[i].status != LUA_OK))
			{
				/* clear inactive script slots */
				lua_close(gui->lua_script[i].L);
				gui->lua_script[i].L = NULL;
				gui->lua_script[i].T = NULL;
				gui->lua_script[i].active = 0;
				gui->lua_script[i].dynamic = 0;
				gui->lua_script[i].win[0] = 0;
				gui->lua_script[i].dyn_func[0] = 0;
			}
			
			/* resume script waiting gui condition 
			if (gui->script_resume && gui->lua_script[i].status == LUA_YIELD &&
				gui->lua_script[i].wait_gui_resume)
			{
				
				gui->lua_script[i].wait_gui_resume = gui->script_resume;
				gui->script_resume =  0;
				gui->lua_script[i].time = clock();
				gui->lua_script[i].n_results = 0;
				gui->lua_script[i].status = lua_resume(gui->lua_script[i].T, NULL, 0, &gui->lua_script[i].n_results);
			}
			*/
		}
	}
	
	if (gui->script_resume && gui->script_wait_t.wait_gui_resume)
	{
		/* get script object from Lua instance */
		lua_pushstring(gui->script_wait_t.T, "cz_script"); /* is indexed as  "cz_script" */
		lua_gettable(gui->script_wait_t.T, LUA_REGISTRYINDEX); 
		struct script_obj *script = lua_touserdata (gui->script_wait_t.T, -1);
		lua_pop(gui->script_wait_t.T, 1);
		
		gui->script_wait_t.wait_gui_resume = gui->script_resume;
		gui->script_resume =  0;
		
		if (gui->script_wait_t.T == script->T){
			script->time = clock();
			script->n_results = 0;
			script->status = lua_resume(gui->script_wait_t.T, NULL, 0, &script->n_results);
		}
		else {
			int n_results = 0;
			lua_resume(gui->script_wait_t.T, NULL, 0, &n_results);
		}
	}
	
	return 1;
}

int gui_script_dyn(gui_obj *gui){
	if (gui->modal == SCRIPT) {
		gui->phanton = NULL;
		gui->draw_phanton = 0;
	}
	else {
		gui_script_clear_dyn(gui);
		return 0;
	}
	
	static int i, j;
	for (i = 0; i < MAX_SCRIPTS; i++){ /* sweep script slots and execute each valid */
		
		/* dynamic functions */
		if (gui->lua_script[i].L != NULL && gui->lua_script[i].T != NULL &&
			strlen(gui->lua_script[i].dyn_func) > 0 && gui->lua_script[i].dynamic)
		{
			gui->lua_script[i].time = clock(); /* refresh clock */
			
			/* get dynamic function */
			lua_getglobal(gui->lua_script[i].T, gui->lua_script[i].dyn_func);
			/* pass current mouse position */
			lua_createtable (gui->lua_script[i].T, 0, 3);
			lua_pushnumber(gui->lua_script[i].T,  gui->step_x[gui->step]);
			lua_setfield(gui->lua_script[i].T, -2, "x");
			lua_pushnumber(gui->lua_script[i].T,  gui->step_y[gui->step]);
			lua_setfield(gui->lua_script[i].T, -2, "y");
			/* pass events */
			if (gui->ev & EV_CANCEL)
				lua_pushliteral(gui->lua_script[i].T, "cancel");
			else if (gui->ev & EV_ENTER){
				lua_pushliteral(gui->lua_script[i].T, "enter");
				if (gui->step == 0){
					gui->step = 1;
					gui->en_distance = 1;
					gui_next_step(gui);
				}
				else {
					gui->step_x[0] = gui->step_x[1];
					gui->step_y[0] = gui->step_y[1];
					gui_next_step(gui);
				}
			}
			else if (gui->ev & EV_MOTION)
				lua_pushliteral(gui->lua_script[i].T, "motion");
			else
				lua_pushliteral(gui->lua_script[i].T, "none");
			lua_setfield(gui->lua_script[i].T, -2, "type");
			
			/*finally call the function */
			gui->lua_script[i].status = lua_pcall(gui->lua_script[i].T, 1, 0, 0);
			
			/* close pending nk_groups, to prevent nuklear crashes */
			for (j = 0; j < gui->lua_script[i].groups; j++){
				nk_group_end(gui->ctx);
			}
			gui->lua_script[i].groups = 0;
			
			if(gui->phanton) gui->draw_phanton = 1;
			
			if (!gui->lua_script[i].dynamic){
				gui->lua_script[i].dyn_func[0] = 0;
			}
			return 1;
		}
	}
	return 1;
}

int gui_script_clear_dyn(gui_obj *gui){
	static int i;
	for (i = 0; i < MAX_SCRIPTS; i++){ /* sweep script slots */
		/* clear all dynamic */
		gui->lua_script[i].dynamic = 0;
		gui->lua_script[i].dyn_func[0] = 0;
	}
	return 1;
}