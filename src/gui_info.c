#include "gui_info.h"
#if(0)
#include "sqlite3.h"
#endif

void nk_dxf_ent_info (struct nk_context *ctx, dxf_node *ent, int id){ /* print the entity structure */
	/* use Nuklear widgets to show a DXF entity structure */
	/* this function is non recursive */
	
	int i = 0;
	int level = 0;
	dxf_node *current, *prev;
	int tree_st[10]; /* max of ten levels of entities inside entities*/
	char text[401];
	
	text[0] = 0;
	id *= 1000; /* each Tree widget has a unique id, up to 1000 inside main entity*/
	
	current = ent;
	while (current){
		prev = current;
		if (current->type == DXF_ENT){
			/* DXF entities are show as Tree widget */
      if (current->obj.id){
				id++; /* increment id for child Trees */
				if (level == 0){ /* verify if is the first Tree */
					if (tree_st[level] = nk_tree_push_id(ctx, NK_TREE_TAB,
              strpool_cstr2( &obj_pool, current->obj.id), NK_MINIMIZED, id)) {
						/* if Tree is not minimized, start the placement of child widgets */
						nk_layout_row_dynamic(ctx, 13, 1);
						nk_label(ctx, "-----", NK_TEXT_LEFT);
					}
				}
				else if (tree_st[level - 1]){ /* verify if the up level Tree is not minimized */
					if (tree_st[level] = nk_tree_push_id(ctx, NK_TREE_TAB,
            strpool_cstr2( &obj_pool, current->obj.id), NK_MINIMIZED, id)) {
						/* if Tree is not minimized, start the placement of child widgets */
						nk_layout_row_dynamic(ctx, 13, 1);
						nk_label(ctx, "-----", NK_TEXT_LEFT);
					}
				}
				else{
					tree_st[level] = 0;
				}
			}
			if (current->obj.content){
				/* starts entity content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				level++;
			}
		}
		else if (current->type == DXF_ATTR){
			/* DXF attributes are show as Label widget */
			/* combine Group and Value, acording its type, in a string */
			i = snprintf (text, 400, "%d = ", current->value.group);
			switch (current->value.t_data) {
				case DXF_STR:
          if (current->value.group == 2 || (current->value.group > 5 && current->value.group < 9) ){
            i += snprintf (text + i, 400 - i, strpool_cstr2( &name_pool, current->value.str));
          } else {
            i += snprintf (text + i, 400 - i, strpool_cstr2( &value_pool, current->value.str));
          }
					break;
				case DXF_FLOAT:
					i += snprintf (text + i, 400 - i, "%f", current->value.d_data);
					break;
				case DXF_INT:
					i += snprintf (text + i, 400 - i, "%d", current->value.i_data);
			}
			if (tree_st[level - 1]){ /* verify if the up level Tree is not minimized */
				nk_label(ctx, text, NK_TEXT_LEFT);
			}
			/* clear string */
			i = 0; text[0] = 0;
			
			current = current->next; /* go to the next in the list */
		}
		/* if the contents sweep reachs to end, try to up in structure */
		while (current == NULL){ 
			if (prev == ent){ /* back to original entity */
				/* ends the top level Tree, if not minimized */
				if (tree_st[level - 1]){
					nk_tree_pop(ctx);
				}
				current = NULL; /* ends loop */
				break;
			}
			
			prev = prev->master;
			if (prev){
				current = prev->next; /* continue loop */
				level --; /* up in structure*/
				/* ends the current Tree, if not minimized */
				if (tree_st[level]){
					nk_tree_pop(ctx);
				}
				/* back to original entity */
				if (prev == ent){
					current = NULL; /* ends loop */
					break;
				}
			}
			else{ /* if structure ends */
				current = NULL; /* ends loop */
				break;
			}
		}
		
		if ((level < 0) || (level > 9)){/* verify if level is out of admissible range */
			current = NULL; /* ends loop */
			break;
		}
	}
}

int info_win (gui_obj *gui){
	int show_info = 1;
	int i = 0;
	gui->next_win_x += gui->next_win_w + 3;
	//gui->next_win_y += gui->next_win_h + 3;
	gui->next_win_w = 200;
	gui->next_win_h = 300;
	
	//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "Info", NK_WINDOW_CLOSABLE, nk_rect(310, 50, 200, 300))){
	if (nk_begin(gui->ctx, _l("Info"), nk_rect(gui->next_win_x, gui->next_win_y, gui->next_win_w, gui->next_win_h),
	NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
	NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)){
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		
		#if (0)
		if (nk_button_label(gui->ctx, _l("Generate DB"))){
			sqlite3 *db;
			sqlite3_stmt *res;
			char *zErrMsg = 0;
			int rc;
			
			
			rc = sqlite3_open("test.db", &db);
    
			if (rc != SQLITE_OK) {
				printf(_l("Cannot open database: %s\n"), sqlite3_errmsg(db));
			}
			else {
				char *sql = "DROP TABLE IF EXISTS Ents;" 
					"CREATE TABLE Ents(Id BIGINT, Entity TEXT, Layer TEXT, Color INT, LineType TEXT, LineW DECIMAL);";
				rc = sqlite3_exec(db, sql, 0, 0, 0);

				if (rc != SQLITE_OK) {
					printf(_l("Failed to fetch data: %s\n"), sqlite3_errmsg(db));
				}    
				else {
					
					if (gui->sel_list != NULL){
						char layer[DXF_MAX_CHARS+1];
						char ltype[DXF_MAX_CHARS+1];
						int color, lw;
						dxf_node *ent = NULL, *tmp = NULL;
						
						list_node *current = gui->sel_list->next;
						char *sql = "INSERT INTO Ents VALUES (?1, ?2, ?3, ?4, ?5, ?6);";
						rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

						if (rc != SQLITE_OK) {
							printf(_l("Failed to insert data: %s\n"), sqlite3_errmsg(db));
							current = NULL;
						}
						
						
						// starts the content sweep 
						//i = 2;
						while (current != NULL){
							if (current->data){
								if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
									ent = current->data;
									
									
									/* clear variables */
									layer[0] = 0;
									ltype[0] = 0;
									color = 256; /* color by layer */
									lw = -1; /* line weight by layer */
									
									/* get raw parameters directly from entity */
									if(tmp = dxf_find_attr2(ent, 8))
										strncpy (layer, tmp->value.s_data, DXF_MAX_CHARS);
									if(tmp = dxf_find_attr2(ent, 6))
										strncpy (ltype, tmp->value.s_data, DXF_MAX_CHARS);
									if(tmp = dxf_find_attr2(ent, 62))
										color = abs(tmp->value.i_data);
									if(tmp = dxf_find_attr2(ent, 370))
										lw = tmp->value.i_data;
									
									
									
									sqlite3_bind_int64(res, 1, (sqlite3_int64)ent);
									sqlite3_bind_text(res, 2, ent->obj.name, -1, NULL);
									sqlite3_bind_text(res, 3, layer, -1, NULL);
									sqlite3_bind_int(res, 4, color);
									sqlite3_bind_text(res, 5, ltype, -1, NULL);
									sqlite3_bind_double(res, 6, lw / 100.0);
									
									rc = sqlite3_step(res);

									if (rc != SQLITE_DONE) {
										printf(_l("Failed to put data\n") );
										break;
									}
									sqlite3_reset(res);
									
								}
							}
							current = current->next;
						}
						
						sqlite3_finalize(res);
					}
					
					
					
				}
			}
			
			sqlite3_close(db);
		}
		#endif
		
		nk_label(gui->ctx, _l("BLK:"), NK_TEXT_LEFT);
		i = 1;
		nk_dxf_ent_info (gui->ctx, gui->drawing->blks, i);
		
		nk_label(gui->ctx, _l("ENTS:"), NK_TEXT_LEFT);
		if (gui->sel_list != NULL){				
			list_node *current = gui->sel_list->next;
			// starts the content sweep 
			i = 2;
			while (current != NULL){
				if (current->data){
					if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							
						// -------------------------------------------
						nk_dxf_ent_info (gui->ctx, (dxf_node *)current->data, i);
						i++;
						
						//---------------------------------------
					}
				}
				current = current->next;
			}
		}
		//nk_popup_end(gui->ctx);
	} else show_info = 0;
	nk_end(gui->ctx);
	
	return show_info;
}