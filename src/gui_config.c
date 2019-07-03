#include "gui_config.h"


int getglobint (lua_State *L, const char *var) {
	int isnum, result;
	lua_getglobal(L, var);
	result = (int)lua_tointegerx(L, -1, &isnum);
	if (!isnum)
		printf("'%s' should be a number\n", var);
	lua_pop(L, 1); /* remove result from the stack */
	return result;
}
void load (lua_State *L, const char *fname, int *w, int *h) {
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
		printf("cannot run config. file: %s", lua_tostring(L, -1));
	*w = getglobint(L, "width");
	*h = getglobint(L, "height");
}

int gui_load_conf (const char *fname, gui_obj *gui) {
	lua_State *L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */
	
	
	/* load configuration file as Lua script*/
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0))
		printf("cannot run config. file: %s", lua_tostring(L, -1));
	
	
	
	/* -------------------- get screen width -------------------*/
	lua_getglobal(L, "width");
	if (lua_isnumber(L, -1)) gui->win_w = (int)lua_tonumber(L, -1);
	else gui->win_w = 1200; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get screen height -------------------*/
	lua_getglobal(L, "height");
	if (lua_isnumber(L, -1)) gui->win_h = (int)lua_tonumber(L, -1);
	else gui->win_h = 710; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get fonts paths -------------------*/
	lua_getglobal(L, "font_path");
	if (lua_isstring(L, -1)){
		const char *font_path = lua_tostring(L, -1);
		strncat(gui->dflt_fonts_path, font_path, 5 * DXF_MAX_CHARS);
		
	}
	else{ /* default value, if not definied in file*/
		#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
		strncpy(gui->dflt_fonts_path, "C:\\Windows\\Fonts\\", 5 * DXF_MAX_CHARS);
		#else
		strncat(gui->dflt_fonts_path, "/usr/share/fonts/", 5 * DXF_MAX_CHARS);
		#endif
	}
	lua_pop(L, 1);
	
	/* -------------------- load list of extra fonts  -------------------*/
	lua_getglobal(L, "fonts");
	if (lua_istable(L, -1)){
		
		/* iterate over table */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) { /* table index are shifted*/
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			//printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
			if (lua_isstring(L, -1)){
				//printf("%s\n", lua_tostring(L, -1));
				add_font_list(gui->font_list, (char *)lua_tostring(L, -1), gui->dflt_fonts_path);
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	
	/* -------------------- get font for use in GUI  -------------------*/
	lua_getglobal(L, "ui_font");
	if (lua_istable(L, -1)){
		/*---- name of font */
		lua_pushnumber(L, 1); /* key*/
		lua_gettable(L, -2);  /* get table[key] */
		if (lua_isstring(L, -1)){
			//printf("%s\n", lua_tostring(L, -1));
			struct tfont *ui_font = get_font_list(gui->font_list, (char *)lua_tostring(L, -1));
			if (!ui_font) { /* default font, if fail to find in list*/
				ui_font = get_font_list(gui->font_list, "romans.shx");
			}
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		else{ /* default value, if not definied in file*/
			struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		lua_pop(L, 1);
		
		/*---- size of text */
		lua_pushnumber(L, 2); /* key*/
		lua_gettable(L, -2);  /* get table[key] */
		if (lua_isnumber(L, -1)){
			//printf("%0.2f\n", lua_tonumber(L, -1));
			gui->ui_font.height = lua_tonumber(L, -1);
		}
		else{/* default value, if not definied in file*/
			gui->ui_font.height = 10.0;
		}
		lua_pop(L, 1);
		
		#if(0)
		/* iterate over table */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) { /* table index are shifted*/
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
		#endif
	}
	else
	{ /* default font, if not definied in file*/
		struct tfont *ui_font = get_font_list(gui->font_list, "romans.shx");
		gui->ui_font.userdata = nk_handle_ptr(ui_font);
		gui->ui_font.height = 10.0;
	}
	lua_pop(L, 1);
	
	lua_close(L);
	
	return 1;
}
int path_escape (char* path, FILE *file){
	if (!path || !file) return 0;
	while (*path){
		if (*path == '\\') fprintf(file, "\\");
		fprintf(file, "%c", *path);
		path++;
	}
	return 1;
}
int gui_save_init (char *fname, gui_obj *gui){
	FILE *file;
	int ret_success = 0;
	int i;
	
	file = fopen(fname, "w"); /* open the file */
	if (!file) return 0;
	
	fprintf(file, "\nwin_width = %d\n", gui->win_w);
	fprintf(file, "win_height = %d\n", gui->win_h);
	fprintf(file, "font_path = \"");
	path_escape(gui->dflt_fonts_path, file);
	fprintf(file, "\"\n");
	
	fprintf(file, "recent = {\n");
	
	for (i = gui->drwg_hist_size - 1; i >= 0; i--){
		/* get position in array, considering as circular buffer */
		int pos = (i + gui->drwg_hist_head) % DRWG_HIST_MAX;
		fprintf(file, "\t\"");
		path_escape(gui->drwg_hist[pos], file);
		fprintf(file, "\",\n");
	}
	fprintf(file, "}\n");
	
	fclose(file);
}