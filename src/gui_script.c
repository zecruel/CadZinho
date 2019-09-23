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
			lua_pushstring(L, "Auto check: no access to CadZinho enviroment");
			lua_error(L);
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
		/* get gui object from Lua instance */
		lua_pushstring(L, "cz_gui"); /* is indexed as  "cz_gui" */
		lua_gettable(L, LUA_REGISTRYINDEX); 
		gui_obj *gui = lua_touserdata (L, -1);
		lua_pop(L, 1);
		
		if (!gui){ /* error in gui object access */
			lua_pushstring(L, "Auto check: no access to CadZinho enviroment");
			lua_error(L);
			return;
		}
		
		clock_t end_t;
		double diff_t;
		/* get the elapsed time since script starts or continue */
		end_t = clock();
		diff_t = (double)(end_t - gui->script_time) / CLOCKS_PER_SEC;
		
		/* verify if timeout is reachead. Its made to prevent user script stuck main program*/
		if (diff_t >= gui->script_timeout){
			char msg[DXF_MAX_CHARS];
			lua_getinfo(L, "Sl", ar); /* fill debug informations */
			
			/* stop script execution */
			snprintf(msg, DXF_MAX_CHARS-1, "Auto check: reached number of iterations on %s, line %d, exec time %f\n", ar->source, ar->currentline, diff_t);
			nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			lua_pushstring(L, "Auto check: Script execution time exceeds timeout");
			lua_error(L);
			return;
		}
	}
}

/* equivalent to a "print" lua function, that outputs to a text edit widget */
static int debug_print (lua_State *L) {
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
				snprintf(msg, DXF_MAX_CHARS - 1, "%d", lua_toboolean(L, i) ? 1 : 0);
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

/* run script from file */
int script_run (gui_obj *gui, char *fname) {
	if(!gui->lua_main) return 0;
	char msg[DXF_MAX_CHARS];
	
	/* create a new lua thread, allowing yield */
	lua_State *T = lua_newthread(gui->lua_main);
	if(!T) return 0;
	gui->lua_script = T;
	
	/* put the gui structure in lua global registry */
	lua_pushstring(T, "cz_gui");
	lua_pushlightuserdata(T, (void *)gui);
	lua_settable(T, LUA_REGISTRYINDEX);
	
	/* add functions in cadzinho object*/
	static const luaL_Reg cz_lib[] = {
		{"db_print",   debug_print},
		{"set_timeout", set_timeout},
		{"ent_append", script_ent_append},
		{"new_pline", script_new_pline},
		{"pline_append", script_pline_append},
		{NULL, NULL}
	};
	luaL_newlib(T, cz_lib);
	lua_setglobal(T, "cadzinho");
	
	/* set start time of script execution */
	gui->script_time = clock();
	gui->script_timeout = 10.0; /* default timeout value */
	
	/* hook function to breakpoints and  timeout verification*/
	lua_sethook(T, script_check, LUA_MASKCOUNT|LUA_MASKLINE, 500);
		
	/* load lua script file */
	if (luaL_loadfile(T, (const char *) fname) != LUA_OK){
		/* error on loading */
		snprintf(msg, DXF_MAX_CHARS-1, "cannot run script file: %s", lua_tostring(T, -1));
		nk_str_append_str_char(&gui->debug_edit.string, msg);
		
		lua_pop(T, 1); /* pop error message from Lua stack */
	}
	
	/* run Lua script*/
	else {
		/* add main entry to do/redo list */
		do_add_entry(&gui->list_do, "SCRIPT");
		
		int e = lua_resume(T, NULL, 0); /* start thread */
		if (e != LUA_OK && e != LUA_YIELD){
			/* execution error */
			snprintf(msg, DXF_MAX_CHARS-1, "error: %s", lua_tostring(T, -1));
			nk_str_append_str_char(&gui->debug_edit.string, msg);
			
			lua_pop(T, 1); /* pop error message from Lua stack */
		}
		/* clear variable if thread is no yielded*/
		if (e != LUA_YIELD) gui->lua_script = NULL;
	}
	
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
	
	if (!init){ /* initialize static strings */
		nk_str_clear(&gui->debug_edit.string);
		source[0] = 0;
		line[0] = 0;
		glob[0] = 0;
		loc[0] = 0;
		init = 1;
	}
	
	if (nk_begin(gui->ctx, "Script", nk_rect(gui->win_w - 404, gui->win_h - 470, 400, 380),
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
				nk_layout_row_dynamic(gui->ctx, 20, 1);
				nk_label(gui->ctx, "Script file:", NK_TEXT_LEFT);
				
				/* user can type the file name/path, or paste text, or drop from system navigator */
				//nk_edit_focus(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER|NK_EDIT_SELECTABLE|NK_EDIT_AUTO_SELECT);
				nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, gui->curr_script, MAX_PATH_LEN, nk_filter_default);
				
				nk_layout_row_static(gui->ctx, 28, 28, 6);
				nk_button_symbol(gui->ctx, NK_SYMBOL_TRIANGLE_RIGHT);
				nk_button_symbol(gui->ctx, NK_SYMBOL_RECT_SOLID);
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				//nk_button_symbol(struct nk_context*, enum nk_symbol_type);
				if (nk_button_label(gui->ctx, "Run")){
					script_run (gui, gui->curr_script);
				}
				if (nk_button_label(gui->ctx, "Continue")){
					if (gui->lua_script) {
						gui->script_time = clock();
						lua_resume(gui->lua_script, NULL, 0);
						
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
			else if (script_tab == VARS && gui->lua_script){
				static int num_vars = 0;
				int ok = 0;
				lua_Debug ar;
				static char vars[50][DXF_MAX_CHARS];
				static char values[50][DXF_MAX_CHARS];
				
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				if (nk_button_label(gui->ctx, "All Globals")){
					lua_pushglobaltable(gui->lua_script);
					lua_pushnil(gui->lua_script);
					i = 0;
					while (lua_next(gui->lua_script, -2) != 0) {
						snprintf(vars[i], DXF_MAX_CHARS-1, "%s", lua_tostring(gui->lua_script, -2));
						lua_getglobal(gui->lua_script, vars[i]);
						print_lua_var(values[i], gui->lua_script);
						lua_pop(gui->lua_script, 1);
						
						
						//snprintf(values[i], DXF_MAX_CHARS-1, "-");
						lua_pop(gui->lua_script, 1);
						i++;
					}
					lua_pop(gui->lua_script, 1);
					num_vars = i;
				}
				if (nk_button_label(gui->ctx, "All Locals")){
					ok = lua_getstack(gui->lua_script, 0, &ar);
					if (ok){
						i = 0;
						const char * name;

						while ((name = lua_getlocal(gui->lua_script, &ar, i+1))) {
							strncpy(vars[i], name, DXF_MAX_CHARS - 1);
							//snprintf(values[i], DXF_MAX_CHARS-1, "%s", lua_tostring(gui->lua_script, -1));
							print_lua_var(values[i], gui->lua_script);
							lua_pop(gui->lua_script, 1);
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
						lua_getglobal(gui->lua_script, glob);
						print_lua_var(str_tmp, gui->lua_script);
						lua_pop(gui->lua_script, 1);
						
						snprintf(msg, DXF_MAX_CHARS-1, "Global %s - %s\n", glob, str_tmp);
						nk_str_append_str_char(&gui->debug_edit.string, msg);
					}
					
					nk_label(gui->ctx, "Local:", NK_TEXT_LEFT);
					nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE | NK_EDIT_CLIPBOARD, loc, DXF_MAX_CHARS - 1, nk_filter_decimal);
					if (nk_button_label(gui->ctx, "Print")){
						long i_loc = strtol(loc, NULL, 10);
						char msg[DXF_MAX_CHARS];
						ok = lua_getstack(gui->lua_script, 0, &ar);
						if (ok){
							const char * name;

							if (name = lua_getlocal(gui->lua_script, &ar, i_loc)) {
								
								print_lua_var(str_tmp, gui->lua_script);
								snprintf(msg, DXF_MAX_CHARS-1, "Local %s - %s\n", name, str_tmp);
								nk_str_append_str_char(&gui->debug_edit.string, msg);
								lua_pop(gui->lua_script, 1);
								
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