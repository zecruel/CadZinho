#include "gui_config.h"
#include "gui_script.h"

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
	"theme = \"green\"\n\n"
	"-- Background color - RGB components, integer values from 0 to 255\n"
	"background = { r=100, g=100, b=100 }\n\n"
	"-- Hilite color - RGB components, integer values from 0 to 255\n"
	"hilite = { r=255, g=0, b=255 }\n\n"
	"-- Drawing cursor type - cross (default), square, x or circle\n"
	"cursor = \"cross\"\n\n";
	return conf;
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
	gui->hatch_fam.next = dxf_hatch_family("Standard", "Internal standard pattern library", gui->dflt_pat);
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
		if (nk_button_label(gui->ctx, "Reload config")){
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
		
		nk_property_float(gui->ctx, "#Alpha", -180.0, &gui->alpha, 180.0f, 1.0f, 1.0);
		nk_property_float(gui->ctx, "#Beta", -180.0, &gui->beta, 180.0f, 1.0f, 1.0);
		nk_property_float(gui->ctx, "#Gamma", -180.0, &gui->gamma, 180.0f, 1.0f, 1.0);
		/*nk_slider_float(gui->ctx, -180.0, &gui->alpha, 180.0f, 0.1f);
		nk_slider_float(gui->ctx, -180.0, &gui->beta, 180.0f, 0.1f);
		nk_slider_float(gui->ctx, -180.0, &gui->gamma, 180.0f, 0.1f);*/
		
		nk_layout_row_dynamic(gui->ctx, 20, 3);
		if (nk_button_label(gui->ctx, "Top")){
			gui->alpha = 0.0;
			gui->beta = 0.0;
			gui->gamma = 0.0;
		}
		if (nk_button_label(gui->ctx, "Front")){
			gui->alpha = 0.0;
			gui->beta = 0.0;
			gui->gamma = 90.0;
		}
		if (nk_button_label(gui->ctx, "Right")){
			gui->alpha = 90.0;
			gui->beta = 0.0;
			gui->gamma = 90.0;
		}
		if (nk_button_label(gui->ctx, "Bottom")){
			gui->alpha = 0.0;
			gui->beta = 0.0;
			gui->gamma = 180.0;
		}
		if (nk_button_label(gui->ctx, "Rear")){
			gui->alpha = 180.0;
			gui->beta = 0.0;
			gui->gamma = 90.0;
		}
		if (nk_button_label(gui->ctx, "Left")){
			gui->alpha = -90.0;
			gui->beta = 0.0;
			gui->gamma = 90.0;
		}
		if (nk_button_label(gui->ctx, "Iso")){
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
		
	} else show_config = 0;
	nk_end(gui->ctx);
	
	return show_config;
}