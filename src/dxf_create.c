#include "dxf_create.h"
#include "dxf_hatch.h"

/*
enum LineWeight { //AcDb::LineWeight
  kLnWt000 = 0,
  kLnWt005 = 5,
  kLnWt009 = 9,
  kLnWt013 = 13,
  kLnWt015 = 15,
  kLnWt018 = 18,
  kLnWt020 = 20,
  kLnWt025 = 25,
  kLnWt030 = 30,
  kLnWt035 = 35,
  kLnWt040 = 40,
  kLnWt050 = 50,
  kLnWt053 = 53,
  kLnWt060 = 60,
  kLnWt070 = 70,
  kLnWt080 = 80,
  kLnWt090 = 90,
  kLnWt100 = 100,
  kLnWt106 = 106,
  kLnWt120 = 120,
  kLnWt140 = 140,
  kLnWt158 = 158,
  kLnWt200 = 200,
  kLnWt211 = 211,
  kLnWtByLayer = -1,
  kLnWtByBlock = -2,
  kLnWtByLwDefault = -3
};
*/

void * do_mem_pool(enum dxf_pool_action action){
	
	static struct do_pool_slot entry, item;
	int i;
	
	void *ret_ptr = NULL;
	
	/* initialize the pool, the first allocation */
	if (entry.size < 1){
		entry.pool[0] = malloc(DO_PAGE * sizeof(struct do_entry));
		if (entry.pool){
			entry.size = 1;
			//printf("Init entry\n");
		}
	}
	
	/* if current page is full */
	if ((entry.pos >= DO_PAGE) && (entry.size > 0)){
		/* try to change to page previuosly allocated */
		if (entry.page < entry.size - 1){
			entry.page++;
			entry.pos = 0;
			//printf("change entry page\n");
		}
		/* or then allocatte a new page */
		else if(entry.page < DO_PAGES-1){
			entry.pool[entry.page + 1] = malloc(DO_PAGE * sizeof(struct do_entry));
			if (entry.pool[entry.page + 1]){
				entry.page++;
				entry.size ++;
				entry.pos = 0;
				//printf("Realloc entry\n");
			}
		}
	}
	
	/* initialize the pool, the first allocation */
	if (item.size < 1){
		item.pool[0] = malloc(DO_PAGE * sizeof(struct do_item));
		if (item.pool){
			item.size = 1;
			//printf("Init item\n");
		}
	}
	
	/* if current page is full */
	if ((item.pos >= DO_PAGE) && (item.size > 0)){
		/* try to change to page previuosly allocated */
		if (item.page < item.size - 1){
			item.page++;
			item.pos = 0;
			//printf("change item page\n");
		}
		/* or then allocatte a new page */
		else if(item.page < DO_PAGES-1){
			item.pool[item.page + 1] = malloc(DO_PAGE * sizeof(struct do_item));
			if (item.pool[item.page + 1]){
				item.page++;
				item.size ++;
				item.pos = 0;
				//printf("Realloc item\n");
			}
		}
	}
	
	ret_ptr = NULL;
	
	if ((entry.pool[entry.page] != NULL)){
		switch (action){
			case ADD_DO_ENTRY:
				if (entry.pos < DO_PAGE){
					ret_ptr = &(((struct do_entry *)entry.pool[entry.page])[entry.pos]);
					entry.pos++;
				}
				break;
			case ZERO_DO_ENTRY:
				entry.pos = 0;
				entry.page = 0;
				break;
			case ADD_DO_ITEM:
				if (item.pos < DO_PAGE){
					ret_ptr = &(((struct do_item *)item.pool[item.page])[item.pos]);
					item.pos++;
				}
				break;
			case ZERO_DO_ITEM:
				item.pos = 0;
				item.page = 0;
				break;
			case FREE_DO_ALL:
				for (i = 0; i < entry.size; i++){
					free(entry.pool[i]);
					entry.pool[i] = NULL;
				}
				entry.pos = 0;
				entry.page = 0;
				entry.size = 0;
				for (i = 0; i < item.size; i++){
					free(item.pool[i]);
					item.pool[i] = NULL;
				}
				item.pos = 0;
				item.page = 0;
				item.size = 0;
				break;
		}
	}

	return ret_ptr;
}

int do_add_item(struct do_entry *entry, dxf_node *old_obj, dxf_node *new_obj){
	int ret = 0;
	if ((entry != NULL) && ((old_obj != NULL) || (new_obj != NULL))){
		struct do_item *item = do_mem_pool(ADD_DO_ITEM);
		if (item){
			if ((entry->current) && (entry->list)){
				entry->current->next = item;
				item->prev = entry->current;
				entry->current = item;
			}
			else{ /* initialize entry's list*/
				entry->list = item;
				entry->current = item;
				item->prev = NULL;
			}
			item->next = NULL;
			item->old_obj = old_obj;
			item->new_obj = new_obj;
			ret = 1;
		}
	}
	return ret;
};

int do_add_entry(struct do_list *list, char *text){
	int ret = 0;
	if (list != NULL){
		struct do_entry *entry = do_mem_pool(ADD_DO_ENTRY);
		if ((entry) && (list->current) && (list->list)){
			list->count++;
			list->current->next = entry;
			entry->prev = list->current;
			entry->next = NULL;
			entry->list = NULL;
			entry->current = NULL;
			list->current = entry;
			
			entry->text[0] = 0; /*initialize string*/
			if (text){
				strncpy(entry->text, text, ACT_CHARS);
				entry->text[ACT_CHARS - 1] = 0; /*terminate string*/
			}
			ret = 1;
		}
	}
	return ret;
};

int init_do_list(struct do_list *list){
	int ret = 0;
	if (list != NULL){
		struct do_entry *entry = do_mem_pool(ADD_DO_ENTRY);
		if (entry){
			list->count = 0;
			list->list = entry;
			list->current = entry;
			entry->prev = NULL;
			entry->next = NULL;
			
			entry->text[0] = 0; /*initialize string*/
			ret =1;
		}
	}
	return ret;
}

int do_undo(struct do_list *list){
	int ret = 0;
	if (list != NULL){
		struct do_entry *entry = list->current;
		if (entry){ if (entry->prev){
			struct do_item *curr_item = entry->list;
			while (curr_item){
				ret =1;
				dxf_obj_subst(curr_item->new_obj, curr_item->old_obj);
				curr_item = curr_item->next;
			}
			list->current = entry->prev; /* change current in the list to prev*/
		}}
	}
	return ret;
}

int do_redo(struct do_list *list){
	int ret = 0;
	if (list != NULL){
		struct do_entry *entry = list->current;
		if (entry){ if (entry->next){
			entry = entry->next;
			struct do_item *curr_item = entry->list;
			while (curr_item){
				ret =1;
				dxf_obj_subst(curr_item->old_obj, curr_item->new_obj);
				curr_item = curr_item->next;
			}
			list->current = entry; /* change current in the list to next*/
		}}
	}
	return ret;
}

int dxf_obj_append(dxf_node *master, dxf_node *obj){
	if ((master) && (obj)){
		if (master->type == DXF_ENT){
			obj->master = master;
			/* start search at end of master's list */
			dxf_node *next = NULL, *prev = master->end;
			
			/* append object between prev and next nodes */
			obj->prev = prev;
			if (prev){
				next = prev->next;
				prev->next = obj;
			}
			obj->next = next;
			if (next){
				next->prev = obj;
			}
			
			if (prev == master->end){
				master->end = obj;
			}
			
			return 1;
		}
	}
	return 0;
}

int dxf_obj_detach(dxf_node *obj){
	/* remove the object from its list */
	if (obj){
		dxf_node *master = obj->master;
		dxf_node *prev = obj->prev;
		dxf_node *next = obj->next;
		/* rebuilt the insertion point */
		if (prev){
			prev->next = next;
			obj->prev = NULL;
		}
		if (next){
			next->prev = prev;
			obj->next = NULL;
		}
		/* verify if the object is at end of master list */
		if (master){
			if (obj == master->end){
				master->end = prev;
			}
		}
		return 1;
	}
	return 0;
}

int dxf_obj_subst(dxf_node *orig, dxf_node *repl){
	/* substitute the orig object from its list for the replace obj */
	if ((repl) && (orig)){
		dxf_node *master = orig->master;
		dxf_node *prev = orig->prev;
		dxf_node *next = orig->next;
		dxf_node *end = orig->end;
		
		/* substitute inherits orig's properties */
		repl->master = master;
		repl->prev = prev;
		repl->next = next;
		repl->end = end;
		
		/* rebuilt the insertion point */
		if (prev){
			prev->next = repl;
		}
		if (next){
			next->prev = repl;
		}
		/* verify if the orig is at end of master list */
		if (master){
			if (orig == master->end){
				master->end = repl;
			}
		}
		return 1;
	}
	else if ((repl == NULL) && (orig)){
		/* detach, but preserve orig properties*/
		dxf_node *master = orig->master;
		dxf_node *prev = orig->prev;
		dxf_node *next = orig->next;
		/* rebuilt the insertion point */
		if (prev){
			prev->next = next;
		}
		if (next){
			next->prev = prev;
		}
		/* verify if the orig is at end of master list */
		if (master){
			if (orig == master->end){
				master->end = prev;
			}
		}
		return 1;
	}
	else if ((repl) && (orig == NULL)){
		/* restore the repl at their referenced point*/
		dxf_node *master = repl->master;
		dxf_node *prev = repl->prev;
		dxf_node *next = repl->next;
		/* rebuilt the insertion point */
		if (prev){
			prev->next = repl;
		}
		if (next){
			next->prev = repl;
		}
		/* verify if the orig is at end of master list */
		if (master){
			if (prev == master->end){
				master->end = repl;
			}
		}
		return 1;
	}
	return 0;
}

dxf_node * dxf_obj_cpy(dxf_node *orig, int pool){
	/* copy a DXF object */
	dxf_node *new_obj = NULL;
	if (orig){
		new_obj = (dxf_node *) dxf_mem_pool(ADD_DXF, pool);
		if (new_obj){
			new_obj->master = orig->master;
			new_obj->prev = orig->prev;
			new_obj->next = orig->next;
			new_obj->end = orig->end;
			new_obj->type = orig->type;
			if (new_obj->type == DXF_ENT){
				new_obj->obj = orig->obj;
			}
			else if (new_obj->type == DXF_ATTR){
				new_obj->value = orig->value;
			}
		}
	}
	return new_obj;
}

int dxf_attr_append(dxf_node *master, int group, void *value){
	if (master){
		if (master->type == DXF_ENT){
			int type = dxf_ident_attr_type(group);
			dxf_node *new_attr = dxf_attr_new(group, type, value);
			if (new_attr){
				new_attr->master = master;
				/* start search at end of master's list */
				dxf_node *next = NULL, *prev = master->end;
				/*  find the last attribute*/
				if (prev){ /*skip if is an entity */
					while (prev->type == DXF_ENT){
						prev = prev->prev;
						if (!prev) break;
					}
				}	
				
				/* append new attr between prev and next nodes */
				new_attr->prev = prev;
				if (prev){
					next = prev->next;
					prev->next = new_attr;
				}
				new_attr->next = next;
				if (next){
					next->prev = new_attr;
				}
				
				if (prev == master->end){
					master->end = new_attr;
				}
				
				return 1;
			}
		}
	}
	return 0;
}

int dxf_attr_change(dxf_node *master, int group, void *value){
	if (master){
		/* find the first attribute*/
		dxf_node *found_attr = dxf_find_attr2(master, group);
		if (found_attr){
			/* identify the type of attrib, according DXF group specification */
			int type = dxf_ident_attr_type(group);
			switch(type) {
				/* change the data */
				case DXF_FLOAT :
					found_attr->value.d_data = *((double *)value);
					break;
				case DXF_INT :
					found_attr->value.i_data = *((int *)value);
					break;
				case DXF_STR :
					found_attr->value.s_data[0] = 0;
					strncpy(found_attr->value.s_data,(char *) value, DXF_MAX_CHARS); /* and copy the string */
			}
			return 1;
		}
	}
	return 0;
}

int dxf_attr_change_i(dxf_node *master, int group, void *value, int idx){
	if (master){
		/* find the attribute, indicated by index */
		dxf_node *found_attr = dxf_find_attr_i(master, group, idx);
		if (found_attr){
			/* identify the type of attrib, according DXF group specification */
			int type = dxf_ident_attr_type(group);
			switch(type) {
				/* change the data */
				case DXF_FLOAT :
					found_attr->value.d_data = *((double *)value);
					break;
				case DXF_INT :
					found_attr->value.i_data = *((int *)value);
					break;
				case DXF_STR :
					found_attr->value.s_data[0] = 0;
					strncpy(found_attr->value.s_data,(char *) value, DXF_MAX_CHARS); /* and copy the string */
			}
			return 1;
		}
	}
	return 0;
}

int dxf_find_ext_appid(dxf_node *obj, char *appid, dxf_node **start, dxf_node **end){
	/* find the range of attributes of extended data, indicated by APPID */
	dxf_node *current;
	int found = 0;
	
	*start = NULL;
	*end = NULL;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				/* try to find the first entry, by matching the APPID */
				if (!found){
					if (current->type == DXF_ATTR){
						if(current->value.group == 1001){
							if(strcmp((char*) current->value.s_data, appid) == 0){
								found = 1; /* appid found */
								*start = current;
								*end = current;
							}
						}
					}
				}
				else{
					/* after the first entry, look by end */
					if (current->type == DXF_ATTR){
						/* breaks if is found a new APPID entry */
						if(current->value.group == 1001){
							break;
						}
						/* update the end mark */
						*end = current;
					}
					/* breaks if is found a entity */
					else break;
				}
				current = current->next;
			}
		}
	}
	return found;
}

int dxf_ext_append(dxf_node *master, char *appid, int group, void *value){
	/* appdend new attrib on extended data, indicated by APPID */
	if (master){
		if (master->type == DXF_ENT){
			dxf_node *start, *end, *prev = NULL, *next = NULL;
			/* look for appid in master */
			if(dxf_find_ext_appid(master, appid, &start, &end)){
				int type = dxf_ident_attr_type(group);
				dxf_node *new_attr = dxf_attr_new(group, type, value);
				if (new_attr){
					new_attr->master = master;
					/* append at end mark */
					prev = end;
					
					/* append new attr between prev and next nodes */
					new_attr->prev = prev;
					if (prev){
						next = prev->next;
						prev->next = new_attr;
					}
					new_attr->next = next;
					if (next){
						next->prev = new_attr;
					}
					
					if (prev == master->end){
						master->end = new_attr;
					}
					
					return 1;
				}
			}
		}
	}
	return 0;
}

void dxf_obj_transverse(dxf_node *source){
	/* it is only a a prototype of function to transverse a DXF entity */
	dxf_node *current = NULL;
	dxf_node *prev = NULL;
	
	if (source){ 
		if (source->type == DXF_ENT){
			if (source->obj.content){
				current = source->obj.content->next;
				prev = current;
			}
		}
	}

	while (current){
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				current = current->obj.content->next;
				prev = current;
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			
			
		}
		
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev == source){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
}

dxf_node *dxf_ent_copy(dxf_node *source, int pool_dest){
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *dest = NULL, *curr_dest = NULL, *new_ent = NULL;
	
	if (source){ 
		if (source->type == DXF_ENT){
			if (source->obj.content){
				current = source->obj.content->next;
				prev = current;
				
				dest = dxf_obj_new (source->obj.name);
				curr_dest = dest;
			}
		}
	}

	while ((current) && (curr_dest)){
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				
				new_ent = dxf_obj_new (current->obj.name);
				dxf_obj_append(curr_dest, new_ent);
				curr_dest = new_ent;
				
				/* starts the content sweep */
				current = current->obj.content->next;
				prev = current;
				
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			if (current->value.t_data == DXF_STR){
				dxf_attr_append(curr_dest, current->value.group, current->value.s_data);
			} else if (current->value.t_data == DXF_FLOAT){
				dxf_attr_append(curr_dest, current->value.group, &current->value.d_data);
			} else if (current->value.t_data == DXF_INT){
				dxf_attr_append(curr_dest, current->value.group, &current->value.i_data);
			}
			
		}
		
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			/* try to back in structure hierarchy */
			prev = prev->master;
			curr_dest = curr_dest->master;
			if (prev == source){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	
	return dest;
}

dxf_node * dxf_new_line (double x0, double y0, double z0,
double x1, double y1, double z1,
int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF LINE */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbLine";
	int ok = 1;
	dxf_node * new_line = dxf_obj_new ("LINE");
	
	ok &= dxf_attr_append(new_line, 5, (void *) handle);
	//ok &= dxf_attr_append(new_line, 330, (void *) handle);
	ok &= dxf_attr_append(new_line, 100, (void *) dxf_class);
	ok &= dxf_attr_append(new_line, 67, (void *) &paper);
	ok &= dxf_attr_append(new_line, 8, (void *) layer);
	ok &= dxf_attr_append(new_line, 6, (void *) ltype);
	ok &= dxf_attr_append(new_line, 62, (void *) &color);
	ok &= dxf_attr_append(new_line, 370, (void *) &lw);
	
	ok &= dxf_attr_append(new_line, 100, (void *) dxf_subclass);
	
	ok &= dxf_attr_append(new_line, 10, (void *) &x0);
	ok &= dxf_attr_append(new_line, 20, (void *) &y0);
	ok &= dxf_attr_append(new_line, 30, (void *) &z0);
	ok &= dxf_attr_append(new_line, 11, (void *) &x1);
	ok &= dxf_attr_append(new_line, 21, (void *) &y1);
	ok &= dxf_attr_append(new_line, 31, (void *) &z1);
	
	/* test */
	/* -------------------------------------------  
	ok &= dxf_attr_append(new_line, 1001, (void *) "ZECRUEL");
	ok &= dxf_attr_append(new_line, 1001, (void *) "ACAD");
	
	ok &= dxf_ext_append(new_line, "ZECRUEL", 1002, (void *) "{");
	ok &= dxf_ext_append(new_line, "ACAD", 1002, (void *) "{");
	ok &= dxf_ext_append(new_line, "ACAD", 1000, (void *) "test1");
	ok &= dxf_ext_append(new_line, "ZECRUEL", 1000, (void *) "test2");
	ok &= dxf_ext_append(new_line, "ACAD", 1002, (void *) "}");
	ok &= dxf_ext_append(new_line, "ZECRUEL", 1002, (void *) "}");
	/* -------------------------------------------  */
	if(ok){
		return new_line;
	}

	return NULL;
}

dxf_node * dxf_new_lwpolyline (double x0, double y0, double z0,
double bulge, int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF LWPOLYLINE */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbPolyline";
	int ok = 1;
	int verts = 1, flags = 0;
	dxf_node * new_poly = dxf_obj_new ("LWPOLYLINE");
	
	ok &= dxf_attr_append(new_poly, 5, (void *) handle);
	//ok &= dxf_attr_append(new_poly, 330, (void *) handle);
	ok &= dxf_attr_append(new_poly, 100, (void *) dxf_class);
	ok &= dxf_attr_append(new_poly, 67, (void *) &paper);
	ok &= dxf_attr_append(new_poly, 8, (void *) layer);
	ok &= dxf_attr_append(new_poly, 6, (void *) ltype);
	ok &= dxf_attr_append(new_poly, 62, (void *) &color);
	ok &= dxf_attr_append(new_poly, 370, (void *) &lw);
	
	ok &= dxf_attr_append(new_poly, 100, (void *) dxf_subclass);
	ok &= dxf_attr_append(new_poly, 90, (void *) &verts);
	ok &= dxf_attr_append(new_poly, 70, (void *) &flags);
	/* place the first vertice */
	ok &= dxf_attr_append(new_poly, 10, (void *) &x0);
	ok &= dxf_attr_append(new_poly, 20, (void *) &y0);
	ok &= dxf_attr_append(new_poly, 30, (void *) &z0);
	//ok &= dxf_attr_append(new_poly, 42, (void *) &bulge);
	
	if(ok){
		return new_poly;
	}

	return NULL;
}

int dxf_lwpoly_append (dxf_node * poly,
double x0, double y0, double z0, double bulge){
	/* append a vertice in a LWPOLYLINE */
	int ok = 1;
	/* look for the vertices counter attribute */
	dxf_node * verts = dxf_find_attr_i(poly, 90, 0);
	if (verts){ /* place the new vertice */
		ok &= dxf_attr_append(poly, 42, (void *) &bulge);
		ok &= dxf_attr_append(poly, 10, (void *) &x0);
		ok &= dxf_attr_append(poly, 20, (void *) &y0);
		ok &= dxf_attr_append(poly, 30, (void *) &z0);
		
		
		if (ok) verts->value.i_data++; /* increment the verts counter */
		
		return ok;
	}
	return 0;
}

int dxf_lwpoly_remove (dxf_node * poly, int idx){
	/* remove a vertice in a LWPOLYLINE, indicated by index */
	int ok = 1;
	/* look for the vertices counter attribute */
	dxf_node * verts = dxf_find_attr_i(poly, 90, 0);
	if (verts){
		/* look for the vertice attributes */
		dxf_node * x = dxf_find_attr_i(poly, 10, idx);
		dxf_node * y = dxf_find_attr_i(poly, 20, idx);
		dxf_node * z = dxf_find_attr_i(poly, 30, idx);
		dxf_node * bulge = dxf_find_attr_i(poly, 42, idx);
		
		/* detach the attribs from the list */
		ok &= dxf_obj_detach(x);
		ok &= dxf_obj_detach(y);
		ok &= dxf_obj_detach(z);
		
		if (ok) {
			dxf_obj_detach(bulge); /* bulge is optional, so the ok flag is not affected*/
			verts->value.i_data--; /* decrement the verts counter */
		}
		
		return ok;
	}
	return 0;
}

dxf_node * dxf_new_circle (double x0, double y0, double z0,
double r, int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF CIRCLE */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbCircle";
	int ok = 1;
	dxf_node * new_circle = dxf_obj_new ("CIRCLE");
	
	ok &= dxf_attr_append(new_circle, 5, (void *) handle);
	ok &= dxf_attr_append(new_circle, 100, (void *) dxf_class);
	ok &= dxf_attr_append(new_circle, 67, (void *) &paper);
	ok &= dxf_attr_append(new_circle, 8, (void *) layer);
	ok &= dxf_attr_append(new_circle, 6, (void *) ltype);
	ok &= dxf_attr_append(new_circle, 62, (void *) &color);
	ok &= dxf_attr_append(new_circle, 370, (void *) &lw);
	
	ok &= dxf_attr_append(new_circle, 100, (void *) dxf_subclass);
	/* place the first vertice */
	ok &= dxf_attr_append(new_circle, 10, (void *) &x0);
	ok &= dxf_attr_append(new_circle, 20, (void *) &y0);
	ok &= dxf_attr_append(new_circle, 30, (void *) &z0);
	ok &= dxf_attr_append(new_circle, 40, (void *) &r);
	
	if(ok){
		return new_circle;
	}

	return NULL;
}

dxf_node * dxf_new_text (double x0, double y0, double z0, double h,
char *txt, int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF TEXT */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbText";
	const char *t_style = "STANDARD";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0;
	dxf_node * new_text = dxf_obj_new ("TEXT");
	
	ok &= dxf_attr_append(new_text, 5, (void *) handle);
	ok &= dxf_attr_append(new_text, 100, (void *) dxf_class);
	ok &= dxf_attr_append(new_text, 67, (void *) &paper);
	ok &= dxf_attr_append(new_text, 8, (void *) layer);
	ok &= dxf_attr_append(new_text, 6, (void *) ltype);
	ok &= dxf_attr_append(new_text, 62, (void *) &color);
	ok &= dxf_attr_append(new_text, 370, (void *) &lw);
	
	ok &= dxf_attr_append(new_text, 100, (void *) dxf_subclass);
	/* place the first vertice */
	ok &= dxf_attr_append(new_text, 10, (void *) &x0);
	ok &= dxf_attr_append(new_text, 20, (void *) &y0);
	ok &= dxf_attr_append(new_text, 30, (void *) &z0);
	ok &= dxf_attr_append(new_text, 40, (void *) &h);
	ok &= dxf_attr_append(new_text, 1, (void *) txt);
	ok &= dxf_attr_append(new_text, 50, (void *) &d_zero);
	ok &= dxf_attr_append(new_text, 7, (void *) t_style);
	ok &= dxf_attr_append(new_text, 72, (void *) &int_zero);
	
	ok &= dxf_attr_append(new_text, 11, (void *) &x0);
	ok &= dxf_attr_append(new_text, 21, (void *) &y0);
	ok &= dxf_attr_append(new_text, 31, (void *) &z0);
	ok &= dxf_attr_append(new_text, 100, (void *) dxf_subclass);
	ok &= dxf_attr_append(new_text, 73, (void *) &int_zero);
	
	if(ok){
		return new_text;
	}

	return NULL;
}

dxf_node * dxf_new_attdef (double x0, double y0, double z0, double h,
char *txt, char *tag, int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF TEXT */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass1 = "AcDbText";
	const char *dxf_subclass2 = "AcDbAttributeDefinition";
	const char *t_style = "STANDARD";
	const char *prompt = "value:";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0;
	dxf_node * attdef = dxf_obj_new ("ATTDEF");
	
	ok &= dxf_attr_append(attdef, 5, (void *) handle);
	ok &= dxf_attr_append(attdef, 100, (void *) dxf_class);
	ok &= dxf_attr_append(attdef, 67, (void *) &paper);
	ok &= dxf_attr_append(attdef, 8, (void *) layer);
	ok &= dxf_attr_append(attdef, 6, (void *) ltype);
	ok &= dxf_attr_append(attdef, 62, (void *) &color);
	ok &= dxf_attr_append(attdef, 370, (void *) &lw);
	
	ok &= dxf_attr_append(attdef, 100, (void *) dxf_subclass1);
	/* place the first vertice */
	ok &= dxf_attr_append(attdef, 10, (void *) &x0);
	ok &= dxf_attr_append(attdef, 20, (void *) &y0);
	ok &= dxf_attr_append(attdef, 30, (void *) &z0);
	ok &= dxf_attr_append(attdef, 40, (void *) &h);
	ok &= dxf_attr_append(attdef, 1, (void *) txt);
	ok &= dxf_attr_append(attdef, 50, (void *) &d_zero);
	ok &= dxf_attr_append(attdef, 7, (void *) t_style);
	ok &= dxf_attr_append(attdef, 72, (void *) &int_zero);
	
	ok &= dxf_attr_append(attdef, 11, (void *) &x0);
	ok &= dxf_attr_append(attdef, 21, (void *) &y0);
	ok &= dxf_attr_append(attdef, 31, (void *) &z0);
	ok &= dxf_attr_append(attdef, 100, (void *) dxf_subclass2);
	ok &= dxf_attr_append(attdef, 3, (void *) prompt);
	ok &= dxf_attr_append(attdef, 2, (void *) tag);
	ok &= dxf_attr_append(attdef, 70, (void *) &int_zero);
	ok &= dxf_attr_append(attdef, 74, (void *) &int_zero);
	
	if(ok){
		return attdef;
	}

	return NULL;
}

dxf_node * dxf_new_attrib (double x0, double y0, double z0, double h,
char *txt, char *tag, int color, char *layer, char *ltype, int lw, int paper){
	/* create a new DXF TEXT */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass1 = "AcDbText";
	const char *dxf_subclass2 = "AcDbAttribute";
	const char *t_style = "STANDARD";
	const char *prompt = "value:";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0;
	dxf_node * attrib = dxf_obj_new ("ATTRIB");
	
	ok &= dxf_attr_append(attrib, 5, (void *) handle);
	ok &= dxf_attr_append(attrib, 100, (void *) dxf_class);
	ok &= dxf_attr_append(attrib, 67, (void *) &paper);
	ok &= dxf_attr_append(attrib, 8, (void *) layer);
	ok &= dxf_attr_append(attrib, 6, (void *) ltype);
	ok &= dxf_attr_append(attrib, 62, (void *) &color);
	ok &= dxf_attr_append(attrib, 370, (void *) &lw);
	
	ok &= dxf_attr_append(attrib, 100, (void *) dxf_subclass1);
	/* place the first vertice */
	ok &= dxf_attr_append(attrib, 10, (void *) &x0);
	ok &= dxf_attr_append(attrib, 20, (void *) &y0);
	ok &= dxf_attr_append(attrib, 30, (void *) &z0);
	ok &= dxf_attr_append(attrib, 40, (void *) &h);
	ok &= dxf_attr_append(attrib, 1, (void *) txt);
	ok &= dxf_attr_append(attrib, 50, (void *) &d_zero);
	//ok &= dxf_attr_append(attrib, 41, (void *) &d_zero);
	//ok &= dxf_attr_append(attrib, 51, (void *) &d_zero);
	ok &= dxf_attr_append(attrib, 7, (void *) t_style);
	//ok &= dxf_attr_append(attrib, 71, (void *) t_style);
	ok &= dxf_attr_append(attrib, 72, (void *) &int_zero);
	ok &= dxf_attr_append(attrib, 11, (void *) &x0);
	ok &= dxf_attr_append(attrib, 21, (void *) &y0);
	ok &= dxf_attr_append(attrib, 31, (void *) &z0);
	//ok &= dxf_attr_append(attrib, 210, (void *) t_style);
	//ok &= dxf_attr_append(attrib, 220, (void *) t_style);
	//ok &= dxf_attr_append(attrib, 230, (void *) t_style);
	
	ok &= dxf_attr_append(attrib, 100, (void *) dxf_subclass2);
	ok &= dxf_attr_append(attrib, 2, (void *) tag);
	ok &= dxf_attr_append(attrib, 70, (void *) &int_zero);
	ok &= dxf_attr_append(attrib, 74, (void *) &int_zero);
	
	if(ok){
		return attrib;
	}

	return NULL;
}

dxf_node * dxf_attdef_cpy (dxf_node *text, char *tag, double x0, double y0, double z0){
	/*create new attdef ent by copying parameters of an text ent */
	/* offset position of attdef by x0, y0, z0*/
	if(text){
		if (text->type != DXF_ENT){
			return NULL; /*Fail - wrong type */
		}
		if (strcmp(text->obj.name, "TEXT") != 0){
			return NULL; /*Fail - wrong text */
		}
		
		double x1= 0.0, y1 = 0.0, z1 = 0.0;
		double x2= 0.0, y2 = 0.0, z2 = 0.0;
		double h = 1.0, rot = 0.0, t_scale_x = 1.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0;
		char txt[DXF_MAX_CHARS];
		char layer[DXF_MAX_CHARS], l_type[DXF_MAX_CHARS];
		char t_style[DXF_MAX_CHARS];
		int color = 0, paper = 0, lw = -2;
		int t_alin_v = 0, t_alin_h = 0;
		
		
		/* clear the strings */
		txt[0] = 0;
		layer[0] = 0;
		l_type[0] = 0;
		t_style[0] = 0;
		
		dxf_node *current = NULL;
		if (text->obj.content){
			current = text->obj.content->next;
		}
		while (current){
			if (current->type == DXF_ATTR){ /* scan parameters */
				switch (current->value.group){
					case 1:
						strcpy(txt, current->value.s_data);
						break;
					case 7:
						strcpy(t_style, current->value.s_data);
						break;
					case 6:
						strcpy(l_type, current->value.s_data);
						break;
					case 8:
						strcpy(layer, current->value.s_data);
						break;
					case 10:
						x1 = current->value.d_data;
						break;
					case 11:
						x2 = current->value.d_data;
						break;
					case 20:
						y1 = current->value.d_data;
						break;
					case 21:
						y2 = current->value.d_data;
						break;
					case 30:
						z1 = current->value.d_data;
						break;
					case 31:
						z2 = current->value.d_data;
						break;
					case 40:
						h = current->value.d_data;
						break;
					case 41:
						t_scale_x = current->value.d_data;
						break;
					case 50:
						rot = current->value.d_data;
						break;
					case 62:
						color = current->value.i_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 72:
						t_alin_h = current->value.i_data;
						break;
					case 73:
						t_alin_v = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
						break;
					case 370:
						lw = current->value.i_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		
		dxf_node * attdef = dxf_new_attdef (x1 - x0, y1 - y0, z1 - z0, h,
			txt, tag, color, layer, l_type, lw, paper);
		
		dxf_attr_change(attdef, 11, (void *)(double[]){x2 - x0});
		dxf_attr_change(attdef, 21, (void *)(double[]){y2 - y0});
		dxf_attr_change(attdef, 31, (void *)(double[]){z2 - z0});
		dxf_attr_change(attdef, 50, (void *)&rot);
		dxf_attr_change(attdef, 7, (void *)t_style);
		dxf_attr_change(attdef, 41, (void *)&t_scale_x);
		dxf_attr_change(attdef, 72, (void *)&t_alin_h);
		dxf_attr_change(attdef, 74, (void *)&t_alin_v);
		dxf_attr_change(attdef, 210, (void *)&extru_x);
		dxf_attr_change(attdef, 220, (void *)&extru_y);
		dxf_attr_change(attdef, 230, (void *)&extru_z);
		
		return attdef;
	}

	return NULL;
}

dxf_node * dxf_attrib_cpy (dxf_node *attdef, double x0, double y0, double z0){
	/*create new attrib ent by copying parameters of an attdef ent */
	/* translate position of attrib by x0, y0, z0*/
	if(attdef){
		if (attdef->type != DXF_ENT){
			return NULL; /*Fail - wrong type */
		}
		if (strcmp(attdef->obj.name, "ATTDEF") != 0){
			return NULL; /*Fail - wrong attdef */
		}
		
		double x1= 0.0, y1 = 0.0, z1 = 0.0;
		double x2= 0.0, y2 = 0.0, z2 = 0.0;
		double h = 1.0, rot = 0.0, t_scale_x = 1.0;
		double extru_x = 0.0, extru_y = 0.0, extru_z = 1.0;
		char txt[DXF_MAX_CHARS], tag[DXF_MAX_CHARS];
		char layer[DXF_MAX_CHARS], l_type[DXF_MAX_CHARS];
		char t_style[DXF_MAX_CHARS];
		int color = 0, paper = 0, lw = -2;
		int t_alin_v = 0, t_alin_h = 0;
		
		
		/* clear the strings */
		txt[0] = 0;
		tag[0] = 0;
		layer[0] = 0;
		l_type[0] = 0;
		t_style[0] = 0;
		
		dxf_node *current = NULL;
		if (attdef->obj.content){
			current = attdef->obj.content->next;
		}
		while (current){
			if (current->type == DXF_ATTR){ /* scan parameters */
				switch (current->value.group){
					case 1:
						strcpy(txt, current->value.s_data);
						break;
					case 2:
						strcpy(tag, current->value.s_data);
						break;
					case 7:
						strcpy(t_style, current->value.s_data);
						break;
					case 6:
						strcpy(l_type, current->value.s_data);
						break;
					case 8:
						strcpy(layer, current->value.s_data);
						break;
					case 10:
						x1 = current->value.d_data;
						break;
					case 11:
						x2 = current->value.d_data;
						break;
					case 20:
						y1 = current->value.d_data;
						break;
					case 21:
						y2 = current->value.d_data;
						break;
					case 30:
						z1 = current->value.d_data;
						break;
					case 31:
						z2 = current->value.d_data;
						break;
					case 40:
						h = current->value.d_data;
						break;
					case 41:
						t_scale_x = current->value.d_data;
						break;
					case 50:
						rot = current->value.d_data;
						break;
					case 62:
						color = current->value.i_data;
						break;
					case 67:
						paper = current->value.i_data;
						break;
					case 72:
						t_alin_h = current->value.i_data;
						break;
					case 74:
						t_alin_v = current->value.i_data;
						break;
					case 210:
						extru_x = current->value.d_data;
						break;
					case 220:
						extru_y = current->value.d_data;
						break;
					case 230:
						extru_z = current->value.d_data;
						break;
					case 370:
						lw = current->value.i_data;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		
		dxf_node * attrib = dxf_new_attrib (x1 + x0, y1 + y0, z1 + z0, h,
			txt, tag, color, layer, l_type, lw, paper);
		
		dxf_attr_change(attrib, 11, (void *)(double[]){x2 + x0});
		dxf_attr_change(attrib, 21, (void *)(double[]){y2 + y0});
		dxf_attr_change(attrib, 31, (void *)(double[]){z2 + z0});
		dxf_attr_change(attrib, 50, (void *)&rot);
		dxf_attr_change(attrib, 7, (void *)t_style);
		dxf_attr_change(attrib, 41, (void *)&t_scale_x);
		dxf_attr_change(attrib, 72, (void *)&t_alin_h);
		dxf_attr_change(attrib, 74, (void *)&t_alin_v);
		dxf_attr_change(attrib, 210, (void *)&extru_x);
		dxf_attr_change(attrib, 220, (void *)&extru_y);
		dxf_attr_change(attrib, 230, (void *)&extru_z);
		
		return attrib;
	}

	return NULL;
}

dxf_node * dxf_new_seqend (char *layer){
	/* create a new SEQEND entity */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	int ok = 1;
	dxf_node * seqend = dxf_obj_new ("SEQEND");
	
	ok &= dxf_attr_append(seqend, 5, (void *) handle);
	ok &= dxf_attr_append(seqend, 100, (void *) dxf_class);
	ok &= dxf_attr_append(seqend, 8, (void *) layer);
	
	if(ok){
		return seqend;
	}

	return NULL;
}

dxf_node * dxf_new_endblk (char *layer, char *owner){
	/* create a new ENDBLK entity */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbBlockEnd";
	int ok = 1;
	dxf_node * new_endblk = dxf_obj_new ("ENDBLK");
	
	ok &= dxf_attr_append(new_endblk, 5, (void *) handle);
	ok &= dxf_attr_append(new_endblk, 330, (void *) owner);
	ok &= dxf_attr_append(new_endblk, 100, (void *) dxf_class);
	ok &= dxf_attr_append(new_endblk, 8, (void *) layer);
	ok &= dxf_attr_append(new_endblk, 100, (void *) dxf_subclass);
	
	if(ok){
		return new_endblk;
	}

	return NULL;
}

dxf_node * dxf_new_begblk (char *name, char *layer, char *owner){
	/* create a new begin BLOCK entity */
	const char *handle = "0";
	const char *str_zero = "";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbBlockBegin";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0;
	dxf_node * blk = dxf_obj_new ("BLOCK");
	
	ok &= dxf_attr_append(blk, 5, (void *) handle);
	ok &= dxf_attr_append(blk, 330, (void *) owner);
	ok &= dxf_attr_append(blk, 100, (void *) dxf_class);
	ok &= dxf_attr_append(blk, 8, (void *) layer);
	ok &= dxf_attr_append(blk, 100, (void *) dxf_subclass);
	ok &= dxf_attr_append(blk, 2, (void *) name);
	ok &= dxf_attr_append(blk, 70, (void *) &int_zero);
	ok &= dxf_attr_append(blk, 10, (void *) &d_zero);
	ok &= dxf_attr_append(blk, 20, (void *) &d_zero);
	ok &= dxf_attr_append(blk, 30, (void *) &d_zero);
	ok &= dxf_attr_append(blk, 3, (void *) name);
	ok &= dxf_attr_append(blk, 1, (void *) str_zero);
	
	if(ok){
		return blk;
	}

	return NULL;
}

dxf_node * dxf_new_blkrec (char *name){
	/* create a new BLOCK_RECORD */
	const char *handle = "0";
	const char *dxf_class = "AcDbSymbolTableRecord";
	const char *dxf_subclass = "AcDbBlockTableRecord";
	int ok = 1, int_zero = 0;
	dxf_node * blk = dxf_obj_new ("BLOCK_RECORD");
	
	ok &= dxf_attr_append(blk, 5, (void *) handle);
	ok &= dxf_attr_append(blk, 100, (void *) dxf_class);
	ok &= dxf_attr_append(blk, 100, (void *) dxf_subclass);
	ok &= dxf_attr_append(blk, 2, (void *) name);
	ok &= dxf_attr_append(blk, 70, (void *) &int_zero);
	
	if(ok){
		return blk;
	}

	return NULL;
}


int dxf_block_append(dxf_node *blk, dxf_node *obj){
	int ok = 0;
	if((blk) && (obj)){
		if ((blk->type != DXF_ENT) || (obj->type != DXF_ENT)){
			return 0; /*Fail - wrong types */
		}
		if (strcmp(blk->obj.name, "BLOCK") != 0){
			return 0; /*Fail - wrong block */
		}
		if (strcmp(obj->obj.name, "BLOCK") == 0){
			return 0; /*Fail - try to nested blocks*/
		}
		
		/*try to find ENDBLK*/
		dxf_node *endblk = dxf_find_obj2(blk, "ENDBLK");
		if (endblk){
			/* append object between endblk and rest of blk contents */
			obj->prev = endblk->prev;
			if (obj->prev){
				obj->prev->next = obj;
			}
			obj->next = endblk;
			endblk->prev = obj;
		}
		else { /* ENDBLK not found */
			/* in this case, simply append object in block */
			dxf_obj_append(blk, obj);
		}
	}
	return ok;
}

int dxf_new_block(dxf_drawing *drawing, char *name, char *layer, list_node *list, struct do_list *list_do){
	int ok = 0;
	
	if ((drawing) && (name)){
		dxf_node *blkrec = NULL, *blk = NULL, *endblk = NULL, *handle = NULL;
		dxf_node *obj, *new_ent;
		
		/* verify if block not exist */
		if (dxf_find_obj_descr2(drawing->blks, "BLOCK", name) == NULL){
			double max_x = 0.0, max_y = 0.0;
			double min_x = 0.0, min_y = 0.0;
			int init_ext = 0;
			
			/* create BLOCK_RECORD table entry*/
			blkrec = dxf_new_blkrec (name);
			ok = ent_handle(drawing, blkrec);
			if (ok) handle = dxf_find_attr2(blkrec, 5); ok = 0;
			
			/* begin block */
			if (handle) blk = dxf_new_begblk (name, layer, (char *)handle->value.s_data);
			/* get a handle */
			ok = ent_handle(drawing, blk);
			/* use the handle to owning the ENDBLK ent */
			if (ok) handle = dxf_find_attr2(blk, 5); ok = 0;
			if (handle) endblk = dxf_new_endblk (layer, (char *)handle->value.s_data);
			
			if (list != NULL){ /* append list of objects in block*/
				list_node *current = list->next;
				/* first get the list coordinates extention */
				while (current != NULL){
					if (current->data){
						obj = (dxf_node *)current->data;
						if (obj->type == DXF_ENT){ /* DXF entity  */
							graph_list_ext(obj->obj.graphics, &init_ext, &min_x, &min_y, &max_x, &max_y);
						}
					}
					current = current->next;
				}
				/*then copy the entities of list and apply offset in their coordinates*/
				current = list->next;
				while (current != NULL){
					if (current->data){
						obj = (dxf_node *)current->data;
						if (obj->type == DXF_ENT){ /* DXF entity  */
							new_ent = dxf_ent_copy(obj, 0);
							ent_handle(drawing, new_ent);
							dxf_edit_move(new_ent, -min_x, -min_y, 0.0);
							dxf_obj_append(blk, new_ent);
							
						}
					}
					current = current->next;
				}
			}
			
			
			/* end the block*/
			if (endblk) ok = ent_handle(drawing, endblk);
			if (ok) ok = dxf_obj_append(blk, endblk);
			
			/*attach to blocks section*/
			if (ok) do_add_entry(list_do, "NEW BLOCK"); /* undo/redo list*/
			if (ok) ok = dxf_obj_append(drawing->blks_rec, blkrec);
			if (ok) do_add_item(list_do->current, NULL, blkrec);/* undo/redo list*/
			if (ok) ok = dxf_obj_append(drawing->blks, blk);
			if (ok) do_add_item(list_do->current, NULL, blk);/* undo/redo list*/
			
		}
	}
	
	return ok;
}

int dxf_new_block2(dxf_drawing *drawing, char *name, char *mark, char *layer, list_node *list, struct do_list *list_do){
	int ok = 0;
	
	if ((drawing) && (name)){
		dxf_node *blkrec = NULL, *blk = NULL, *endblk = NULL, *handle = NULL;
		dxf_node *obj, *new_ent, *text, *attdef;
		
		/* verify if block not exist */
		if (dxf_find_obj_descr2(drawing->blks, "BLOCK", name) == NULL){
			double max_x = 0.0, max_y = 0.0;
			double min_x = 0.0, min_y = 0.0;
			int init_ext = 0;
			char txt[DXF_MAX_CHARS], tag[DXF_MAX_CHARS];
			txt[0] = 0;
			tag[0] = 0;
			
			int mark_len = strlen(mark);
			
			/* create BLOCK_RECORD table entry*/
			blkrec = dxf_new_blkrec (name);
			ok = ent_handle(drawing, blkrec);
			if (ok) handle = dxf_find_attr2(blkrec, 5); ok = 0;
			
			/* begin block */
			if (handle) blk = dxf_new_begblk (name, layer, (char *)handle->value.s_data);
			/* get a handle */
			ok = ent_handle(drawing, blk);
			/* use the handle to owning the ENDBLK ent */
			if (ok) handle = dxf_find_attr2(blk, 5); ok = 0;
			if (handle) endblk = dxf_new_endblk (layer, (char *)handle->value.s_data);
			
			if (list != NULL){ /* append list of objects in block*/
				list_node *current = list->next;
				/* first get the list coordinates extention */
				while (current != NULL){
					if (current->data){
						obj = (dxf_node *)current->data;
						if (obj->type == DXF_ENT){ /* DXF entity  */
							graph_list_ext(obj->obj.graphics, &init_ext, &min_x, &min_y, &max_x, &max_y);
						}
					}
					current = current->next;
				}
				/*then copy the entities of list and apply offset in their coordinates*/
				current = list->next;
				while (current != NULL){
					if (current->data){
						obj = (dxf_node *)current->data;
						if (obj->type == DXF_ENT){ /* DXF entity  */
							if(strcmp(obj->obj.name, "TEXT") == 0){
								text = dxf_find_attr2(obj, 1);
								if (text){
									if (strncmp (text->value.s_data, mark, mark_len) == 0){
										current = current->next;
										continue;
									}
								}
							}
							new_ent = dxf_ent_copy(obj, 0);
							ent_handle(drawing, new_ent);
							dxf_edit_move(new_ent, -min_x, -min_y, 0.0);
							dxf_obj_append(blk, new_ent);
						}
					}
					current = current->next;
				}
				/*then transform marked text entities to attdef*/
				current = list->next;
				while (current != NULL){
					if (current->data){
						obj = (dxf_node *)current->data;
						if (obj->type == DXF_ENT){ /* DXF entity  */
							if(strcmp(obj->obj.name, "TEXT") == 0){
								text = dxf_find_attr2(obj, 1);
								if (text){
									if (strncmp (text->value.s_data, mark, mark_len) == 0){
										new_ent = dxf_attdef_cpy (obj, text->value.s_data + mark_len, min_x, min_y, 0.0);
										ent_handle(drawing, new_ent);
										dxf_obj_append(blk, new_ent);
									}
								}
							}
						}
					}
					current = current->next;
				}
			}
			
			
			/* end the block*/
			if (endblk) ok = ent_handle(drawing, endblk);
			if (ok) ok = dxf_obj_append(blk, endblk);
			
			/*attach to blocks section*/
			if (ok) do_add_entry(list_do, "NEW BLOCK"); /* undo/redo list*/
			if (ok) ok = dxf_obj_append(drawing->blks_rec, blkrec);
			if (ok) do_add_item(list_do->current, NULL, blkrec);/* undo/redo list*/
			if (ok) ok = dxf_obj_append(drawing->blks, blk);
			if (ok) do_add_item(list_do->current, NULL, blk);/* undo/redo list*/
			
		}
	}
	
	return ok;
}

dxf_node * dxf_new_insert (char *name, double x0, double y0, double z0,
int color, char *layer, char *ltype, int lw, int paper){
	/* create a new INSERT */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbBlockReference";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0, d_one = 1.0;
	dxf_node * ins = dxf_obj_new ("INSERT");
	
	ok &= dxf_attr_append(ins, 5, (void *) handle);
	ok &= dxf_attr_append(ins, 100, (void *) dxf_class);
	ok &= dxf_attr_append(ins, 67, (void *) &paper);
	ok &= dxf_attr_append(ins, 8, (void *) layer);
	ok &= dxf_attr_append(ins, 6, (void *) ltype);
	ok &= dxf_attr_append(ins, 62, (void *) &color);
	ok &= dxf_attr_append(ins, 370, (void *) &lw);
	
	ok &= dxf_attr_append(ins, 100, (void *) dxf_subclass);
	ok &= dxf_attr_append(ins, 66, (void *) &int_zero);
	ok &= dxf_attr_append(ins, 2, (void *) name);
	
	/* insert point */
	ok &= dxf_attr_append(ins, 10, (void *) &x0);
	ok &= dxf_attr_append(ins, 20, (void *) &y0);
	ok &= dxf_attr_append(ins, 30, (void *) &z0);
	
	/* scales */
	ok &= dxf_attr_append(ins, 41, (void *) &d_one);
	ok &= dxf_attr_append(ins, 42, (void *) &d_one);
	ok &= dxf_attr_append(ins, 43, (void *) &d_one);
	
	/* rotation */
	ok &= dxf_attr_append(ins, 50, (void *) &d_zero);
	
	
	if(ok){
		return ins;
	}

	return NULL;
}

int dxf_insert_append(dxf_drawing *drawing, dxf_node *ins, dxf_node *obj){
	int ok = 0;
	if((ins) && (obj)){
		if ((ins->type != DXF_ENT) || (obj->type != DXF_ENT)){
			return 0; /*Fail - wrong types */
		}
		if (strcmp(ins->obj.name, "INSERT") != 0){
			return 0; /*Fail - wrong insert */
		}
		if (strcmp(obj->obj.name, "ATTRIB") != 0){
			return 0; /*Fail - only acept ATTRIB ents*/
		}
		
		/* set ents follow flag*/
		dxf_attr_change(ins, 66, (int[]){1});
		
		/*try to find SEQEND*/
		dxf_node *seqend = dxf_find_obj2(ins, "SEQEND");
		if (seqend){
			/* append object between seqend and rest of insert contents */
			obj->prev = seqend->prev;
			if (obj->prev){
				obj->prev->next = obj;
			}
			obj->next = seqend;
			seqend->prev = obj;
		}
		else { /* SEQEND not found */
			/* in this case, append object in insert, create and append a valid SEQEND */
			dxf_obj_append(ins, obj);
			seqend = dxf_new_seqend ("0");
			ent_handle(drawing, seqend);
			dxf_obj_append(ins, seqend);
		}
		ok = 1;
	}
	return ok;
}

int dxf_new_layer (dxf_drawing *drawing, char *name, int color, char *ltype){
	
	if (!drawing) 
		return 0; /* error -  not drawing */
	
	if ((drawing->t_layer == NULL) || (drawing->main_struct == NULL)) 
		return 0; /* error -  not main structure */
	
	char name_cpy[DXF_MAX_CHARS], *new_name;
	strncpy(name_cpy, name, DXF_MAX_CHARS);
	new_name = trimwhitespace(name_cpy);
	
	if (strlen(new_name) == 0) return 0; /* error -  no name */
	
	/* verify if not exists */
	if (dxf_find_obj_descr2(drawing->t_layer, "LAYER", new_name) != NULL) 
		return 0; /* error -  exists layer with same name */
	
	if ((abs(color) > 255) || (color == 0)) color = 7;
	
	const char *handle = "0";
	const char *dxf_class = "AcDbSymbolTableRecord";
	const char *dxf_subclass = "AcDbLayerTableRecord";
	int int_zero = 0, ok = 0;
	
	/* create a new LAYER */
	dxf_node * lay = dxf_obj_new ("LAYER");
	
	if (lay) {
		ok = 1;
		ok &= dxf_attr_append(lay, 5, (void *) handle);
		ok &= dxf_attr_append(lay, 100, (void *) dxf_class);
		ok &= dxf_attr_append(lay, 100, (void *) dxf_subclass);
		ok &= dxf_attr_append(lay, 2, (void *) new_name);
		ok &= dxf_attr_append(lay, 70, (void *) &int_zero);
		ok &= dxf_attr_append(lay, 62, (void *) &color);
		ok &= dxf_attr_append(lay, 6, (void *) ltype);
		ok &= dxf_attr_append(lay, 370, (void *) &int_zero);
		ok &= dxf_attr_append(lay, 390, (void *) handle);
		
		/* get current handle and increment the handle seed*/
		ok &= ent_handle(drawing, lay);
		
		/* append the layer to correpondent table */
		dxf_append(drawing->t_layer, lay);
		
		/* update the layers in drawing  */
		dxf_layer_assemb (drawing);
	}
	
	return ok;
}

int dxf_new_tstyle (dxf_drawing *drawing, char *name){
	
	if (!drawing) 
		return 0; /* error -  not drawing */
	
	if ((drawing->t_style == NULL) || (drawing->main_struct == NULL)) 
		return 0; /* error -  not main structure */
	
	char name_cpy[DXF_MAX_CHARS], *new_name;
	strncpy(name_cpy, name, DXF_MAX_CHARS);
	new_name = trimwhitespace(name_cpy);
	
	if (strlen(new_name) == 0) return 0; /* error -  no name */
	
	/* verify if not exists */
	if (dxf_find_obj_descr2(drawing->t_style, "STYLE", new_name) != NULL) 
		return 0; /* error -  exists style with same name */
		
	const char *handle = "0";
	const char *dxf_class = "AcDbSymbolTableRecord";
	const char *dxf_subclass = "AcDbTextStyleTableRecord";
	const char *font = "TXT.SHX";
	const char *blank = "";
	int int_zero = 0, ok = 0;
	double d_zero = 0.0, d_one = 1.0;
	
	/* create a new STYLE */
	dxf_node * sty = dxf_obj_new ("STYLE");
	
	if (sty) {
		ok = 1;
		ok &= dxf_attr_append(sty, 5, (void *) handle);
		ok &= dxf_attr_append(sty, 100, (void *) dxf_class);
		ok &= dxf_attr_append(sty, 100, (void *) dxf_subclass);
		ok &= dxf_attr_append(sty, 2, (void *) new_name);
		ok &= dxf_attr_append(sty, 70, (void *) &int_zero);
		ok &= dxf_attr_append(sty, 40, (void *) &d_zero);
		ok &= dxf_attr_append(sty, 41, (void *) &d_one);
		ok &= dxf_attr_append(sty, 50, (void *) &d_zero);
		ok &= dxf_attr_append(sty, 71, (void *) &int_zero);
		ok &= dxf_attr_append(sty, 42, (void *) &d_one);
		ok &= dxf_attr_append(sty, 3, (void *) font);
		ok &= dxf_attr_append(sty, 4, (void *) blank);
		
		/* get current handle and increment the handle seed*/
		ok &= ent_handle(drawing, sty);
		
		/* append the style to correpondent table */
		dxf_append(drawing->t_style, sty);
		
		/* update the styles in drawing  */
		dxf_tstyles_assemb (drawing);
	}
	return ok;
}

dxf_node * dxf_new_hatch (struct h_pattern *pattern, graph_obj *bound,
int solid, int assoc,
int style, /* 0 = normal odd, 1 = outer, 2 = ignore */
int type, /* 0 = user, 1 = predefined, 2 =custom */
double rot, double scale,
int color, char *layer, char *ltype, int lw, int paper){
	/* create a new hatch */
	const char *handle = "0";
	const char *dxf_class = "AcDbEntity";
	const char *dxf_subclass = "AcDbHatch";
	int ok = 1, int_zero = 0;
	double d_zero = 0.0, d_one = 1.0;
	dxf_node * hatch = dxf_obj_new ("HATCH");
	
	if ((!pattern) || (!bound)) return NULL;
	
	ok &= dxf_attr_append(hatch, 5, (void *) handle);
	ok &= dxf_attr_append(hatch, 100, (void *) dxf_class);
	ok &= dxf_attr_append(hatch, 67, (void *) &paper);
	ok &= dxf_attr_append(hatch, 8, (void *) layer);
	ok &= dxf_attr_append(hatch, 6, (void *) ltype);
	ok &= dxf_attr_append(hatch, 62, (void *) &color);
	ok &= dxf_attr_append(hatch, 370, (void *) &lw);
	
	ok &= dxf_attr_append(hatch, 100, (void *) dxf_subclass);
	
	/* elevation point */
	ok &= dxf_attr_append(hatch, 10, (void *) &d_zero); /*always zero*/
	ok &= dxf_attr_append(hatch, 20, (void *) &d_zero); /*always zero*/
	ok &= dxf_attr_append(hatch, 30, (void *) &d_zero); /* z coordinate is the elevation*/
	
	ok &= dxf_attr_append(hatch, 210, (void *) &d_zero);
	ok &= dxf_attr_append(hatch, 220, (void *) &d_zero);
	ok &= dxf_attr_append(hatch, 230, (void *) &d_one);
	
	/* pattern name */
	ok &= dxf_attr_append(hatch, 2, (void *) pattern->name);
	
	/* fill flag */
	ok &= dxf_attr_append(hatch, 70, (void *) &solid);
	/* associativity flag */
	ok &= dxf_attr_append(hatch, 71, (void *) &assoc);
	
	/* number of boundary loops */
	ok &= dxf_attr_append(hatch, 91, (void *) (int[]){1});
	
	/*============== bondaries =============*/
	/* boundary type */
	ok &= dxf_attr_append(hatch, 92, (void *) (int[]){0}); /* not polyline*/
	
	int num_edges = 0;
	line_node *curr_graph = bound->list->next;
	while(curr_graph){ /*get number of edges */
		num_edges++;
		curr_graph = curr_graph->next; /* go to next */
	}
	
	ok &= dxf_attr_append(hatch, 93, (void *) &num_edges);
	
	curr_graph = bound->list->next;
	while(curr_graph){ /*sweep the list content */
		ok &= dxf_attr_append(hatch, 72, (void *) (int[]){1}); /* 1 = Line edge */
		ok &= dxf_attr_append(hatch, 10, (void *) &(curr_graph->x0));
		ok &= dxf_attr_append(hatch, 20, (void *) &(curr_graph->y0));
		ok &= dxf_attr_append(hatch, 11, (void *) &(curr_graph->x1));
		ok &= dxf_attr_append(hatch, 21, (void *) &(curr_graph->y1));
		
		curr_graph = curr_graph->next; /* go to next */
	}
	
	/* number of source boundary objects - ?? */
	ok &= dxf_attr_append(hatch, 97, (void *) &int_zero);
	
	/*===================================*/
	
	/* hatch style */
	ok &= dxf_attr_append(hatch, 75, (void *) &style);
	/* pattern type */
	ok &= dxf_attr_append(hatch, 76, (void *) &type);
	
	if (!solid){
		/* pattern rotation */
		ok &= dxf_attr_append(hatch, 52, (void *) &rot);
		/* pattern scale */
		ok &= dxf_attr_append(hatch, 41, (void *) &scale);
		/* double pattern - ?? */
		ok &= dxf_attr_append(hatch, 77, (void *) &int_zero);
		
		/* number of definition lines */
		ok &= dxf_attr_append(hatch, 78, (void *) &(pattern->num_lines));
		
		/*============== lines =============*/
		struct hatch_line *curr_l = pattern->lines;
		while (curr_l){
			double ang = fmod(curr_l->ang + rot, 360.0);
			double cosine = cos(ang * M_PI/180);
			double sine = sin(ang * M_PI/180);
			double dx = scale * (cosine*curr_l->dx - sine*curr_l->dy);
			double dy = scale * (sine*curr_l->dx + cosine*curr_l->dy);
			cosine = cos(rot * M_PI/180);
			sine = sin(rot * M_PI/180);
			double ox = scale * (cosine*curr_l->ox - sine*curr_l->oy);
			double oy = scale * (sine*curr_l->ox + cosine*curr_l->oy);
			int i;
			double dash[20];
			
			for (i = 0; i < curr_l->num_dash; i++){
				dash[i] = scale * curr_l->dash[i];
			}
			
			 /* line angle */
			ok &= dxf_attr_append(hatch, 53, (void *) &ang);
			/* base point */
			ok &= dxf_attr_append(hatch, 43, (void *) &ox);
			ok &= dxf_attr_append(hatch, 44, (void *) &oy);
			/*offset*/
			ok &= dxf_attr_append(hatch, 45, (void *) &dx);
			ok &= dxf_attr_append(hatch, 46, (void *) &dy);
			/*number of dash elements*/
			ok &= dxf_attr_append(hatch, 79, (void *) &(curr_l->num_dash));
			
			for (i = 0; i < curr_l->num_dash; i++){
				ok &= dxf_attr_append(hatch, 49, (void *) &(dash[i]));
			}
			
			curr_l = curr_l->next;
		}
	
		/*===================================*/
		
	}
	
	/* number seed points - ?? */
	ok &= dxf_attr_append(hatch, 98, (void *) &int_zero);
	
	if(ok){
		return hatch;
	}

	return NULL;
}


int dxf_edit_move2 (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z){
	/* move the object relactive to offset distances */
	if (obj){
		dxf_node *current;
		int i, j;
		for (i = 0; i < 8; i++){ /* sweep in range of DXF points (10-17, 20-27, 30-37) */
			for (j = 0; current = dxf_find_attr_i(obj, 10 + i, j); j++){
				current->value.d_data += ofs_x;
			}
			for (j = 0; current = dxf_find_attr_i(obj, 20 + i, j); j++){
				current->value.d_data += ofs_y;
			}
			for (j = 0; current = dxf_find_attr_i(obj, 30 + i, j); j++){
				current->value.d_data += ofs_z;
			}
		}
		return 1;
	}
	return 0;
}

int dxf_edit_move (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	if (obj){ 
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				ent_type =  dxf_ident_ent_type (obj);
				current = obj->obj.content->next;
				prev = current;
			}
		}
	}

	while (current){
		ret = 1;
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				current = current->obj.content->next;
				prev = current;
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			if (current->value.group == 10){ 
				current->value.d_data += ofs_x;
			}
			if (current->value.group == 20){ 
				current->value.d_data += ofs_y;
			}
			if (current->value.group == 30){ 
				current->value.d_data += ofs_z;
			}
			
			switch (ent_type){
				case DXF_LINE:
					if (current->value.group == 11){ 
						current->value.d_data += ofs_x;
					}
					if (current->value.group == 21){ 
						current->value.d_data += ofs_y;
					}
					if (current->value.group == 31){ 
						current->value.d_data += ofs_z;
					}
					break;
				case DXF_TEXT:
					if (current->value.group == 11){ 
						current->value.d_data += ofs_x;
					}
					if (current->value.group == 21){ 
						current->value.d_data += ofs_y;
					}
					if (current->value.group == 31){ 
						current->value.d_data += ofs_z;
					}
					break;
				default:
					break;
			}
		}
		
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev == obj){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	return ret;
}

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z){
	/* move the object and its childrens,  relactive to offset distances */
	dxf_node *current = NULL;
	dxf_node *prev = NULL;
	int ret = 0;
	enum dxf_graph ent_type = DXF_NONE;
	
	if (obj){ 
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				ent_type =  dxf_ident_ent_type (obj);
				current = obj->obj.content->next;
				prev = current;
			}
		}
	}

	while (current){
		ret = 1;
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				current = current->obj.content->next;
				prev = current;
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			if ((current->value.group >= 10) && (current->value.group < 19)){ 
				current->value.d_data *= scale_x;
			}
			if ((current->value.group >= 20) && (current->value.group < 29)){ 
				current->value.d_data *= scale_y;
			}
			if ((current->value.group >= 30) && (current->value.group < 38)){ 
				current->value.d_data *= scale_z;
			}
			
			switch (ent_type){
				case DXF_CIRCLE:
					if (current->value.group == 40) { 
						current->value.d_data *= scale_x;
					}
					break;
				case DXF_TEXT:
					if (current->value.group == 40) { 
						current->value.d_data *= scale_x;
					}
					break;
				default:
					break;
			}
		}
		
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev == obj){ /* stop the search if back on initial entity */
				//printf("para\n");
				current = NULL;
				break;
			}
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	return ret;
}

int ent_handle(dxf_drawing *drawing, dxf_node *element){
	int ok = 0;
	if (drawing && element){
		if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
			/* get current handle and increment the handle seed*/
			long int handle = 0;
			char hdl_str[DXF_MAX_CHARS];
			
			if (drawing->hand_seed){
				/* get the last handle value and convert to integer */
				handle = strtol(drawing->hand_seed->value.s_data, NULL, 16);
				/* convert back to hexadecimal string, to write in element */
				snprintf(hdl_str, DXF_MAX_CHARS, "%X", handle);
				/* increment value of seed and write back */
				snprintf(drawing->hand_seed->value.s_data, DXF_MAX_CHARS, "%X", handle + 1);
			}
			
			/* change element handle */
			if (handle){
				ok = dxf_attr_change(element, 5, hdl_str);
			}
		}
	}
	
	return ok;
}

void drawing_ent_append(dxf_drawing *drawing, dxf_node *element){
	if (drawing && element){
		if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
			/* get current handle and increment the handle seed*/
			ent_handle(drawing, element);
			
			/*  append drawing entities's list */
			element->master = drawing->ents;
			element->prev = drawing->ents->end;
			if (drawing->ents->end){
				drawing->ents->end->next = element;
			}
			element->next = NULL; /* append at end of list */
			drawing->ents->end = element;
		}
	}
}