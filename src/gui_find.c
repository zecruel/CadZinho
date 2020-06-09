#include "gui_use.h"

struct find_el {
	dxf_node  *ent;
	int str_idx;
	int attr_idx;
};

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

int txt_ent_find_i(dxf_node *ent, lua_State *L, char* pat, int *start, int *end, int *next){
	/* using Lua engine, try to find match pattern in DXF entity text */
	
	dxf_node  *x = NULL;
	luaL_Buffer b;
	int i, n = 0;
	
	*start = 0; *end = 0;
	lua_getglobal(L, "find_i"); /* get function to be called */
	
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
	
	lua_pushnumber(L, *next);
	
	if (lua_pcall(L, 3, 3, 0) == LUA_OK){
		 /* success */
		*end = (int)lua_tonumber(L, -1);
		*start = (int)lua_tonumber(L, -2);
		n = (int)lua_tonumber(L, -3);
		
		lua_pop(L, 3); /* clear Lua stack - pop returned values */
	}
	
	if (n > *next){
		*next = *next + 1;
		return 1;
	}
	else if (n == *next){
		*next = 1;
		return 1;
	}
	
	*next = 1;
	return 0;
}

char * txt_ent_repl(dxf_node *ent, lua_State *L, char* pat, char* rpl){
	/* using Lua engine, try to find match and replace pattern in DXF entity text */
	
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
	
	/* using Lua, try to find match and replace pattern in text */
	lua_pushstring(L, pat);
	lua_pushstring(L, rpl);
	if (lua_pcall(L, 3, 2, 0) == LUA_OK){
		if (lua_isnumber(L, -1)) { /* success */
			int n = (int)lua_tonumber(L, -1); /* number of matches */
			if (n > 0) new_text = (char *)lua_tostring(L, -2);
			
		}
		lua_pop(L, 2); /* clear Lua stack - pop returned values */
	}
	
	lua_pop(L, 1); /* clear Lua stack - pop library "string" */
	
	return new_text;
}

int txt_ent_find_rect(dxf_node *ent, lua_State *L, char* pat, double rect[4], int *next){
	int start = 0, end = 0;
	
	
	/* try to find text pattern */
	if (!txt_ent_find_i(ent, L, pat, &start, &end, next)) return 0;
	/* success */
	
	/* get aproximate graphic location of match, 
	by start-end char indexes and counting graph glyphs */
	list_node *curr_lst = NULL;
	graph_obj *curr_graph = NULL;
	curr_lst = ((list_node *)ent->obj.graphics)->next;

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
	
	return ini_pos;

}

int dwg_find (dxf_drawing *drawing, lua_State *L, char* pat, double rect[4], enum dxf_graph filter, struct find_el *found, struct find_el *next){
	dxf_node *current = NULL, *first;
	found->ent = NULL;
	int str_idx;
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)){
		/* fail */
		next->ent = NULL;
		next->attr_idx = 0;
		next->str_idx = 1;
		
		found->ent = NULL;
		found->attr_idx = 0;
		found->str_idx = 1;
		
		return 0;
	}
	
	first = next->ent; 
	
	/* init the search */
	if (next->ent) current = next->ent; /* from previous element */
	else current = drawing->ents->obj.content->next; /* or from begin */
	
	next->ent = NULL;
	found->ent = NULL;
	
	/* sweep entities section */
	while (current != NULL){
		if (current != first){
			next->attr_idx = 0;
			next->str_idx = 1;
		}
		
		if (current->type == DXF_ENT){ /* look for DXF entity */
			enum dxf_graph ent_type = dxf_ident_ent_type (current);
			
			/*verify if entity layer is on and thaw */
			if ((!drawing->layers[current->obj.layer].off) && 
				(!drawing->layers[current->obj.layer].frozen) ){
				/* and if  is a compatible entity */
				if (ent_type & filter){
					if (found->ent) { /* prepare for next element */
						next->ent = current;
						return 1;
					}
					str_idx = next->str_idx;
					if (txt_ent_find_rect(current, L, pat, rect, &str_idx)){
						found->ent = current;
						found->attr_idx = 0;
						found->str_idx = next->str_idx;
						next->str_idx = str_idx;
						if (str_idx > 1) {
							next->ent = current;
							return 1;
						}
					}
					
				}
				else if (ent_type == DXF_INSERT && (filter & DXF_ATTRIB) ){
					int num_attr = 0;
					dxf_node *attr = NULL, *nxt_attr = NULL;
					
					num_attr = 0;
					while (attr = dxf_find_obj_nxt(current, &nxt_attr, "ATTRIB")){
						num_attr++;
						
						
						if (found->ent) { /* prepare for next element */
							next->ent = current;
							return 1;
						}
						if (num_attr > next->attr_idx){
							str_idx = next->str_idx;
							if (txt_ent_find_rect(attr, L, pat, rect, &str_idx)){
								found->ent = current;
								found->attr_idx = num_attr - 1;
								found->str_idx = next->str_idx;
								next->str_idx = str_idx;
								next->attr_idx = num_attr;
								if (str_idx > 1) {
									next->ent = current;
									next->attr_idx = num_attr - 1;
									return 1;
								}
							}
						}
						
						
						if (!nxt_attr) break; 
					}
				}
				
			}
		}
		current = current->next;
	}
	
	next->attr_idx = 0;
	next->str_idx = 1;
	
	return (found->ent) != NULL;
}

int ent_find (dxf_node *ent, lua_State *L, char* pat, enum dxf_graph filter){
	
	/* verify structures */
	if (!ent) return 0;
	if (ent->type == DXF_ENT) return 0;
	enum dxf_graph ent_type = dxf_ident_ent_type (ent);
	int start = 0, end = 0, next = 1;
	
	/* and if  is a compatible entity */
	if (ent_type & filter){
		next = 1;
		/* try to find text pattern */
		return txt_ent_find_i(ent, L, pat, &start, &end, &next);
		
	}
	else if (ent_type == DXF_INSERT && (filter & DXF_ATTRIB) ){
		dxf_node *attr = NULL, *nxt_attr = NULL;
		int num_attr = 0;
		
		while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
			num_attr++;
			next = 1;
			/* try to find text pattern */
			if (txt_ent_find_i(attr, L, pat, &start, &end, &next)) return num_attr;
			
			if (!nxt_attr) break;
		}
	}
	return 0;
}

int ent_replace (dxf_node * ent, lua_State *L, char * search, char * repl, int str_idx, int attr_idx){
	
	/* verify structures */
	if (!ent) return 0;
	if (!L) return 0;
	if (ent->type != DXF_ENT) return 0;
	
	if ( (strcmp(ent->obj.name, "TEXT") == 0)  ||
		(strcmp(ent->obj.name, "MTEXT") == 0) )
	{
		/* get edited text */
		char *text = txt_ent_repl(ent, L, search, repl);
		if (text){
			
			/* replace the text */
			if (strcmp(ent->obj.name, "MTEXT") == 0) 
				mtext_change_text (ent, text, strlen(text), DWG_LIFE);
			else if (strcmp(ent->obj.name, "TEXT") == 0) {
				dxf_attr_change(ent, 1, text);
			}
			
			return 1;
		}
	}
	else if (strcmp(ent->obj.name, "INSERT") == 0) {
		int num_attr = 0;
		dxf_node *attr = NULL, *nxt_attr = NULL;
		
		num_attr = 0;
		while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
			if (attr_idx == num_attr){
				char *text = txt_ent_repl(attr, L, search, repl);
				if (text) dxf_attr_change(attr, 1, text);
				return 1;
			}
			
			num_attr++;
			if (!nxt_attr) break; 
		}
	}
	
	
	return 0;
}

list_node * list_find(list_node *list, lua_State *L, char* pat, double rect[4], list_node **next, enum dxf_graph filter){
	/* try to find match pattern in drawing DXF entities text */
	
	list_node *current = NULL;
	list_node *found = NULL;
	
	/* verify structures */
	if (!list){
		/* fail */
		*next = NULL;
		return NULL;
	}
	
	/* init the serch */
	if (*next) current = *next; /* from previous element */
	else current = list->next; /* or from begin */
	
	*next = NULL;
	
	int start = 0, end = 0;
	
	/* sweep list of entities */
	while (current != NULL){
		if (current->data){
			enum dxf_graph ent_type = dxf_ident_ent_type ((dxf_node *)current->data);
			if (ent_type & filter) { /* check if  is a compatible entity */
				if (found) { /* prepare for next element */
					*next = current;
					return found;
				}
				/* try to find text pattern */
				if (txt_ent_find((dxf_node *)current->data, L, pat, &start, &end)){
					/* success */
					found = current;
					
					/* get aproximate graphic location of match, 
					by start-end char indexes and counting graph glyphs */
					list_node *curr_lst = NULL;
					graph_obj *curr_graph = NULL;
					curr_lst = ((list_node *)((dxf_node *)current->data)->obj.graphics)->next;

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

list_node *  gui_dwg_sel_filter(dxf_drawing *drawing, enum dxf_graph filter, int pool_idx){
	dxf_node *current = NULL;
	list_node *list = NULL;
	
	/* verify structures */
	if (!drawing || (drawing->ents == NULL) || (drawing->main_struct == NULL)){
		/* fail */
		return NULL;
	}
	
	/* init the serch */
	current = drawing->ents->obj.content->next; /* or from begin */
	
	/* sweep entities section */
	while (current != NULL){
		if (current->type == DXF_ENT){ /* look for DXF entity */
			enum dxf_graph ent_type = dxf_ident_ent_type (current);
			
			/*verify if entity layer is on and thaw */
			if ((!drawing->layers[current->obj.layer].off) && 
				(!drawing->layers[current->obj.layer].frozen) ){
				/* and if  is a compatible entity */
				if (ent_type & filter){
					if (!list) list = list_new(NULL, pool_idx);
					/* append to list*/
					list_node * new_node = list_new(current, pool_idx);
					list_push(list, new_node);
				}
				else if (ent_type == DXF_INSERT && (filter & DXF_ATTRIB) ){
					int num_attr = 0;
					dxf_node *attr = NULL;
					
					num_attr = 0;
					while (attr = dxf_find_obj_i(current, "ATTRIB", num_attr)){
						/* add attribute entity to list */
						if (!list) list = list_new(NULL, pool_idx);
						/* append to list*/
						list_node * new_node = list_new(attr, pool_idx);
						list_push(list, new_node);
						
						num_attr++;
					}
				}
				
			}
		}
		current = current->next;
	}
	
	return list;
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
	static struct find_el found, next;
	
	
	if (gui->modal != FIND) {
		if (L) {
			lua_close(L);
			L = NULL;
		}
		next.ent = NULL;
		next.attr_idx = 0;
		next.str_idx = 1;
		
		found.ent = NULL;
		found.attr_idx = 0;
		found.str_idx = 1;
		return 0;
	}
	
	if (!L){
		L = luaL_newstate(); /* opens Lua */
		luaL_openlibs(L); /* opens the standard libraries */
		
		const char *f =
		"i = 0\n"
		"count = 0\n"
		"sub_pat = \"\"\n"
		"\n"
		"function count_match (str)\n"
		"	count = count + 1\n"
		"	if count == i then return sub_pat\n"
		"	else return str\n"
		"	end\n"
		"end\n"
		"\n"
		"function sub_i(str, pat, sub, idx)\n"
		"	count = 0\n"
		"	i = idx\n"
		"	sub_pat = sub\n"
		"	ret_str, _ = string.gsub(str, pat, count_match)\n"
		"	return ret_str\n"
		"end\n"
		"\n"
		"function find_i(str, pat, idx)\n"
		"	n = 0\n"
		"	start = 0\n"
		"	end_str = 1\n"
		"	\n"
		"	ret_st = 0\n"
		"	ret_end = 0\n"
		"	\n"
		"	repeat\n"
		"		start, end_str = string.find(str, pat, end_str)\n"
		"		if start then\n"
		"			n = n + 1\n"
		"			if n == idx then\n"
		"				ret_st = start\n"
		"				ret_end = end_str\n"
		"			end\n"
		"		end\n"
		"	until start == nil\n"
		"	return n, ret_st, ret_end\n"
		"end\n";
		luaL_dostring(L, f);
		
		next.ent = NULL;
		next.attr_idx = 0;
		next.str_idx = 1;
		
		found.ent = NULL;
		found.attr_idx = 0;
		found.str_idx = 1;
	}
	
	static char search[DXF_MAX_CHARS+1] = "";
	static char repl[DXF_MAX_CHARS+1] = "";
	static char log[64] = "";
	
	double rect[4]; /* to draw a rectangle on found text */
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, "Find/Replace text", NK_TEXT_LEFT);
	
	nk_label(gui->ctx, "Search:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, search, DXF_MAX_CHARS, nk_filter_default);

	if (nk_button_label(gui->ctx, "Find Next") && strlen(search) > 0 ){
		log[0] = 0;
		
		/* try to find next match */
		if(dwg_find(gui->drawing, L, search, rect, DXF_TEXT | DXF_MTEXT | DXF_ATTRIB, &found, &next)){
			/* success - draw rectangle on found text pattern */
			gui->step_x[0] = rect[0];
			gui->step_y[0] = rect[1];
			gui->step_x[1] = rect[2];
			gui->step_y[1] = rect[3];
			
			/*verify if txt is visible and adjust view, if its necessary*/
			if (!gui_scr_visible(gui, rect[0], rect[1])) 
				gui_scr_centralize(gui, rect[0], rect[1]);
			
			gui->step = 2; /* to draw  rectangle in dyamic mode*/
			
			if (!next.ent){
				snprintf(log, 63, "End of search");
			}
		}
		else {
			gui->step = 0;
			snprintf(log, 63, "No elements matched");
		}
	}
	
	nk_label(gui->ctx, "Replace:", NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, repl, DXF_MAX_CHARS, nk_filter_default);
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	if (nk_button_label(gui->ctx, "Current") && strlen(search) > 0 ){
		log[0] = 0;
		dxf_node *new_ent = NULL;
		if (found.ent) new_ent = dxf_ent_copy(found.ent, DWG_LIFE); /* copy original entity */
		if (new_ent){
			
			if(ent_replace (new_ent, L, search, repl, found.str_idx, found.attr_idx)){
				
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
				dxf_obj_subst(found.ent, new_ent);
				
				/* update undo/redo list */
				do_add_entry(&gui->list_do, "REPLACE");
				do_add_item(gui->list_do.current, found.ent, new_ent);
				
				if (next.ent == found.ent) next.ent = new_ent;
				found.ent = new_ent;

			}
		}
		/* try to find next match */
		if(dwg_find(gui->drawing, L, search, rect, DXF_TEXT | DXF_MTEXT | DXF_ATTRIB, &found, &next)){
			/* success - draw rectangle on found text pattern */
			gui->step_x[0] = rect[0];
			gui->step_y[0] = rect[1];
			gui->step_x[1] = rect[2];
			gui->step_y[1] = rect[3];
			
			/*verify if txt is visible and adjust view, if its necessary*/
			if (!gui_scr_visible(gui, rect[0], rect[1])) 
				gui_scr_centralize(gui, rect[0], rect[1]);
			
			gui->step = 2; /* to draw  rectangle in dyamic mode*/
			
			if (!next.ent){
				snprintf(log, 63, "End of search");
			}
		}
		else {
			gui->step = 0;
			snprintf(log, 63, "No elements matched");
		}
		
	}
	
	if (nk_button_label(gui->ctx, "Selection")){
		log[0] = 0;
		int ini_do = 0, n = 0;
		
		list_node *nxt = NULL, *fnd;
		while(fnd = list_find(gui->sel_list, L, search, rect, &nxt, DXF_TEXT | DXF_MTEXT)){
			dxf_node *new_ent = dxf_ent_copy((dxf_node *)fnd->data, DWG_LIFE); /* copy original entity */
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
					dxf_obj_subst((dxf_node *)fnd->data, new_ent);
					
					/* update undo/redo list */
					if (!ini_do){
						do_add_entry(&gui->list_do, "REPLACE");
						ini_do = 1;
					}
					do_add_item(gui->list_do.current, (dxf_node *)fnd->data, new_ent);
					fnd->data = new_ent; /* replace in list too */
					n++;
				}
				snprintf(log, 63, "Total replaced: %d", n);
			}
			if(!nxt) break;
		}
	}
	if (nk_button_label(gui->ctx, "All")){
		log[0] = 0;
		int ini_do = 0, n = 0;
		
		list_node *nxt = NULL, *fnd;
		list_node * list = gui_dwg_sel_filter(gui->drawing, DXF_TEXT | DXF_MTEXT, FRAME_LIFE);
		while(fnd = list_find(list, L, search, rect, &nxt, DXF_TEXT | DXF_MTEXT)){
			dxf_node *new_ent = dxf_ent_copy((dxf_node *)fnd->data, DWG_LIFE); /* copy original entity */
			if (new_ent){
				/* get edited text */
				char *text = txt_ent_repl(new_ent, L, search, repl);
				if (text){					
					/* replace the text */
					if (strcmp(new_ent->obj.name, "MTEXT") == 0) 
						mtext_change_text (new_ent, text, strlen(text), DWG_LIFE);
					else if (strcmp(new_ent->obj.name, "TEXT") == 0) {
						dxf_attr_change(new_ent, 1, text);
					}
					new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
					dxf_obj_subst((dxf_node *)fnd->data, new_ent);
					
					/* update undo/redo list */
					if (!ini_do){
						do_add_entry(&gui->list_do, "REPLACE");
						ini_do = 1;
					}
					do_add_item(gui->list_do.current, (dxf_node *)fnd->data, new_ent);
					fnd->data = new_ent; /* replace in list too */
					n++;
				}
				snprintf(log, 63, "Total replaced: %d", n);
			}
			if(!nxt) break;
		}
	}
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, log, NK_TEXT_LEFT);
	
	return 1;
}