#include "gui_use.h"

struct find_el {
	dxf_node  *ent;
	int str_idx;
	int attr_idx;
};

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
	lua_pushstring(L, pat); /* pattern to find */
	lua_pushnumber(L, *next); /* match index to get position */
	
	if (lua_pcall(L, 3, 3, 0) == LUA_OK){
		 /* success */
		/* text position of match (relative to index) */
		*end = (int)lua_tonumber(L, -1);
		*start = (int)lua_tonumber(L, -2);
		n = (int)lua_tonumber(L, -3); /* total number of matches */
		
		lua_pop(L, 3); /* clear Lua stack - pop returned values */
	}
	/* update index to next search */
	if (n > *next){
		/* if has more matches */
		*next = *next + 1; /* go to next index */
		return 1;
	}
	else if (n == *next){
		/* if no next match */
		*next = 1; /* go to first match */
		return 1;
	}
	
	/* fail to look pattern */
	*next = 1;
	return 0;
}

char * txt_ent_repl(dxf_node *ent, lua_State *L, char* pat, char* rpl){
	/* using Lua engine, try to find match and replace pattern in DXF entity text.
	This function will replace all matches in entity's text. It permits full functionality
	of Lua gsub, like numbered group capture replace.
	*/
	
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
	lua_pushstring(L, pat); /* pattern to find */
	lua_pushstring(L, rpl); /* text or pattern to replace */
	if (lua_pcall(L, 3, 2, 0) == LUA_OK){
		if (lua_isnumber(L, -1)) { /* success */
			int n = (int)lua_tonumber(L, -1); /* number of matches */
			if (n > 0) new_text = (char *)lua_tostring(L, -2);
			
		}
		lua_pop(L, 2); /* clear Lua stack - pop returned values */
	}
	
	lua_pop(L, 1); /* clear Lua stack - pop library "string" */
	
	return new_text; /* return the new string */
}

char * txt_ent_repl_i(dxf_node *ent, lua_State *L, char* pat, char* rpl, int idx){
	/* using Lua engine, try to find match and replace pattern in DXF entity text.
	This function will replace only one match, pointed by index. It have limitations
	in Lua gsub.
	*/
	
	dxf_node  *x = NULL;
	luaL_Buffer b;
	int i;
	
	char * new_text = NULL;
	
	lua_getglobal(L, "sub_i"); /* get function to be called */
	
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
	lua_pushstring(L, pat);/* pattern to find */
	lua_pushstring(L, rpl); /* text to replace */
	lua_pushnumber(L, idx); /* index of match to replace */
	if (lua_pcall(L, 4, 1, 0) == LUA_OK){
		/* success */
		new_text = (char *)lua_tostring(L, -1);
		
		lua_pop(L, 1); /* clear Lua stack - pop returned values */
	}
	
	return new_text; /* return the new string */
}

int txt_ent_find_rect(dxf_node *ent, lua_State *L, char* pat, double rect[4], int *next){
	/* try to find match pattern in DXF entity text and return rectangle coordinates with
	aproximate graphic location */
	
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
	
	/* fix weigth and height to non zero values */
	w = max_x - min_x;
	h = max_y - min_y;
	if (fabs(w) < 1e-9) w = h;
	if (fabs(h) < 1e-9) h = w;
	
	/* update return values */
	rect[0] = min_x - 0.1*w;
	rect[1] = min_y - 0.1*h;
	rect[2] = max_x + 0.1*w;
	rect[3] = max_y + 0.1*h;
	
	return ini_pos;

}

int dwg_find (dxf_drawing *drawing, lua_State *L, char* pat, double rect[4], enum dxf_graph filter, struct find_el *found, struct find_el *next){
	/* try to find match pattern in DXF drawing entities.
	Target elements are choosen by filter.
	It return:
	- rectangle coordinates with aproximate graphic location
	- found element, with entity, string match index and attribute index (in case of insert entity)
	- next element, subsequent to found, to continue the search
	*/
	dxf_node *current = NULL, *first, *tmp = NULL;
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
			
			/*verify if entity layer is on and thaw */
			if ((!drawing->layers[current->obj.layer].off) && 
				(!drawing->layers[current->obj.layer].frozen) ){
				/* and if  is a compatible entity */
				if ( ( (strcmp(current->obj.name, "TEXT") == 0) && (filter & DXF_TEXT) ) ||
					( (strcmp(current->obj.name, "MTEXT") == 0) && (filter & DXF_MTEXT) ) )
				{
					/* TEXT and MTEXT DXF entities */
					if (found->ent) { /* prepare for next element */
						next->ent = current;
						return 1;
					}
					str_idx = next->str_idx; /* get next match index in string */
					/* try to find text pattern */
					if (txt_ent_find_rect(current, L, pat, rect, &str_idx)){
						/* success */
						found->ent = current;
						found->attr_idx = 0;
						found->str_idx = next->str_idx;
						next->str_idx = str_idx;
						if (str_idx > 1) {
							/* if string has more than one match, continue in entity */
							next->ent = current;
							return 1;
						}
					}
					
				}
				else if ( (strcmp(current->obj.name, "INSERT") == 0) && (filter & DXF_ATTRIB) ){
					/* ATTRIB DXF entities inside INSERTs*/
					int num_attr = 0;
					dxf_node *attr = NULL, *nxt_attr = NULL;
					
					/* sweep INSERT looking for ATTRIBs */
					num_attr = 0;
					while (attr = dxf_find_obj_nxt(current, &nxt_attr, "ATTRIB")){
						num_attr++; /* current index */
						
						if (found->ent) { /* prepare for next element */
							next->ent = current;
							return 1;
						}
						
						/* verify if it is a hidden attribute */
						if(tmp = dxf_find_attr2(attr, 70)){
							if(tmp->value.i_data & 1){
								if (nxt_attr) continue;
								else break;
							}
						}
						
						if (num_attr > next->attr_idx){ /* check if  index is in range */
							str_idx = next->str_idx; /* get next match index in string */
							/* try to find text pattern */
							if (txt_ent_find_rect(attr, L, pat, rect, &str_idx)){
								/* success */
								found->ent = current;
								found->attr_idx = num_attr - 1;
								found->str_idx = next->str_idx;
								next->str_idx = str_idx;
								next->attr_idx = num_attr;
								if (str_idx > 1) {
									/* if string has more than one match, continue in entity */
									next->ent = current;
									next->attr_idx = num_attr - 1;
									return 1;
								}
							}
						}
						
						if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
					}
				}
				
			}
		}
		current = current->next; /* get next entity in drawing*/
	}
	/* end of entities in drawing */
	
	next->attr_idx = 0;
	next->str_idx = 1;
	
	return (found->ent) != NULL;
}

int ent_find (dxf_node *ent, lua_State *L, char* pat, enum dxf_graph filter){
	/* try to find match pattern in DXF entity, simple verification
	Target elements are choosen by filter.
	*/
	/* verify structures */
	if (!ent) return 0;
	if (ent->type != DXF_ENT) return 0;
	int start = 0, end = 0, next = 1, ok = 0;
	dxf_node *tmp = NULL;
	
	/* and if  is a compatible entity */
	if ( ( (strcmp(ent->obj.name, "TEXT") == 0) && (filter & DXF_TEXT) ) ||
		( (strcmp(ent->obj.name, "MTEXT") == 0) && (filter & DXF_MTEXT) ) )
	{
		/* TEXT and MTEXT DXF entities */
		next = 1;
		/* try to find text pattern */
		ok =  txt_ent_find_i(ent, L, pat, &start, &end, &next);
		
	}
	else if ( (strcmp(ent->obj.name, "INSERT") == 0) && (filter & DXF_ATTRIB) ){
		/* ATTRIB DXF entities inside INSERTs*/
		dxf_node *attr = NULL, *nxt_attr = NULL;
		int num_attr = 0;
		
		/* sweep INSERT looking for ATTRIBs */
		while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
			num_attr++;
			next = 1;
			
			/* verify if it is a hidden attribute */
			if(tmp = dxf_find_attr2(attr, 70)){
				if(tmp->value.i_data & 1){
					if (nxt_attr) continue;
					else break;
				}
			}
			/* try to find text pattern */
			if (txt_ent_find_i(attr, L, pat, &start, &end, &next)) ok = 1;
			
			if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
		}
	}
	return ok;
}

int ent_replace (dxf_node * ent, lua_State *L, char * search, char * repl, int str_idx, int attr_idx, int entire_el){
	/* try to find and replace match pattern in DXF entity, according 
	match index in string (if not entire entity) and attrib index (in inserts).
	It not perform entity filtering */
	
	/* verify structures */
	if (!ent) return 0;
	if (!L) return 0;
	if (ent->type != DXF_ENT) return 0;
	
	if ( (strcmp(ent->obj.name, "TEXT") == 0)  ||
		(strcmp(ent->obj.name, "MTEXT") == 0) )
	{
		/* TEXT and MTEXT DXF entities */
		/* perform replace in string, according entire flag */
		char *text = NULL;
		if (entire_el) text = txt_ent_repl(ent, L, search, repl);
		else text = txt_ent_repl_i(ent, L, search, repl, str_idx);
		if (text){
			/* replace the text  in entity */
			if (strcmp(ent->obj.name, "MTEXT") == 0) 
				mtext_change_text (ent, text, strlen(text), ent->obj.pool);
			else if (strcmp(ent->obj.name, "TEXT") == 0) {
				dxf_attr_change(ent, 1, text);
			}
			
			return 1;
		}
	}
	else if (strcmp(ent->obj.name, "INSERT") == 0) {
		/* ATTRIB DXF entities inside INSERTs*/
		int num_attr = 0;
		dxf_node *attr = NULL, *nxt_attr = NULL;
		
		/* sweep INSERT */
		num_attr = 0;
		while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
			if (attr_idx == num_attr){
				/* perform replace in string, according entire flag */
				char *text = NULL;
				if (entire_el) text = txt_ent_repl(attr, L, search, repl);
				else text = txt_ent_repl_i(attr, L, search, repl, str_idx);
				/* replace the text  in entity */
				if (text) dxf_attr_change(attr, 1, text);
				return 1;
			}
			
			num_attr++;
			if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
		}
	}
	
	
	return 0;
}


int ent_replace_all (dxf_node * ent, lua_State *L, char * search, char * repl, enum dxf_graph filter){
	/* try to find and replace all match patterns in DXF entity.
	It perform entity filtering */
	
	/* verify structures */
	if (!ent) return 0;
	if (!L) return 0;
	if (ent->type != DXF_ENT) return 0;
	
	int ok = 0;
	
	if ( ( (strcmp(ent->obj.name, "TEXT") == 0) && (filter & DXF_TEXT) ) ||
		( (strcmp(ent->obj.name, "MTEXT") == 0) && (filter & DXF_MTEXT) ) )
	{
		/* TEXT and MTEXT DXF entities */
		/* perform replace in string */
		char *text = txt_ent_repl(ent, L, search, repl);
		if (text){
			/* replace the text  in entity */
			if (strcmp(ent->obj.name, "MTEXT") == 0) 
				mtext_change_text (ent, text, strlen(text), DWG_LIFE);
			else if (strcmp(ent->obj.name, "TEXT") == 0) {
				dxf_attr_change(ent, 1, text);
			}
			
			ok = 1;
		}
	}
	else if ( (strcmp(ent->obj.name, "INSERT") == 0) && (filter & DXF_ATTRIB) ){
		/* ATTRIB DXF entities inside INSERTs*/
		dxf_node *attr = NULL, *nxt_attr = NULL, *tmp = NULL;
		
		/* sweep INSERT */
		while (attr = dxf_find_obj_nxt(ent, &nxt_attr, "ATTRIB")){
			/* verify if it is a hidden attribute */
			if(tmp = dxf_find_attr2(attr, 70)){
				if(tmp->value.i_data & 1){
					if (nxt_attr) continue;
					else break;
				}
			}
			/* perform replace in string */
			char *text = txt_ent_repl(attr, L, search, repl);
			if (text) {
				/* replace the text  in entity */
				dxf_attr_change(attr, 1, text);
				ok = 1;
			}
			if (!nxt_attr) break; /* end of ATTRIBs in INSERT*/
		}
	}
	
	
	return ok;
}

int gui_find_interactive(gui_obj *gui){
	if (gui->modal != FIND) return 0;
	
	if (gui->ev & EV_CANCEL){ /* quit and back to default modal */
		gui->step = 0;
		gui_default_modal(gui);
	}
	
	if (gui->step > 0){
		/* draw the approximate rectangle on found text */
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
	
	
	if (gui->modal != FIND) { /* clear find/replace enviroment */
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
	
	if (!L){ /* init Lua engine and static structures */
		L = luaL_newstate(); /* opens Lua */
		luaL_openlibs(L); /* opens the standard libraries */
		
		/* custom Lua functions to indexed find and replace text */
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
		"				break\n"
		"			end\n"
		"		end\n"
		"	until not start\n"
		"	return n, ret_st, ret_end\n"
		"end\n";
		luaL_dostring(L, f);
		
		
		/* intit structs */
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
	static enum dxf_graph filter = DXF_TEXT | DXF_MTEXT | DXF_ATTRIB;
	
	double rect[4]; /* to draw a rectangle on found text */
	
	static int entire_el = 1, f_text = 1, f_mtext = 1, f_tag = 1;
	/* Title */
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, _l("Find/Replace text"), NK_TEXT_LEFT);
	/* string pattern to find */
	nk_label(gui->ctx, _l("Search:"), NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, search, DXF_MAX_CHARS, nk_filter_default);
	/* entity type filter */
	nk_layout_row_dynamic(gui->ctx, 20, 3);
	nk_checkbox_label(gui->ctx, _l("Text"), &f_text);
	nk_checkbox_label(gui->ctx, _l("MText"), &f_mtext);
	nk_checkbox_label(gui->ctx, _l("Tag"), &f_tag);
	
	if(f_text) filter |= DXF_TEXT;
	else filter &= ~DXF_TEXT;
	if(f_mtext) filter |= DXF_MTEXT;
	else filter &= ~DXF_MTEXT;
	if(f_tag) filter |= DXF_ATTRIB;
	else filter &= ~DXF_ATTRIB;
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	if (nk_button_label(gui->ctx, _l("Find Next")) && strlen(search) > 0 ){
		/* try to find next match */
		log[0] = 0;
		if(dwg_find(gui->drawing, L, search, rect, filter, &found, &next)){
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
				snprintf(log, 63, _l("End of search"));
			}
		}
		else {
			gui->step = 0;
			snprintf(log, 63, _l("No elements matched"));
		}
	}
	/* string pattern to replace */
	nk_label(gui->ctx, _l("Replace:"), NK_TEXT_LEFT);
	nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER|NK_EDIT_CLIPBOARD, repl, DXF_MAX_CHARS, nk_filter_default);
	
	nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 2, (float[]){0.45f, 0.55f});
	if (nk_button_label(gui->ctx, _l("Current")) && strlen(search) > 0 ){
		/* replace current match */
		log[0] = 0;
		dxf_node *new_ent = NULL;
		if (found.ent) new_ent = dxf_ent_copy(found.ent, DWG_LIFE); /* copy original entity */
		if (new_ent){
			/* replace text in entity */
			if(ent_replace (new_ent, L, search, repl, found.str_idx, found.attr_idx, entire_el)){
				/* replace original entity */
				new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
				dxf_obj_subst(found.ent, new_ent);
				
				/* update undo/redo list */
				do_add_entry(&gui->list_do, _l("REPLACE"));
				do_add_item(gui->list_do.current, found.ent, new_ent);
				
				if (next.ent == found.ent) { /* if is a partial replace */
					/* update index */
					if (next.str_idx > 1) next.str_idx--;
					next.ent = new_ent; /* update next entity */
				}
				found.ent = new_ent; /* update current entity */

			}
		}
		/* try to find next match */
		if(dwg_find(gui->drawing, L, search, rect, filter, &found, &next)){
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
				snprintf(log, 63, _l("End of search"));
			}
		}
		else {
			gui->step = 0;
			snprintf(log, 63, _l("No elements matched"));
		}
		
	}
	
	/* flag to replace all matches in entity's text. It permits full functionality
	of Lua gsub, like numbered group capture replace. */
	nk_checkbox_label(gui->ctx, _l("Entire element"), &entire_el);
	
	nk_layout_row_dynamic(gui->ctx, 20, 2);
	
	if (nk_button_label(gui->ctx, _l("Selection")) && strlen(search) > 0 ){
		/* find and replace text pattern in all entities in current selection, according type filter */
		log[0] = 0;
		int ini_do = 0, n = 0;
		
		list_node *curr_lst = gui->sel_list->next;
		/* sweep list of entities */
		while (curr_lst != NULL){
			if (curr_lst->data){
				/* look for text */
				if (ent_find (curr_lst->data, L, search, filter) ){
					/* copy original entity */
					dxf_node *new_ent = dxf_ent_copy((dxf_node *)curr_lst->data, DWG_LIFE);
					if (new_ent){
						/* replace the text */
						ent_replace_all (new_ent, L, search, repl, filter);
						
						new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
						dxf_obj_subst((dxf_node *)curr_lst->data, new_ent);
						
						/* update undo/redo list */
						if (!ini_do){
							do_add_entry(&gui->list_do, _l("REPLACE"));
							ini_do = 1;
						}
						do_add_item(gui->list_do.current, (dxf_node *)curr_lst->data, new_ent);
						curr_lst->data = new_ent; /* replace in list too */
						n++;
					}
				}
			}
			curr_lst = curr_lst->next;
		}
		if (n >0) snprintf(log, 63, _l("Total replaced: %d"), n);
		else snprintf(log, 63, _l("No elements matched"));
	}
	if (nk_button_label(gui->ctx, _l("All")) && strlen(search) > 0 ){
		/* find and replace text pattern in all entities in current drawing, according type filter */
		log[0] = 0;
		int ini_do = 0, n = 0;
		
		dxf_node *current = gui->drawing->ents->obj.content->next;
		/* sweep list of entities */
		while (current != NULL){
			/* look for text */
			if (ent_find (current, L, search, filter) ){
				/* copy original entity */
				dxf_node *new_ent = dxf_ent_copy(current, DWG_LIFE);
				if (new_ent){
					/* replace the text */
					ent_replace_all (new_ent, L, search, repl, filter);
					
					new_ent->obj.graphics = dxf_graph_parse(gui->drawing, new_ent, 0, DWG_LIFE);
					dxf_obj_subst(current, new_ent);
					
					/* update undo/redo list */
					if (!ini_do){
						do_add_entry(&gui->list_do, _l("REPLACE"));
						ini_do = 1;
					}
					do_add_item(gui->list_do.current, current, new_ent);
					n++;
				}
			}
			current = current->next;
		}
		if (n >0) snprintf(log, 63, _l("Total replaced: %d"), n);
		else snprintf(log, 63, _l("No elements matched"));
	}
	
	nk_layout_row_dynamic(gui->ctx, 20, 1);
	nk_label(gui->ctx, log, NK_TEXT_LEFT);
	
	return 1;
}