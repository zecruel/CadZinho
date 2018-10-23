#include "list.h"

void * list_mem_pool(enum list_pool_action action, int idx){
	
	static list_pool_slot mem_pool[LIST_NUM_POOL], line[LIST_NUM_POOL];
	int i;
	
	void *ret_ptr = NULL;
	
	if ((idx >= 0) && (idx < LIST_NUM_POOL)){ /* check if index is valid */
		
		/* initialize the pool, the first allocation */
		if (mem_pool[idx].size < 1){
			mem_pool[idx].pool[0] = malloc(LIST_PAGE * sizeof(list_node));
			if (mem_pool[idx].pool){
				mem_pool[idx].size = 1;
				//printf("Init mem_pool\n");
			}
		}
		
		/* if current page is full */
		if ((mem_pool[idx].pos >= LIST_PAGE) && (mem_pool[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (mem_pool[idx].page < mem_pool[idx].size - 1){
				mem_pool[idx].page++;
				mem_pool[idx].pos = 0;
				//printf("change mem_pool page\n");
			}
			/* or then allocatte a new page */
			else if(mem_pool[idx].page < LIST_POOL_PAGES-1){
				mem_pool[idx].pool[mem_pool[idx].page + 1] = malloc(LIST_PAGE * sizeof(list_node));
				if (mem_pool[idx].pool[mem_pool[idx].page + 1]){
					mem_pool[idx].page++;
					mem_pool[idx].size ++;
					mem_pool[idx].pos = 0;
					//printf("Realloc mem_pool\n");
				}
			}
		}
		
		ret_ptr = NULL;
		
		if ((mem_pool[idx].pool[mem_pool[idx].page] != NULL)){
			switch (action){
				case ADD_LIST:
					if (mem_pool[idx].pos < LIST_PAGE){
						ret_ptr = &(((list_node *)mem_pool[idx].pool[mem_pool[idx].page])[mem_pool[idx].pos]);
						mem_pool[idx].pos++;
					}
					break;
				case ZERO_LIST:
					mem_pool[idx].pos = 0;
					mem_pool[idx].page = 0;
					break;
				case FREE_LIST:
					for (i = 0; i < mem_pool[idx].size; i++){
						free(mem_pool[idx].pool[i]);
						mem_pool[idx].pool[i] = NULL;
					}
					mem_pool[idx].pos = 0;
					mem_pool[idx].page = 0;
					mem_pool[idx].size = 0;
					break;
			}
		}
	}
	return ret_ptr;
}

list_node * list_new (void *data, int pool){
	//char *new_name = NULL;
	
	/* create a new LIST Object */
	list_node *new_obj = (list_node *)list_mem_pool(ADD_LIST, pool);
	if (new_obj){
		new_obj->next = NULL;
		new_obj->prev = NULL;
		new_obj->end = new_obj;
		new_obj->data = data;
	}
	return new_obj;
}

int list_push(list_node * list, list_node * new_node){
	
	if (list && new_node){
		new_node->prev = list->end;
		new_node->next = NULL;
		new_node->end = new_node;
		list->end = new_node;
		if (new_node->prev){
			new_node->prev->next = new_node;
		}
		
		return 1; /* return success */
	}
	return 0; /* return fail */
}

list_node *list_pop(list_node * list){
	list_node *ret_node;
	if (list){
		ret_node = list->end;
		if (ret_node){
			if (ret_node->prev){
				list->end = ret_node->prev;
				ret_node->prev->next = NULL;
				ret_node->prev = NULL;
				ret_node->next = NULL;
				return ret_node; /* return success */
			}
		}
	}
	return NULL; /* return fail */
}

list_node *list_find_data(list_node * list, void *data){
	list_node *current = NULL;
	
	if (list && data){
		current = list->next;
		while (current != NULL){
			if (current->data == data){
				return current; /* return success */
			}
			current = current->next;
		}
	}
	return NULL; /* return fail */
}

int list_clear(list_node * list){
	
	if (list){
		list->end = list;
		list->next = NULL;
		
		return 1; /* return success */
	}
	return 0; /* return fail */
}

int list_merge(list_node * dest, list_node * src){
	
	if (!dest || !src) return 0; /* return fail */
	
	list_node *next = src->next;
	list_node *end = dest->end;
	
	if (next && end){
		next->prev = end;
		end->next = next;
		dest->end = src->end;
		
		return 1; /* return success */
	}
	
	return 0; /* return fail */
}