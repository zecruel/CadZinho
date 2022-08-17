#include "gui_use.h"
#include "gui_script.h"

int gui_add_tools_win (gui_obj *gui){
	/* Additional tools GUI window */
	int show_add_tools = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 200;
	gui->next_win_h = 300;
	
	if (nk_begin(gui->ctx, "Additional tools", nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		if ( gui->add_tools_script.active ){
			lua_State *L = gui->add_tools_script.T;
			
			/* try to get definition table */
			lua_getglobal(L, "add_tools");
			if (lua_istable(L, -1)){
				char script_file [DXF_MAX_CHARS + 1] = "";
				
				/* iterate over parent table */
				int n = lua_rawlen (L, -1);
				int i;
				for (i = 1; i <= n; i++){
					if (lua_geti(L, -1, i) == LUA_TTABLE){  /* each element is a child table */
						/* script file */
						if (lua_geti(L, -1, 1) == LUA_TSTRING){
							strncpy(script_file, lua_tostring(L, -1), DXF_MAX_CHARS);
						}
						else continue; /* no file */
						if (strlen(script_file) < 5) continue; /* invalid name */
						
						lua_pop(L, 1);
						/* caption */
						if (lua_getfield(L, -1, "caption") == LUA_TSTRING){
							if (nk_button_label(gui->ctx, lua_tostring(L, -1))){
								gui_script_exec_file_slot (gui, script_file);
							}
						}
						lua_pop(L, 1);
					}
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);
		}
		
		} else show_add_tools = 0;
	nk_end(gui->ctx);
	
	return show_add_tools;
}