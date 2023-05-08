#include "gui_config.h"
#include "gui_script.h"

const char * no_flag = "<svg width=\"88.0000\" height=\"68.0000\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
  "<path fill=\"rgb(254, 254, 254)\" stroke=\"rgb(0, 0, 0)\" stroke-width=\"3.0\" d=\"M0 67 L88 67 L88 1 L0 1 L0 67 \"/>\n"
  "<path stroke=\"rgb(0, 0, 0)\" stroke-width=\"3.0\" fill=\"none\" d=\"M36.46 24.78 L36.46 23.1 L38.13 19.75 L39.81 18.08 L43.16 16.4 L49.87 16.4 L53.22 18.08 L54.9 19.75 L56.57 23.1 L56.57 26.46 L54.9 29.81 L53.22 31.49 L46.51 34.84 L46.51 39.87 M46.51 48.25 L44.84 49.92 L46.51 51.6 L48.19 49.92 L46.51 48.25 \"/>\n"
  "</svg>";

const char* gui_dflt_conf() {
	static char buf[8192];
  char l_str[10];
	
	static const char *conf = "-- CadZinho enviroment configuration file\n"
	"-- This file is writen in Lua language\n"
	"-- NOTE1: The purpose of this script file is load global initial parameters. Don't expect script features to fully work.\n"
	"-- NOTE2: For Windows, strings with path dir separator '\\' must be escaped, eg. \"C:\\\\mydir\\\\myfile.lua\"\n\n"
	"-- Paths to look for font files. Each path is separeted by \";\" or \":\", according system default\n"
	"font_path = \"%s\"\n\n"
	"-- List of fonts to be loaded at startup (especify only file name, without path)\n"
	"fonts = {\n"
	"    \"romans.shx\",\n" 
	"    \"txt.shx\",\n"
	"    \"Cadman_Roman.ttf\",\n"
	"    \"%s\"\n"
	"}\n\n"
	"-- Font to use in user interface (must be preloaded). File and size in pts\n"
	"ui_font = {\"%s\", 11}\n\n"
	"-- Interface theme - green (default), black, white, red, blue, dark, brown or purple\n"
	"theme = \"green\"\n\n"
	"-- Background color - RGB components, integer values from 0 to 255\n"
	"background = { r=100, g=100, b=100 }\n\n"
	"-- Hilite color - RGB components, integer values from 0 to 255\n"
	"hilite = { r=255, g=0, b=255 }\n\n"
	"-- Drawing cursor type - cross (default), square, x or circle\n"
	"cursor = \"cross\"\n\n"
  "-- Main language in GUI (translation)\n"
  "language = \"%s\"\n\n";
	
  SDL_Locale *locale = SDL_GetPreferredLocales();
  
  if(locale){
    if (locale[0].language && locale[0].country)
      snprintf(l_str, 9, "%s_%s", locale[0].language, locale[0].country);
    SDL_free(locale);
  }
  
	char * fonts_path = escape_path((char *)dflt_fonts_dir ());
	
	const char* font = plat_dflt_font ();
	
	snprintf(buf, 8191, conf, fonts_path, font,
    "Cadman_Roman.ttf", l_str);
	
	return buf;
}

void gui_load_lang (gui_obj *gui){
  /* Recommendation: Language string written according codes defined in POSIX
  convention (standarts ISO 639-1 and ISO 3166-1 alpha-2), like lang_COUNTRY
  codes (eg. en_US, pt_BR, cz_CZ) */
  
  char new_path[PATH_MAX_CHARS+1];
  new_path[0] = 0;
  const char *platform = operating_system();
  
  
  snprintf(new_path, PATH_MAX_CHARS, "%s.UTF-8", gui->main_lang);
  setlocale(LC_CTYPE, new_path);
  setlocale(LC_COLLATE, new_path);
  
  /* look for language file in -program-/lang/ directory */
	if (!strcmp(platform, "unix") /* Handle Linux, Unix, posix */
	|| !strcmp(platform, "linux") 
	|| !strcmp(platform, "freeBSD")) {
		snprintf(new_path, PATH_MAX_CHARS, "%sshare/cadzinho/lang/%s.lua", 
      gui->base_dir, gui->main_lang);
	} else {
    snprintf(new_path, PATH_MAX_CHARS, "%slang%c%s.lua", 
      gui->base_dir, DIR_SEPARATOR, gui->main_lang);
  }
  
  /* init auxiliary script object */
  if (gui_script_init (gui, &gui->main_lang_scr, new_path, NULL) == 1){
    gui->main_lang_scr.active = 1;
    
    gui->main_lang_scr.time = clock();
    gui->main_lang_scr.timeout = 1.0; /* default timeout value */
    gui->main_lang_scr.do_init = 0;
    
    lua_getglobal(gui->main_lang_scr.T, "cz_main_func");
    int n_results = 0;
    gui->main_lang_scr.status = lua_resume(gui->main_lang_scr.T, NULL, 0, &n_results); /* start thread */
    if (gui->main_lang_scr.status != LUA_OK){
      /* error */
      /* close script and clean instance*/
      lua_close(gui->main_lang_scr.L);
      gui->main_lang_scr.L = NULL;
      gui->main_lang_scr.T = NULL;
      gui->main_lang_scr.active = 0;
      gui->main_lang_scr.dynamic = 0;
    }
    else{
      /* globals always in stack to prevent garbage colector */
      /* 'translate' table in stack pos = 1 */
      if (lua_getglobal(gui->main_lang_scr.T, "translate") != LUA_TTABLE){
        gui->main_lang_scr.active = 0;
        lua_pop(gui->main_lang_scr.T, 1);
      }
      else {
        /* 'descr' string in stack pos = 2 */
        if (lua_getglobal(gui->main_lang_scr.T, "descr") != LUA_TSTRING){
          lua_pop(gui->main_lang_scr.T, 1);
          lua_pushliteral(gui->main_lang_scr.T, "");
        }
        /* 'flag' SVG image in stack pos = 3 */
        lua_getglobal(gui->main_lang_scr.T, "cadzinho");
        lua_pushstring(gui->main_lang_scr.T, "svg_image");
        lua_gettable(gui->main_lang_scr.T, -2);
        if (lua_getglobal(gui->main_lang_scr.T, "flag") != LUA_TSTRING){
          /* missing flag SVG string */
          lua_pop(gui->main_lang_scr.T, 1);
          lua_pushstring(gui->main_lang_scr.T, no_flag); /* 'missing flag' image */
        }
        /* try to rasterize flag image to 24x18 pix (4:3) */
        lua_pushnumber(gui->main_lang_scr.T, 24);
        lua_pushnumber(gui->main_lang_scr.T, 18);
        gui->main_lang_scr.time = clock();
        lua_pcall(gui->main_lang_scr.T, 3, 1, 0);
        if (!lua_isuserdata(gui->main_lang_scr.T, -1)){
          /* fail in rasterization - replace to default image */
          lua_pop(gui->main_lang_scr.T, 1);
          lua_pushstring(gui->main_lang_scr.T, "svg_image");
          lua_gettable(gui->main_lang_scr.T, -2);
          lua_pushstring(gui->main_lang_scr.T, no_flag); /* 'missing flag' image */
          lua_pushnumber(gui->main_lang_scr.T, 24);
          lua_pushnumber(gui->main_lang_scr.T, 18);
          gui->main_lang_scr.time = clock();
          lua_pcall(gui->main_lang_scr.T, 3, 1, 0);
        }
        lua_replace(gui->main_lang_scr.T, 3);
      }
    }
  }
}
static void gui_change_lang (gui_obj *gui, lua_State *L){
  /* full path of config file */
	char config_path[PATH_MAX_CHARS + 1];
	config_path[0] = 0;
	snprintf(config_path, PATH_MAX_CHARS, "%sconfig.lua", gui->pref_path);
  
  /* Its will alter the config.lua file, changing the language variable */
  lua_getglobal(L, "change_lang_config");
  lua_pushstring(L, config_path); /* path to config.lua file */
  lua_pushstring(L, gui->main_lang); /* language chosen */
  if (lua_pcall(L, 2, 0, 0) == LUA_OK){ /* file changed successfully */
    gui_load_lang(gui); /* load a new language */
  }
  else{
    //char *error = lua_tostring(L, -1);
    lua_pop(L, 1); /* pop error message from the stack */
  }
}

int gui_load_conf (gui_obj *gui){
	/* initialize fonts paths with base directory and resource folder */
	snprintf(gui->dflt_fonts_path, 5 * DXF_MAX_CHARS, "%s%c%sres%cfont%c%c", gui->pref_path, PATH_SEPARATOR,
		gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR, PATH_SEPARATOR);
	
	/* load default files */
	char new_path[PATH_MAX_CHARS+1];
	
	/* seed */
	new_path[0] = 0;
	snprintf(new_path, PATH_MAX_CHARS, "%sres%cseed.dxf", gui->pref_path, DIR_SEPARATOR);
	gui->seed = try_load_dflt(new_path, (char *)dxf_seed_2007);
	/* linetypes */
	new_path[0] = 0;
	snprintf(new_path, PATH_MAX_CHARS, "%sres%clin%cdefault.lin", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
	gui->dflt_lin = try_load_dflt(new_path, (char *)ltype_lib_dflt());
	new_path[0] = 0;
	snprintf(new_path, PATH_MAX_CHARS, "%sres%clin%cextra.lin", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
	gui->extra_lin = try_load_dflt(new_path, (char *)ltype_lib_extra());
	/* hatch pattern */
	new_path[0] = 0;
	snprintf(new_path, PATH_MAX_CHARS, "%sres%cpat%cdefault.pat", gui->pref_path, DIR_SEPARATOR, DIR_SEPARATOR);
	gui->dflt_pat = try_load_dflt(new_path, (char *)h_pattern_lib_dflt());
	
	/* load hatches patterns -NEED IMPROVEMENTS */
	gui->hatch_fam.next = dxf_hatch_family(_l("Standard"), 
    _l("Internal standard pattern library"), gui->dflt_pat);
	if(gui->hatch_fam.next) gui->end_fam = gui->hatch_fam.next;
	
	/* full path of config file */
	char config_path[PATH_MAX_CHARS + 1];
	config_path[0] = 0;
	snprintf(config_path, PATH_MAX_CHARS, "%sconfig.lua", gui->pref_path);
	
	/* verify if file exists, or try to create a new one with default options */
	miss_file (config_path, (char*)gui_dflt_conf());
	
	/* init the Lua instance, to run configuration */
	struct script_obj conf_script;
	conf_script.L = NULL;
	conf_script.T = NULL;
	conf_script.active = 0;
	conf_script.dynamic = 0;
	
	if (gui_script_init (gui, &conf_script, config_path, (char*)gui_dflt_conf()) == 1){
		conf_script.time = clock();
		conf_script.timeout = 1.0; /* default timeout value */
		conf_script.do_init = 0;
		
		lua_getglobal(conf_script.T, "cz_main_func");
		int n_results = 0; /* for Lua 5.4*/
		conf_script.status = lua_resume(conf_script.T, NULL, 0, &n_results); /* start thread */
		if (conf_script.status != LUA_OK){
			conf_script.active = 0; /* error */			
		}
		/* finaly get configuration from global variables in Lua instance */
		gui_get_conf (conf_script.T);
    
    /* language */
    if(strlen(gui->main_lang) > 1){
      gui_load_lang(gui);
		}
		/* close script and clean instance*/
		lua_close(conf_script.L);
		conf_script.L = NULL;
		conf_script.T = NULL;
		conf_script.active = 0;
		conf_script.dynamic = 0;
	}
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
		else if (strcmp(theme, "dracula") == 0){
			gui->theme = THEME_DRACULA;
		}
		else if (strcmp(theme, "default") == 0){
			gui->theme = THEME_DEFAULT;
		}
	}
	lua_pop(L, 1);
	
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
				ui_font = get_font_list(gui->font_list, "Cadman_Roman.ttf");
			}
			gui->ui_font.userdata = nk_handle_ptr(ui_font);
		}
		else{ /* default value, if not definied in file*/
			struct tfont *ui_font = get_font_list(gui->font_list, "Cadman_Roman.ttf");
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
			gui->ui_font.height = 11.0;
		}
		lua_pop(L, 1);		
		
	}
	else { /* default font, if not definied in file*/
		struct tfont *ui_font = get_font_list(gui->font_list, "Cadman_Roman.ttf");
		gui->ui_font.userdata = nk_handle_ptr(ui_font);
		gui->ui_font.height = 11.0;
	}
	lua_pop(L, 1);
	
	/* -------------------- get background color -------------------*/
	lua_getglobal(L, "background");
	if (lua_istable(L, -1)){
		if (lua_getfield(L, -1, "r") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->background.r = value;
			}
		}
		lua_pop(L, 1);
		if (lua_getfield(L, -1, "g") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->background.g = value;
			}
		}
		lua_pop(L, 1);
		if (lua_getfield(L, -1, "b") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->background.b = value;
			}
		}
		lua_pop(L, 1);
	}
	else{ /* default value, if not definied in file*/
		gui->background.r = 100;
		gui->background.g = 100;
		gui->background.b = 100;
	}
	lua_pop(L, 1);
	
	/* -------------------- get hilite color -------------------*/
	lua_getglobal(L, "hilite");
	if (lua_istable(L, -1)){
		if (lua_getfield(L, -1, "r") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->hilite.r = value;
			}
		}
		lua_pop(L, 1);
		if (lua_getfield(L, -1, "g") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->hilite.g = value;
			}
		}
		lua_pop(L, 1);
		if (lua_getfield(L, -1, "b") == LUA_TNUMBER){
			int value = lua_tonumber(L, -1);
			if (value >= 0 && value < 256){
				gui->hilite.b = value;
			}
		}
		lua_pop(L, 1);
	}
	else{ /* default value, if not definied in file*/
		gui->hilite.r = 255;
		gui->hilite.g = 0;
		gui->hilite.b = 255;
	}
	lua_pop(L, 1);
	
	/* -------------------- get cursor type -------------------*/
	lua_getglobal(L, "cursor");
	if (lua_isstring(L, -1)){
		const char *cursor = lua_tostring(L, -1);
		if (strcmp(cursor, "cross") == 0){
			gui->cursor = CURSOR_CROSS;
		}
		else if (strcmp(cursor, "square") == 0){
			gui->cursor = CURSOR_SQUARE;
		}
		else if (strcmp(cursor, "x") == 0){
			gui->cursor = CURSOR_X;
		}
		else if (strcmp(cursor, "circle") == 0){
			gui->cursor = CURSOR_CIRCLE;
		}
	}
	lua_pop(L, 1);
  
  /* ------------------ get gui language --------------- */
  gui->main_lang[0] = 0;
	if (lua_getglobal(L, "language") == LUA_TSTRING){
    strncpy(gui->main_lang, lua_tostring(L, -1), DXF_MAX_CHARS);
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
	
	/* save recent files */
	fprintf(file, "recent = {\n");
	i = 0;
	list_node *rcnt_curr = gui->recent_drwg->next;
	while (rcnt_curr != NULL && i < DRWG_RECENT_MAX){ /* sweep the list */
		if (rcnt_curr->data){
			STRPOOL_U64 str_a = (STRPOOL_U64) rcnt_curr->data;  /* str key in pool */
			char *rcnt_file = (char *) strpool_cstr( &gui->file_pool, str_a); /* get string */
			fprintf(file, "    \"");
			path_escape(rcnt_file, file); /* escape especial '\' character used in Windows */
			fprintf(file, "\",\n");
		}
		rcnt_curr = rcnt_curr->next;
		i++;
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
	else gui->win_w = 1200; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- get screen height -------------------*/
	lua_getglobal(L, "win_height");
	if (lua_isnumber(L, -1)) gui->win_h = (int)lua_tonumber(L, -1);
	else gui->win_h = 685; /* default value, if not definied in file*/
	lua_pop(L, 1);
	
	/* -------------------- load list of recent drawing files  -------------------*/
	lua_getglobal(L, "recent");
	if (lua_istable(L, -1)){
		/* iterate over table */
		lua_pushnil(L);  /* first key */
		while (lua_next(L, -2) != 0) { /* table index are shifted*/
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			if (lua_isstring(L, -1)){
				STRPOOL_U64 str_a = strpool_inject( &gui->file_pool, lua_tostring(L, -1) , (int) strlen(lua_tostring(L, -1) ) );
				list_push(gui->recent_drwg, list_new((void *)str_a, PRG_LIFE));
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
		
		#if(0)
		int len = lua_rawlen(L, -1);
		int i;
		
		for (i = len; i > 0; i--){ /* iterate over table in reverse order */
			lua_rawgeti(L, -1, i); 
			if (lua_isstring(L, -1)){
				STRPOOL_U64 str_a = strpool_inject( &gui->file_pool, lua_tostring(L, -1) , (int) strlen(lua_tostring(L, -1) ) );
				list_push(gui->recent_drwg, list_new((void *)str_a, PRG_LIFE));
			}
			
			lua_pop(L, 1);
		}
		#endif
	}
	lua_pop(L, 1);
	
	return 1;
}

int config_win (gui_obj *gui){
  static struct script_obj lang_scr;
  static int init = 0;
  
	int show_config = 1;
	int i = 0;
  static int num_lang = 0;
	
	static enum Cfg_group {
			GRP_PREF,
			GRP_3D,
			GRP_INFO,
	} cfg_grp = GRP_PREF;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "config", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, _l("Config"), nk_rect(418, 88, 400, 300),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
    
    if (!init){ /* init Lua engine and static structures */
      init = 1;
      num_lang = 0;
      /* custom Lua functions to manage language config */
      const char *f =
        "function change_lang_config (fname, lang)\n"
        "  local f = io.open(fname, \"r\")\n"
        "  local content = f:read(\"a\")\n"
        "  f:close()\n"
        "  if string.find(content, '(\\n[^-\\n]*language%s*=%s*)%g+') then\n"
        "    content = string.gsub (content, '(\\n[^-\\n]*language%s*=%s*)%g+', '%1\"'.. lang ..'\"')\n"
        "  else\n"
        "    content = content .. '\\nlanguage = \"' .. lang .. '\"'\n"
        "  end\n"
        "  f = io.open(fname, \"w+\")\n"
        "  f:write(content)\n"
        "  f:close()\n"
        "end\n"
        "function list_lang (path)\n"
        "  local no_flag = [[<svg width=\"88.0000\" height=\"68.0000\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
        "    <path fill=\"rgb(254, 254, 254)\" stroke=\"rgb(0, 0, 0)\" stroke-width=\"3.0\" d=\"M0 67 L88 67 L88 1 L0 1 L0 67 \"/>\n"
        "    <path stroke=\"rgb(0, 0, 0)\" stroke-width=\"3.0\" fill=\"none\" d=\"M36.46 24.78 L36.46 23.1 L38.13 19.75 L39.81 18.08 L43.16 16.4 L49.87 16.4 L53.22 18.08 L54.9 19.75 L56.57 23.1 L56.57 26.46 L54.9 29.81 L53.22 31.49 L46.51 34.84 L46.51 39.87 M46.51 48.25 L44.84 49.92 L46.51 51.6 L48.19 49.92 L46.51 48.25 \"/>\n"
        "    </svg>]]\n"
        "  langs = {}\n"
        "  langs_o = {}\n"
        "  local dir = fs.dir(path)\n"
        "  if not dir then return langs end -- quit on error\n"
        "  for i = 1, #dir do\n"
        "    if dir[i].is_dir == false then\n"
        "      if string.find(dir[i].name, '^%l%l_%u%u%.lua$') then\n"
        "        langs[#langs+1] = string.match(dir[i].name, '^%l%l_%u%u')\n"
        "      end\n"
        "    end\n"
        "  end\n"
        "  table.sort(langs)\n"
        "  for i, lang in ipairs(langs) do\n"
        "    local img = nil\n"
        "    dofile(path .. lang .. '.lua')\n"
        "    if type (flag) ~= 'string' then\n"
        "      flag = no_flag\n"
        "    end\n"
        "    img = cadzinho.svg_image(flag, 24, 18)\n"
        "    if not img then\n"
        "      img = cadzinho.svg_image(no_flag, 24, 18)\n"
        "    end\n"
        "    langs_o[i] = {descr = descr, flag = img}\n"
        "    descr = nil\n"
        "    flag = nil\n"
        "  end\n"
        "  return langs, langs_o\n"
        "end";
      lang_scr.L = NULL;
      lang_scr.T = NULL;
      lang_scr.active = 0;
      lang_scr.dynamic = 0;
      
      /* init the auxiliary language script object  */
      if (gui_script_init (gui, &lang_scr, NULL, (char*)f) == 1){
        lang_scr.time = clock();
        lang_scr.timeout = 10.0; /* default timeout value */
        lang_scr.do_init = 0;
        
        lua_getglobal(lang_scr.T, "cz_main_func");
        int n_results = 0; /* for Lua 5.4*/
        lang_scr.status = lua_resume(lang_scr.T, NULL, 0, &n_results); /* start thread */
        if (lang_scr.status != LUA_OK){
          lang_scr.active = 0; /* error */			
        } else lang_scr.active = 1;
      }
      if (lang_scr.active){
        /* get languages available in system */
        char new_path[PATH_MAX_CHARS+1];
        const char *platform = operating_system();
        /* look for language files in -program-/lang/ directory */
        if (!strcmp(platform, "unix") /* Handle Linux, Unix, posix */
        || !strcmp(platform, "linux") 
        || !strcmp(platform, "freeBSD")) {
          snprintf(new_path, PATH_MAX_CHARS, "%sshare/cadzinho/lang/", 
            gui->base_dir);
        } else {
          snprintf(new_path, PATH_MAX_CHARS, "%slang%c", 
            gui->base_dir, DIR_SEPARATOR);
        }
        
        lua_getglobal(lang_scr.T, "list_lang"); /* get function to be called */
        lua_pushstring(lang_scr.T, new_path); /* path to lang dir */
        lang_scr.time = clock();
        if (lua_pcall(lang_scr.T, 1, 2, 0) == LUA_OK){
          /* numer of languages available */
          num_lang = lua_rawlen (lang_scr.T, 1);
        }
        else{
          const char *error = lua_tostring(lang_scr.T, -1);
          lua_pop(lang_scr.T, 1); /* pop error message from the stack */
        }
      }
    }
		/* Config groups - Preferences, Raw info, 3D view */
		nk_style_push_vec2(gui->ctx, &gui->ctx->style.window.spacing, nk_vec2(0,5));
		nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 4);
		if (gui_tab (gui, _l("Preferences"), cfg_grp == GRP_PREF)) cfg_grp = GRP_PREF;
		if (gui_tab (gui, _l("Info"), cfg_grp == GRP_INFO)) cfg_grp = GRP_INFO;
		if (gui_tab (gui, _l("3D"), cfg_grp == GRP_3D)) cfg_grp = GRP_3D;
		nk_style_pop_vec2(gui->ctx);
		nk_layout_row_end(gui->ctx);
		
		if(cfg_grp == GRP_PREF){
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, _l("Preferences folder:"), NK_TEXT_LEFT);
			nk_edit_string_zero_terminated(gui->ctx, 
				NK_EDIT_READ_ONLY,
				gui->pref_path, PATH_MAX_CHARS, nk_filter_default);
			nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.3, 0.7});
			if (nk_button_label(gui->ctx, _l("Copy"))){
				SDL_SetClipboardText(gui->pref_path);
			}
			if (nk_button_label(gui->ctx, _l("Open folder"))){
				opener(gui->pref_path);
			}
      nk_layout_row_dynamic(gui->ctx, 5, 1);
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, _l("Config File:"), NK_TEXT_LEFT);
			if (nk_button_label(gui->ctx, _l("Open file"))){
				/* full path of config file */
				char config_path[PATH_MAX_CHARS + 1];
				config_path[0] = 0;
				strncpy(config_path, gui->pref_path, PATH_MAX_CHARS);
				strncat(config_path, "config.lua", PATH_MAX_CHARS);
				opener(config_path);
			}
			if (nk_button_label(gui->ctx, _l("Reload config"))){
				gui_list_font_free (gui->ui_font_list);
				gui->ui_font_list = gui_new_font (NULL);
				
				if (gui->seed) free(gui->seed);
				if (gui->dflt_pat) free(gui->dflt_pat);
				if (gui->dflt_lin) free(gui->dflt_lin);
				if (gui->extra_lin) free(gui->extra_lin);
				
				gui->seed = NULL;
				gui->dflt_pat = NULL;
				gui->dflt_lin = NULL;
				gui->extra_lin = NULL;
				
				gui_load_conf (gui);
				set_style(gui, gui->theme);
				
				{
					double font_size = 0.8;
					for (i = 0; i < FONT_NUM_SIZE; i++){
						gui->alt_font_sizes[i] = gui->ui_font;
						gui->alt_font_sizes[i].height = font_size * gui->ui_font.height;
						font_size += 0.2;
					}
				}
			}
      
      nk_layout_row_dynamic(gui->ctx, 5, 1);
      nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_label(gui->ctx, _l("Interface language:"), NK_TEXT_LEFT);
      nk_layout_row(gui->ctx, NK_STATIC, 20, 4, (float []){80, 10, 25, 220});
      
      /* language chooser */
      int h = num_lang * 30 + 5;
      h = (h < 300)? h : 300;
      if (nk_combo_begin_label(gui->ctx, gui->main_lang, nk_vec2(300,h))) {
        for (i = 1; i <= num_lang; i++){
          nk_layout_row_dynamic(gui->ctx, 25, 1);
          /* get table with language description and country flag */
          lua_rawgeti (lang_scr.T, 2, i);
          lua_pushstring(lang_scr.T, "descr");
          int type = lua_gettable(lang_scr.T, -2);
          lua_pushstring(lang_scr.T, "flag");
          int type2 = lua_gettable(lang_scr.T, -3);
          /* get current language Id */
          lua_rawgeti (lang_scr.T, 1, i);
          if (type == LUA_TSTRING) {
            /* use description string, if available */
            if (type2 == LUA_TUSERDATA) {
              struct script_rast_image * img_obj = (struct script_rast_image *)
                lua_touserdata (lang_scr.T, -2);
              if (nk_button_image_label(gui->ctx, nk_image_ptr(img_obj->img),
                lua_tostring(lang_scr.T, -3), NK_TEXT_RIGHT))
              {
                strncpy(gui->main_lang, lua_tostring(lang_scr.T, -1), DXF_MAX_CHARS);
                gui_change_lang(gui, lang_scr.T); /* change in config.lua file too */
                nk_combo_close(gui->ctx);
              }
            }
            else{
              if (nk_button_label(gui->ctx, lua_tostring(lang_scr.T, -3))) {
                strncpy(gui->main_lang, lua_tostring(lang_scr.T, -1), DXF_MAX_CHARS);
                gui_change_lang(gui, lang_scr.T); /* change in config.lua file too */
                nk_combo_close(gui->ctx);
              }
            }
          }
          else {
            /* use language Id code, when description string not available */
            if (type2 == LUA_TUSERDATA) {
              struct script_rast_image * img_obj = (struct script_rast_image *)
                lua_touserdata (lang_scr.T, -2);
              if (nk_button_image_label(gui->ctx, nk_image_ptr(img_obj->img),
                lua_tostring(lang_scr.T, -1), NK_TEXT_RIGHT))
              {
                strncpy(gui->main_lang, lua_tostring(lang_scr.T, -1), DXF_MAX_CHARS);
                gui_change_lang(gui, lang_scr.T); /* change in config.lua file too */
                nk_combo_close(gui->ctx);
              }
            }
            else{
              if (nk_button_label(gui->ctx, lua_tostring(lang_scr.T, -1))) {
                strncpy(gui->main_lang, lua_tostring(lang_scr.T, -1), DXF_MAX_CHARS);
                gui_change_lang(gui, lang_scr.T); /* change in config.lua file too */
                nk_combo_close(gui->ctx);
              }
            }
          }
          lua_pop(lang_scr.T, 4); /* clean the Lua stack */
          
        }
        nk_combo_end(gui->ctx);
      }
      nk_label(gui->ctx, " ", NK_TEXT_LEFT); /* just a space */
      
      /* current language */
      if (gui->main_lang_scr.active) {
        /* contry flag */
        struct script_rast_image * img_obj = (struct script_rast_image *)
          lua_touserdata (gui->main_lang_scr.T, 3);
        nk_image(gui->ctx, nk_image_ptr(img_obj->img));
        /* language description */
        nk_label(gui->ctx, lua_tostring(gui->main_lang_scr.T, 2), NK_TEXT_LEFT);
      }
      
      else { /* no language available */
        nk_label(gui->ctx, " ", NK_TEXT_LEFT);
        nk_label(gui->ctx, " ", NK_TEXT_LEFT);
      }
      
		}
		else if(cfg_grp == GRP_INFO){
			nk_layout_row_dynamic(gui->ctx, 60, 1);
			
			nk_label_wrap(gui->ctx, _l("The following window is used to visualize "
				"the raw parameters of the selected elements, according to "
				"the DXF specification. It is useful for advanced users to debug "
				"current file entities"));
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			if (nk_button_label(gui->ctx, _l("Open Info Window"))){
				gui->show_info = 1;
			}
			
		}
		else if(cfg_grp == GRP_3D){
			
			nk_layout_row_dynamic(gui->ctx, 60, 1);
			
			nk_label_wrap(gui->ctx, _l("This is a experimental 3D view mode. To return to default 2D view, choose \"Top\" or set all angles to 0"));
			
			nk_layout_row_dynamic(gui->ctx, 20, 1);
			nk_property_float(gui->ctx, _l("#Alpha"), -180.0, &gui->alpha, 180.0f, 1.0f, 1.0);
			nk_property_float(gui->ctx, _l("#Beta"), -180.0, &gui->beta, 180.0f, 1.0f, 1.0);
			nk_property_float(gui->ctx, _l("#Gamma"), -180.0, &gui->gamma, 180.0f, 1.0f, 1.0);
			/*nk_slider_float(gui->ctx, -180.0, &gui->alpha, 180.0f, 0.1f);
			nk_slider_float(gui->ctx, -180.0, &gui->beta, 180.0f, 0.1f);
			nk_slider_float(gui->ctx, -180.0, &gui->gamma, 180.0f, 0.1f);*/
			
			nk_layout_row_dynamic(gui->ctx, 20, 3);
			if (nk_button_label(gui->ctx, _l("Top"))){
				gui->alpha = 0.0;
				gui->beta = 0.0;
				gui->gamma = 0.0;
			}
			if (nk_button_label(gui->ctx, _l("Front"))){
				gui->alpha = 0.0;
				gui->beta = 0.0;
				gui->gamma = 90.0;
			}
			if (nk_button_label(gui->ctx, _l("Right"))){
				gui->alpha = 90.0;
				gui->beta = 0.0;
				gui->gamma = 90.0;
			}
			if (nk_button_label(gui->ctx, _l("Bottom"))){
				gui->alpha = 0.0;
				gui->beta = 0.0;
				gui->gamma = 180.0;
			}
			if (nk_button_label(gui->ctx, _l("Rear"))){
				gui->alpha = 180.0;
				gui->beta = 0.0;
				gui->gamma = 90.0;
			}
			if (nk_button_label(gui->ctx, _l("Left"))){
				gui->alpha = -90.0;
				gui->beta = 0.0;
				gui->gamma = 90.0;
			}
			if (nk_button_label(gui->ctx, _l("Iso"))){
				gui->alpha = 45.0;
				gui->beta = 0.0;
				gui->gamma = 90.0 - 35.264;
			}
			
			double sin_alpha, cos_alpha, sin_beta, cos_beta, sin_gamma, cos_gamma;
			
			sin_alpha = sin(gui->alpha * M_PI / 180.0);
			cos_alpha = cos(gui->alpha * M_PI / 180.0);
			sin_beta = sin(gui->beta * M_PI / 180.0);
			cos_beta = cos(gui->beta * M_PI / 180.0);
			sin_gamma = sin(gui->gamma * M_PI / 180.0);
			cos_gamma = cos(gui->gamma * M_PI / 180.0);
			
			float mat_a[4][4], mat_b[4][4], res[4][4];
			
			mat_a[0][0] = 1.0;
			mat_a[0][1] = 0.0;
			mat_a[0][2] = 0.0;
			mat_a[0][3] = 0.0;
			mat_a[1][0] = 0.0;
			mat_a[1][1] = 1.0;
			mat_a[1][2] = 0.0;
			mat_a[1][3] = 0.0;
			mat_a[2][0] = 0.0;
			mat_a[2][1] = 0.0;
			mat_a[2][2] = 1.0;
			mat_a[2][3] = 0.0;
			mat_a[3][0] = -((float)gui->win_w)/2.0;
			mat_a[3][1] = -((float)gui->win_h)/2.0;
			mat_a[3][2] = 0.0;
			mat_a[3][3] = 1.0;
			
			mat_b[0][0] = cos_alpha*cos_beta;
			mat_b[0][1] = cos_alpha*sin_beta*sin_gamma - sin_alpha*cos_gamma;
			mat_b[0][2] = cos_alpha*sin_beta*cos_gamma + sin_alpha*sin_gamma;
			mat_b[0][3] = 0.0;
			mat_b[1][0] = sin_alpha*cos_beta;
			mat_b[1][1] = sin_alpha*sin_beta*sin_gamma + cos_alpha*cos_gamma;
			mat_b[1][2] = sin_alpha*sin_beta*cos_gamma - cos_alpha*sin_gamma;
			mat_b[1][3] = 0.0;
			mat_b[2][0] = -sin_beta;
			mat_b[2][1] = cos_beta*sin_gamma;
			mat_b[2][2] = cos_beta*cos_gamma;
			mat_b[2][3] = 0.0;
			mat_b[3][0] = 0.0;
			mat_b[3][1] = 0.0;
			mat_b[3][2] = 0.0;
			mat_b[3][3] = 1.0;
			
			matrix4_mul(mat_b[0], mat_a[0], res[0]);
			
			mat_b[0][0] = 1.0;
			mat_b[0][1] = 0.0;
			mat_b[0][2] = 0.0;
			mat_b[0][3] = 0.0;
			mat_b[1][0] = 0.0;
			mat_b[1][1] = 1.0;
			mat_b[1][2] = 0.0;
			mat_b[1][3] = 0.0;
			mat_b[2][0] = 0.0;
			mat_b[2][1] = 0.0;
			mat_b[2][2] = 1.0;
			mat_b[2][3] = 0.0;
			mat_b[3][0] = ((float)gui->win_w)/2.0;
			mat_b[3][1] = ((float)gui->win_h)/2.0;
			mat_b[3][2] = 0.0;
			mat_b[3][3] = 1.0;
			
			matrix4_mul(mat_b[0], res[0], gui->drwg_view[0]);
			
			invert_4matrix(gui->drwg_view[0], gui->drwg_view_i[0]);
		}
	} else {
    show_config = 0;
    if (lang_scr.L) {
			lua_close(lang_scr.L);
			lang_scr.L = NULL;
		}
    lang_scr.T = NULL;
    lang_scr.active = 0;
    lang_scr.dynamic = 0;
    init = 0;
  }
	nk_end(gui->ctx);
	
	return show_config;
}