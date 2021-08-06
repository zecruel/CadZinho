#include "gui_config.h"

const char* gui_dflt_conf() {
	static const char *conf = "-- CadZinho enviroment configuration file\n"
	"-- This file is writen in Lua language\n"
	"-- NOTE1: The purpose of this script file is load global initial parameters. Don't expect script features to fully work.\n"
	"-- NOTE2: For Windows, strings with path dir separator '\\' must be escaped, eg. \"C:\\\\mydir\\\\myfile.lua\"\n\n"
	"-- Paths to look for font files. Each path is separeted by \";\" or \":\", according system default\n"
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)|| defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
		"font_path = \"C:\\\\Windows\\\\Fonts\\\\\"\n\n"
	#elif __APPLE__ || __MACH__
		"font_path = \"/Library/Fonts/\"\n\n"
	#else
		"font_path = \"/usr/share/fonts/\"\n\n"
	#endif
	"-- List of fonts to be loaded at startup (especify only file name, without path)\n"
	"fonts = {\n"
	"    \"romans.shx\",\n" 
	"    \"txt.shx\"\n"
	"}\n\n"
	"-- Font to use in user interface (must be preloaded). File and size in pts\n"
	"ui_font = {\"txt.shx\", 10}\n\n"
	"-- Interface theme - green (default), black, white, red, blue, dark, brown or purple\n"
	"theme = \"green\"\n\n";
	return conf;
}

int gui_get_conf (lua_State *L) {
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
		
	/* -------------------- get theme -------------------*/
	lua_getglobal(L, "theme");
	if (lua_isstring(L, -1)){
		const char *theme = lua_tostring(L, -1);
		//enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_GREEN};
		if (strcmp(theme, "green") == 0){
			gui->theme = THEME_GREEN;
		}
		else if (strcmp(theme, "black") == 0){
			gui->theme = THEME_BLACK;
		}
		else if (strcmp(theme, "white") == 0){
			gui->theme = THEME_WHITE;
		}
		else if (strcmp(theme, "red") == 0){
			gui->theme = THEME_RED;
		}
		else if (strcmp(theme, "blue") == 0){
			gui->theme = THEME_BLUE;
		}
		else if (strcmp(theme, "dark") == 0){
			gui->theme = THEME_DARK;
		}
		else if (strcmp(theme, "brown") == 0){
			gui->theme = THEME_BROWN;
		}
		else if (strcmp(theme, "purple") == 0){
			gui->theme = THEME_PURPLE;
		}
		else if (strcmp(theme, "default") == 0){
			gui->theme = THEME_DEFAULT;
		}
	}
	
	/* -------------------- get fonts paths -------------------*/
	lua_getglobal(L, "font_path");
	if (lua_isstring(L, -1)){
		const char *font_path = lua_tostring(L, -1);
		strncat(gui->dflt_fonts_path, font_path, 5 * DXF_MAX_CHARS);
		
	}
	else{ /* default value, if not definied in file*/
		const char * font_dir = dflt_fonts_dir ();
		if (font_dir)
			strncat(gui->dflt_fonts_path, font_dir, 5 * DXF_MAX_CHARS);
		else
			strncat(gui->dflt_fonts_path, "/", 5 * DXF_MAX_CHARS);
		
	}
	lua_pop(L, 1);
	
	/* -------------------- load list of fonts  -------------------*/
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
			struct tfont *ui_font = get_font_list(gui->font_list, (char *)lua_tostring(L, -1));
			if (!ui_font) { /* default font, if fail to find in list*/
				ui_font = get_font_list(gui->font_list, "txt.shx");
			}
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		else{ /* default value, if not definied in file*/
			struct tfont *ui_font = get_font_list(gui->font_list, "txt.shx");
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		lua_pop(L, 1);
		
		/*---- size of text */
		lua_pushnumber(L, 2); /* key*/
		lua_gettable(L, -2);  /* get table[key] */
		if (lua_isnumber(L, -1)){
			gui->ui_font.height = lua_tonumber(L, -1);
		}
		else{/* default value, if not definied in file*/
			gui->ui_font.height = 10.0;
		}
		lua_pop(L, 1);		
		
	}
	else { /* default font, if not definied in file*/
		struct tfont *ui_font = get_font_list(gui->font_list, "txt.shx");
		gui->ui_font.userdata = nk_handle_ptr(ui_font);
		gui->ui_font.height = 10.0;
	}
	lua_pop(L, 1);
	
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
	fprintf(file, "-- CadZinho session states save\n");
	fprintf(file, "-- This file is writed automatically at end of CadZinho session\n\n");
	fprintf(file, "win_x = %d\n", gui->win_x);
	fprintf(file, "win_y = %d\n", gui->win_y);
	fprintf(file, "win_width = %d\n", gui->win_w);
	fprintf(file, "win_height = %d\n", gui->win_h);
	
	fprintf(file, "recent = {\n");
	for (i = 1; i <= gui->drwg_rcnt_size; i++){
		/* get position in array, considering as circular buffer */
		int pos = (gui->drwg_rcnt_pos - i);
		if (pos < 0) pos = DRWG_RECENT_MAX + pos;
		fprintf(file, "    \"");
		path_escape(gui->drwg_recent[pos], file);
		fprintf(file, "\",\n");
	}
	fprintf(file, "}\n");
	
	fclose(file);
}

int gui_get_ini (lua_State *L) {
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
	
	/* get window position */
	lua_getglobal(L, "win_x");
	if (lua_isnumber(L, -1)) gui->win_x = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	lua_getglobal(L, "win_y");
	if (lua_isnumber(L, -1)) gui->win_y = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	/* -------------------- get screen width -------------------*/
	lua_getglobal(L, "win_width");
	if (lua_isnumber(L, -1)) gui->win_w = (int)lua_tonumber(L, -1);
	else gui->win_w = 1120; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get screen height -------------------*/
	lua_getglobal(L, "win_height");
	if (lua_isnumber(L, -1)) gui->win_h = (int)lua_tonumber(L, -1);
	else gui->win_h = 600; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- load list of recent drawing files  -------------------*/
	lua_getglobal(L, "recent");
	if (lua_istable(L, -1)){
		int len = lua_rawlen(L, -1);
		int i;
		
		for (i = len; i > 0; i--){ /* iterate over table in reverse order */
			lua_rawgeti(L, -1, i); 
			if (lua_isstring(L, -1)){
				/* put file path in recent file list */
				strncpy (gui->drwg_recent[gui->drwg_rcnt_pos], lua_tostring(L, -1) , DXF_MAX_CHARS);
				if (gui->drwg_rcnt_pos < DRWG_RECENT_MAX - 1)
					gui->drwg_rcnt_pos++;
				else gui->drwg_rcnt_pos = 0; /* circular buffer */
				
				if (gui->drwg_rcnt_size < DRWG_RECENT_MAX)
					gui->drwg_rcnt_size++;
				
			}
			
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	
	return 1;
}

int config_win (gui_obj *gui){
	int show_config = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 200;
	gui->next_win_h = 300;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "config", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, "Config", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Pref Path:", NK_TEXT_LEFT);
		if (nk_button_label(gui->ctx, "Open dir")){
			if(gui->pref_path) opener(gui->pref_path);
			else {
				
			}
		}
		if (nk_button_label(gui->ctx, "Open config")){
			/* full path of config file */
			char config_path[DXF_MAX_CHARS + 1];
			config_path[0] = 0;
			strncpy(config_path, gui->pref_path, DXF_MAX_CHARS);
			strncat(config_path, "config.lua", DXF_MAX_CHARS);
			opener(config_path);
		}
		
	} else show_config = 0;
	nk_end(gui->ctx);
	
	return show_config;
}