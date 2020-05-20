#include "gui_use.h"

int find(lua_State *L, char* source, char* pat){
	int start = 0, end = 0;
	
	lua_getglobal(L, "string"); /* get library */
	lua_getfield(L, 1, "find"); /* and function to be called */
	lua_pushstring(L, source);
	lua_pushstring(L, pat);
	if (lua_pcall(L, 2, 2, 0) == LUA_OK){
		if (lua_isnumber(L, -1)) {
			end = (int)lua_tonumber(L, -1);
			start = (int)lua_tonumber(L, -2);
			
		}
		lua_pop(L, 2); /* pop returned values */
	}
	
	lua_pop(L, 1); /* pop library "string" */
	
	return start;
}

int txt_ent_find(dxf_node *ent, lua_State *L, char* pat, int *start, int *end){
	/* using Lua engine, try to find match pattern in DXF entity text */
	
	dxf_node  *x = NULL;
	luaL_Buffer b;
	int i;
	
	*start = 0; *end = 0;
	lua_getglobal(L, "string"); /* get library */
	lua_getfield(L, 1, "find"); /* and function to be called */
	
	/* get DXF entity text strings */
	luaL_buffinit(L, &b); /* init the Lua buffer */
	for (i = 0; x = dxf_find_attr_i(ent, 3, i); i++){
		/* first, get the additional text (MTEXT ent) */
		luaL_addstring(&b, x->value.s_data);
	}
	for (i = 0; x = dxf_find_attr_i(ent, 1, i); i++){
		/* finally, get main text */
		luaL_addstring(&b, x->value.s_data);
	}
	luaL_pushresult(&b); /* finalize string and put on Lua stack */
	
	/* using Lua, try to find match pattern in text */
	lua_pushstring(L, pat);
	if (lua_pcall(L, 2, 2, 0) == LUA_OK){
		if (lua_isnumber(L, -1)) { /* success */
			*end = (int)lua_tonumber(L, -1);
			*start = (int)lua_tonumber(L, -2);
			
		}
		lua_pop(L, 2); /* clear Lua stack - pop returned values */
	}
	
	lua_pop(L, 1); /* clear Lua stack - pop library "string" */
	
	return *start > 0;
}

char * txt_ent_repl(dxf_node *ent, lua_State *L, char* pat, char* rpl){
	/* using Lua engine, try to find match pattern in DXF entity text */
	
	dxf_node  *x = NULL;
	luaL_Buffer b;
	int i;
	
	char * new_text = NULL;
	
	lua_getglobal(L, "string"); /* get library */
	lua_getfield(L, 1, "gsub"); /* and function to be called */
	
	/* get DXF entity text strings */
	luaL_buffinit(L, &b); /* init the Lua buffer */
	for (i = 0; x = dxf_find_attr_i(ent, 3, i); i++){
		/* first, get the additional text (MTEXT ent) */
		luaL_addstring(&b, x->value.s_data);
	}
	for (i = 0; x = dxf_find_attr_i(ent, 1, i); i++){
		/* finally, get main text */
		luaL_addstring(&b, x->value.s_data);
	}
	luaL_pushresult(&b); /* finalize string and put on Lua stack */
	
	/* using Lua, try to find match pattern in text */
	lua_pushstring(L, pat);
	lua_pushstring(L, rpl);
	if (lua_pcall(L, 3, 2, 0) == LUA_OK){
		if (lua_isnumber(L, -1)) { /* success */
			//*end = (int)lua_tonumber(L, -1);
			new_text = (char *)lua_tostring(L, -2);
			
		}
		lua_pop(L, 2); /* clear Lua stack - pop returned values */
	}
	
	lua_pop(L, 1); /* clear Lua stack - pop library "string" */
	
	return new_text;
}

dxf_node * dwg_find(dxf_drawing *drawing, lua_State *L, char* pat, double rect[4], dxf_node **next){
	/* try to find match pattern in drawing DXF entities text */
	
	dxf_node *current = NULL;
	dxf_node *found = NULL;
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)){
		/* fail */
		*next = NULL;
		return NULL;
	}
	
	/* init the serch */
	if (*next) current = *next; /* from previous element */
	else current = drawing->ents->obj.content->next; /* or from begin */
	
	*next = NULL;
	
	int start = 0, end = 0;
	
	/* sweep entities section */
	while (current != NULL){
		if (current->type == DXF_ENT){ /* look for DXF entity */
			/*verify if entity layer is on and thaw */
			if ((!drawing->layers[current->obj.layer].off) && 
				(!drawing->layers[current->obj.layer].frozen) &&
				/* and if  is a compatible entity */
				( (strcmp(current->obj.name, "TEXT") == 0)  ||
				(strcmp(current->obj.name, "MTEXT") == 0) )
			){
				if (found) { /* prepare for next element */
					*next = current;
					return found;
				}
				/* try to find text pattern */
				if (txt_ent_find(current, L, pat, &start, &end)){
					/* success */
					found = current;
					
					/* get aproximate graphic location of match, 
					by start-end char indexes and counting graph glyphs */
					list_node *curr_lst = NULL;
					graph_obj *curr_graph = NULL;
					curr_lst = ((list_node *)current->obj.graphics)->next;

					double min_x, min_y, max_x, max_y, w, h;
					int ini_pos = 0, i = 1;

					/* sweep glyphs list */
					while (curr_lst != NULL){
						if (curr_lst->data){
							curr_graph = (graph_obj *)curr_lst->data;
							/* check if count is in range */
							if ( (i >= start && i <= end) &&
								(curr_graph->list->next != NULL)){ /*and if graph is not empty */
								/* get each glyph corners and update the rectangle values */
								if (ini_pos == 0){
									ini_pos = 1;
									min_x = curr_graph->ext_min_x;
									min_y = curr_graph->ext_min_y;
									max_x = curr_graph->ext_max_x;
									max_y = curr_graph->ext_max_y;
								}
								else{
									min_x = (min_x < curr_graph->ext_min_x) ? min_x : curr_graph->ext_min_x;
									min_y = (min_y < curr_graph->ext_min_y) ? min_y : curr_graph->ext_min_y;
									max_x = (max_x > curr_graph->ext_max_x) ? max_x : curr_graph->ext_max_x;
									max_y = (max_y > curr_graph->ext_max_y) ? max_y : curr_graph->ext_max_y;
								}
							}
						}
						curr_lst = curr_lst->next;
						i++;
					}
					
					w = max_x - min_x;
					h = max_y - min_y;
					
					if (fabs(w) < 1e-9) w = h;
					if (fabs(h) < 1e-9) h = w;
					
					/* update return value */
					rect[0] = min_x - 0.1*w;
					rect[1] = min_y - 0.1*h;
					rect[2] = max_x + 0.1*w;
					rect[3] = max_y + 0.1*h;
				}
			}
		}
		current = current->next;
	}
	
	return found;
}

int gui_find_interactive(gui_obj *gui){
	if (gui->modal != FIND) return 0;
	
	if (gui->ev & EV_CANCEL){
		gui->step = 0;
		gui_default_modal(gui);
	}
	
	if (gui->step > 0){
		/* draw the rectangle */
		gui->draw_phanton = 0;
		gui->phanton = list_new(NULL, FRAME_LIFE);
		graph_obj *graph = graph_new(FRAME_LIFE);
		if (graph){
			gui->draw_phanton = 1;
			
			line_add(graph, gui->step_x[0], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[0], 0);
			line_add(graph, gui->step_x[1], gui->step_y[0], 0,
				gui->step_x[1], gui->step_y[1], 0);
			line_add(graph, gui->step_x[1], gui->step_y[1], 0,
				gui->step_x[0], gui->step_y[1], 0);
			line_add(graph, gui->step_x[0], gui->step_y[1], 0,
				gui->step_x[0], gui->step_y[0], 0);
			list_node * new_node = list_new(graph, FRAME_LIFE);
			list_push(gui->phanton, new_node);
		}
	}else gui->draw_phanton = 0;
	
	return 1;
}

int gui_find_info (gui_obj *gui){
	static lua_State *L = NULL;
	static dxf_node *found = NULL;
	static dxf_node *next = NULL;
	
	if (gui->modal != FIND) {
		if (L) {
			lua_close(L);
			L = NULL;
			next = NULL;
			found = NULL;
		}
		return 0;
	}
	
	if (!L){
		L = luaL_newstate(); /* opens Lua */
		luaL_openlibs(L); /* opens the standard libraries */
	}
	
	static char search[DXF_MAX_CHARS+1] = "";
	static char repl[DXF_MAX_CHARS+1] = "";
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Find/Replace text", NK_TEXT_LEFT);
	
	nk_label(gui->ctx, "Search:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, search, DXF_MAX_CHARS, nk_filter_default);
	
	nk_label(gui->ctx, "Replace:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, repl, DXF_MAX_CHARS, nk_filter_default);
	
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	if (nk_button_label(gui->ctx, "Next")){
		//find(L, search, repl); /* ---------------test -------- */
		double rect[4];
		
		if(found = dwg_find(gui->drawing, L, search, rect, &next)){
			gui->step_x[0] = rect[0];
			gui->step_y[0] = rect[1];
			gui->step_x[1] = rect[2];
			gui->step_y[1] = rect[3];
			
			gui->step = 2;
		}
		else gui->step = 0;
	}
	if (nk_button_label(gui->ctx, "Replace")){
		
		dxf_node *new_ent = dxf_ent_copy(found, DWG_LIFE); /* copy original entity */
		if (new_ent){
			/* get edited text */
			char *blank = "";
			char *text = txt_ent_repl(new_ent, L, search, repl);
			if (text){
				int len = strlen(text);
				if (!text) text = blank;
				int i;
				dxf_node *x;
				
				/* replace the text */
				if (strcmp(new_ent->obj.name, "MTEXT") == 0) 
					mtext_change_text (new_ent, text, len, DWG_LIFE);
				else if (strcmp(new_ent->obj.name, "TEXT") == 0) {
					for (i = 0; x = dxf_find_attr_i(new_ent, 1, i); i++){
						/* limit of TEXT entity length */
						len = (len < DXF_MAX_CHARS - 1)? len : DXF_MAX_CHARS - 1;
						strncpy(x->value.s_data, text, len);
						x->value.s_data[len] = 0; /* terminate string */
					}
				}
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
				dxf_obj_subst(found, new_ent);
				
				/* update undo/redo list */
				do_add_entry(&gui->list_do, "REPLACE");
				do_add_item(gui->list_do.current, found, new_ent);
			}
		}
		
	}
	if (nk_button_label(gui->ctx, "Selection")){
		
	}
	
	return 1;
}