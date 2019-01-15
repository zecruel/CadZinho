#ifndef _LIST_LIB
#define _LIST_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_NUM_POOL 5
#define LIST_PAGE 500000
#define LIST_POOL_PAGES 1000

enum list_pool_action{
	ADD_LIST,
	ZERO_LIST,
	FREE_LIST
};

struct List_pool_slot{
	void *pool[LIST_POOL_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};
typedef struct List_pool_slot list_pool_slot;

struct List_node{
	struct List_node *next;    /* next node (double linked list) */
	struct List_node *prev;    /* previous node (double linked list) */
	struct List_node *end; /*last object in list*/
	
	void *data;
};
typedef struct List_node list_node;

void * list_mem_pool(enum list_pool_action action, int idx);

list_node * list_new (void *data, int pool);

int list_push(list_node * list, list_node * new_node);

list_node *list_pop(list_node * list);

list_node *list_find_data(list_node * list, void *data);

list_node *list_get_idx(list_node * list, int idx);

int list_clear(list_node * list);

int list_merge(list_node * dest, list_node * src);

#endif