#include "gui_script.h"

/* ************* TEST ******************** */
void script_check(lua_State *L, lua_Debug *ar){
	// Only listen to "Hook Lines" events
	if(ar->event == LUA_HOOKLINE){
		/* get gui object from Lua instance */
		lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
		lua_gettable(L, LUA_REGISTRYINDEX); 
		gui_obj *gui = lua_touserdata (L, -1);
		lua_pop(L, 1);
		
		if (!gui){
			lua_pushstring(L, "Auto check: no access to CadZinho enviroment");
			lua_error(L);
			return;
		}
		
		char msg[DXF_MAX_CHARS];
		char source[DXF_MAX_CHARS];
		
		lua_getinfo(L, "Sl", ar);
		strncpy(source, get_filename(ar->short_src), DXF_MAX_CHARS - 1);
		
		int i;
		for (i = 0; i < gui->num_brk_pts; i++){
			if (ar->currentline == gui->brk_pts[i].line) {
				if (strcmp(source, gui->brk_pts[i].source) == 0){
					lua_yield (L, 0);
					snprintf(msg, DXF_MAX_CHARS-1, "db: Thread paused at: %s-line %d\n", source, ar->currentline);
					nk_str_append_str_char(&gui->debug_edit.string, msg);
				}
			}
		}
	}
	else if(ar->event == LUA_HOOKCOUNT){
		/* get gui object from Lua instance */
		lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
		lua_gettable(L, LUA_REGISTRYINDEX); 
		gui_obj *gui = lua_touserdata (L, -1);
		lua_pop(L, 1);
		
		if (!gui){
			lua_pushstring(L, "Auto check: no access to CadZinho enviroment");
			lua_error(L);
			return;
		}
		
		clock_t end_t;
		double diff_t;
		end_t = clock();
		diff_t = (double)(end_t - gui->script_time) / CLOCKS_PER_SEC;
		
		if (diff_t >= gui->script_timeout){
			char msg[DXF_MAX_CHARS];
			lua_getinfo(L, "Sl", ar);
			snprintf(msg, DXF_MAX_CHARS-1, "Auto check: reached number of iterations on %s, line %d, exec time %f\n", ar->source, ar->currentline, diff_t);
			nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			lua_pushstring(L, "Auto check: Script execution time exceeds timeout");
			lua_error(L);
			return;
		}
	}
}

static int debug_print (lua_State *L) {
	/* get gui object from Lua instance */
	lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
	lua_gettable(L, LUA_REGISTRYINDEX); 
	gui_obj *gui = lua_touserdata (L, -1);
	lua_pop(L, 1);
	
	if (gui){
		char msg[DXF_MAX_CHARS];
		snprintf(msg, DXF_MAX_CHARS-1, "db: %s\n", lua_tostring(L, -1));
		nk_str_append_str_char(&gui->debug_edit.string, msg);
	}
}

int script_run (gui_obj *gui, char *fname) {
	if(!gui->lua_main) return 0;
	char msg[DXF_MAX_CHARS];
	//lua_State *L = luaL_newstate(); /* opens Lua */
	//luaL_openlibs(L); /* opens the standard libraries */
	
	lua_State *T = lua_newthread(gui->lua_main);
	if(!T) return 0;
	gui->lua_script = T;
	
	lua_pushstring(T, "cz_gui");
	lua_pushlightuserdata(T, (void *)gui);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	static const luaL_Reg cz_lib[] = {
		{"db_print",   debug_print},
		{NULL, NULL}
	};
	luaL_newlib(T, cz_lib);
	lua_setglobal(T, "cadzinho");
	
	gui->script_time = clock();
	gui->script_timeout = 10.0;
	
	
	/* ******************* TEST ******************** */
	//lua_sethook(L, script_check, LUA_MASKLINE, 0);
	lua_sethook(T, script_check, LUA_MASKCOUNT|LUA_MASKLINE, 500);
		
	if (luaL_loadfile(T, (const char *) fname) != LUA_OK){
		
		snprintf(msg, DXF_MAX_CHARS-1, "cannot run script file: %s", lua_tostring(T, -1));
		nk_str_append_str_char(&gui->debug_edit.string, msg);
		
		lua_pop(T, 1); /* pop error message from Lua stack */
	}
	
	/* run file as Lua script*/
	else {
		int e = lua_resume(T, NULL, 0);
		if (e != LUA_OK && e != LUA_YIELD){
			snprintf(msg, DXF_MAX_CHARS-1, "error: %s", lua_tostring(T, -1));
			nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			lua_pop(T, 1); /* pop error message from Lua stack */
		}
		if (e != LUA_YIELD) gui->lua_script = NULL;
	}
	
	//lua_close(L);
	
	return 1;
	
}

int script_win (gui_obj *gui){
	int show_script = 1;
	int i = 0;
	
	static int init = 0;
	
	if (!init){
		nk_str_clear(&gui->debug_edit.string);
		init = 1;
	}
	
	if (nk_begin(gui->ctx, "Script", nk_rect(gui->win_w - 404, gui->win_h - 350, 400, 250),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Script file:", NK_TEXT_LEFT);
		
		/* user can type the file name/path, or paste text, or drop from system navigator */
		//nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
		nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_script, MAX_PATH_LEN, nk_filter_default);
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		if (nk_button_label(gui->ctx, "Run")){
			/* ----------------------------- */
			gui->num_brk_pts = 2;
			gui->brk_pts[0].line = 3;
			strncpy(gui->brk_pts[0].source, "test.lua", DXF_MAX_CHARS - 1);
			gui->brk_pts[1].line = 5;
			strncpy(gui->brk_pts[1].source, "test.lua", DXF_MAX_CHARS - 1);
			/* ----------------------------- */
			script_run (gui, gui->curr_script);
		}
		if (nk_button_label(gui->ctx, "Continue")){
			if (gui->lua_script) {
				lua_resume(gui->lua_script, NULL, 0);
				gui->script_time = clock();
			}
		}
		
		
		nk_layout_row_dynamic(gui->ctx, 20, 2);
		nk_label(gui->ctx, "Output:", NK_TEXT_LEFT);
		if (nk_button_label(gui->ctx, "Clear")){
			nk_str_clear(&gui->debug_edit.string);
		}
		nk_layout_row_dynamic(gui->ctx, 100, 1);
		nk_edit_buffer_wrap(gui->ctx, NK_EDIT_EDITOR, &(gui->debug_edit), nk_filter_default);
		
	} else {
		show_script = 0;
		//init = 0;
	}
	nk_end(gui->ctx);
	
	return show_script;
}