#include "dxf.h"
#include "dxf.h"
#include <math.h>
#define DXF_NUM_POOL 5
#define DXF_PAGE 500000
#include "font.h"
#include "dxf_image.h"

char const* strpool_cstr2( strpool_t const* pool, STRPOOL_U64 handle ){
  static char const* no_ret = "";
  char const* ret = strpool_cstr( pool, handle );
  
  if (ret) return ret;
  else return no_ret;
}

void * dxf_mem_pool(enum dxf_pool_action action, int idx){
	
	static dxf_pool_slot mem_pool[DXF_NUM_POOL];
	int i;
	
	void *ret_ptr = NULL;
	
	if ((idx >= 0) && (idx < DXF_NUM_POOL)){ /* check if index is valid */
		
		/* initialize the pool, the first allocation */
		if (mem_pool[idx].size < 1){
			mem_pool[idx].pool[0] = malloc(DXF_PAGE * sizeof(dxf_node));
			if (mem_pool[idx].pool){
				mem_pool[idx].size = 1;
			}
		}
		
		/* if current page is full */
		if ((mem_pool[idx].pos >= DXF_PAGE) && (mem_pool[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (mem_pool[idx].page < mem_pool[idx].size - 1){
				mem_pool[idx].page++;
				mem_pool[idx].pos = 0;
			}
			/* or then allocatte a new page */
			else if(mem_pool[idx].page < DXF_POOL_PAGES-1){
				mem_pool[idx].pool[mem_pool[idx].page + 1] = malloc(DXF_PAGE * sizeof(dxf_node));
				if (mem_pool[idx].pool[mem_pool[idx].page + 1]){
					mem_pool[idx].page++;
					mem_pool[idx].size ++;
					mem_pool[idx].pos = 0;
				}
			}
		}
		
		ret_ptr = NULL;
		
		if ((mem_pool[idx].pool[mem_pool[idx].page] != NULL)){
			switch (action){
				case ADD_DXF:
					if (mem_pool[idx].pos < DXF_PAGE){
						ret_ptr = &(((dxf_node *)mem_pool[idx].pool[mem_pool[idx].page])[mem_pool[idx].pos]);
						mem_pool[idx].pos++;
					}
					break;
				case ZERO_DXF:
					mem_pool[idx].pos = 0;
					mem_pool[idx].page = 0;
					break;
				case FREE_DXF:
					for (i = 0; i < mem_pool[idx].size; i++){
						free(mem_pool[idx].pool[i]);
						//dxf_ent_clear (mem_pool[idx].pool[i]);
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

void dxf_ent_print2 (dxf_node *ent){ /* print the entity structure */
	/* this function is non recursive */
	int i;
	int indent = 0;
	dxf_node *current, *prev;
	
	current = ent;
	while (current){
		prev = current;
		if (current->type == DXF_ENT){ /* DXF entity */
			
      for (i=0; i<indent; i++){ /* print the indentation spaces */
        printf("    ");
      }
      /* print the string of entity's name */
      printf("%s\n", strpool_cstr2( &obj_pool, current->obj.id));
			
			if (current->obj.content){
				/* starts the content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				indent++;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			for (i=0; i<indent; i++){ /* print the indentation spaces */
				printf("    ");
			}
			printf ("%d = ", current->value.group); /* print the DFX group */
			/* print the value of atrribute, acording its type */
			switch (current->value.t_data) {
				case DXF_STR:
          if (current->value.group == 2 || (current->value.group > 5 && current->value.group < 9) ){
            printf("%s", strpool_cstr2( &name_pool, current->value.str));
          } else {
            printf("%s", strpool_cstr2( &value_pool, current->value.str));
          }
					break;
				case DXF_FLOAT:
					printf("%f", current->value.d_data);
					break;
				case DXF_INT:
					printf("%d", current->value.i_data);
			}
			printf("\n");
			current = current->next; /* go to the next in the list */
		}
		while (current == NULL){
			if (prev == ent){
				current = NULL;
				break;
			}
			prev = prev->master;
			if (prev){
				current = prev->next;
				indent --;
				if (prev == ent){
					current = NULL;
					printf("fim loop ");
					break;
				}
			}
			else{
				current = NULL;
				break;
			}
		}
	}
}

void dxf_ent_print_f (dxf_node *ent, char *path){ /* print the entity structure on file*/
	/* this function is non recursive */
	int i;
	int indent = 0;
	dxf_node *current, *prev;
	
	FILE *file;
	file = fopen(path, "w"); /* open the file */
	
	current = ent;
	while ((current != NULL) && (file != NULL)){
		prev = current;
		if (current->type == DXF_ENT){ /* DXF entity */
      for (i=0; i<indent; i++){ /* print the indentation spaces */
        fprintf(file, "    ");
      }
      
      fprintf(file, "%s\n", strpool_cstr2( &obj_pool, current->obj.id));
			
			if (current->obj.content){
				/* starts the content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				indent++;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			for (i=0; i<indent; i++){ /* print the indentation spaces */
				fprintf(file, "    ");
			}
			fprintf(file, "%d = ", current->value.group); /* print the DFX group */
			/* print the value of atrribute, acording its type */
			switch (current->value.t_data) {
				case DXF_STR:
          if (current->value.group == 2 || (current->value.group > 5 && current->value.group < 9) ){
            fprintf(file, "%s", strpool_cstr2( &name_pool, current->value.str));
          } else {
            fprintf(file, "%s", strpool_cstr2( &value_pool, current->value.str));
          }
        
					break;
				case DXF_FLOAT:
					fprintf(file, "%f", current->value.d_data);
					break;
				case DXF_INT:
					fprintf(file, "%d", current->value.i_data);
			}
			fprintf(file, "\n");
			current = current->next; /* go to the next in the list */
		}
		while (current == NULL){
			if (prev == ent){
				current = NULL;
				break;
			}
			prev = prev->master;
			if (prev){
				current = prev->next;
				indent --;
				if (prev == ent){
					current = NULL;
					fprintf(file, "fim loop ");
					break;
				}
			}
			else{
				current = NULL;
				break;
			}
		}
	}
	fclose(file);
}

dxf_node * dxf_obj_new (char *name, int pool){
	
	/* create a new DXF Object */
	//dxf_node *new_obj = (dxf_node *) malloc(sizeof(dxf_node));
	//dxf_mem_pool(enum dxf_pool_action action, int idx)
	dxf_node *new_obj = dxf_mem_pool(ADD_DXF, pool);
	if (new_obj){
    new_obj->obj.id = 0;
		if(name){
      new_obj->obj.id = strpool_inject( &obj_pool, (char const*) name, strlen(name) );
		}
		new_obj->master = NULL;
		new_obj->prev = NULL;
		new_obj->next = NULL;
		new_obj->type = DXF_ENT;
		new_obj->obj.layer = 0;
		new_obj->obj.pool = pool;
		new_obj->obj.graphics = NULL;
		
		/* create head of content's list */
		//new_obj->obj.content = (dxf_node *) malloc(sizeof(dxf_node));
		new_obj->obj.content = (dxf_node *) dxf_mem_pool(ADD_DXF, pool);
		if(new_obj->obj.content){
			new_obj->end = new_obj->obj.content;
			new_obj->obj.content->master = new_obj;
			new_obj->obj.content->prev = NULL;
			new_obj->obj.content->next = NULL;
			new_obj->obj.content->type = DXF_ATTR;
			new_obj->obj.content->value.t_data = DXF_INT;
		}
		else { /* if error, free the memory */
			//free(new_obj);
			new_obj = NULL;
		}
	}
	return new_obj;
}

dxf_node * dxf_obj_new2 (STRPOOL_U64 name, int pool){
	/* create a new DXF Object */
	dxf_node *new_obj = dxf_mem_pool(ADD_DXF, pool);
	if (!new_obj) return NULL;
  
  new_obj->obj.id = name;
  new_obj->master = NULL;
  new_obj->prev = NULL;
  new_obj->next = NULL;
  new_obj->type = DXF_ENT;
  new_obj->obj.layer = 0;
  new_obj->obj.pool = pool;
  new_obj->obj.graphics = NULL;
  
  /* create head of content's list */
  new_obj->obj.content = (dxf_node *) dxf_mem_pool(ADD_DXF, pool);
  if(!new_obj->obj.content) return NULL;
  
  new_obj->end = new_obj->obj.content;
  new_obj->obj.content->master = new_obj;
  new_obj->obj.content->prev = NULL;
  new_obj->obj.content->next = NULL;
  new_obj->obj.content->type = DXF_ATTR;
  new_obj->obj.content->value.t_data = DXF_INT;
  
	return new_obj;
}

int dxf_ident_attr_type (int group){
	/* Identifies the data type of the attribute,  */
	/* according to the value of the group. (DXF ranges) */
	
	if ((group >= 0) && (group < 10)){
		return DXF_STR; /* string */
	}
	else if( (group >= 10) && (group < 60)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 60) && (group < 100)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 100) && (group < 106)){
		return DXF_STR; /* string */
	}
	else if( (group >= 110) && (group < 150)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 170) && (group < 180)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 210) && (group < 240)){
		return DXF_FLOAT; /*  float */
	}
	else if( (group >= 270) && (group < 290)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 300) && (group < 370)){
		return DXF_STR; /* string */
	}
	else if( (group >= 370) && (group < 390)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 390) && (group < 400)){
		return DXF_STR; /* string */
	}
	else if( (group >= 400) && (group < 410)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 410) && (group < 420)){
		return DXF_STR; /* string */
	}
	else if( (group >= 420) && (group < 430)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 430) && (group < 440)){
		return DXF_STR; /* string */
	}
	else if( (group >= 440) && (group < 460)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 460) && (group < 470)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 470) && (group < 480)){
		return DXF_STR; /* string */
	}
	else if( (group >= 999) && (group < 1010)){
		return DXF_STR; /* string */
	}
	else if( (group >= 1010) && (group < 1060)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 1060) && (group < 1072)){
		return DXF_INT; /* integer */
	}
	else{ /* not defined type */
		return DXF_STR; /* default type is string */
	}
}

#if (0)
int dxf_ident_ent_type (dxf_node *obj){
	enum dxf_graph ent_type = DXF_NONE;
	if (obj){
		if (obj->type == DXF_ENT){
			if (strcmp(obj->obj.name, "LINE") == 0){
				ent_type = DXF_LINE;
			}
			else if (strcmp(obj->obj.name, "POINT") == 0){
				ent_type = DXF_POINT;
			}
			else if (strcmp(obj->obj.name, "CIRCLE") == 0){
				ent_type = DXF_CIRCLE;
			}
			else if (strcmp(obj->obj.name, "ARC") == 0){
				ent_type = DXF_ARC;
			}
			else if (strcmp(obj->obj.name, "TRACE") == 0){
				ent_type = DXF_TRACE;
			}
			else if (strcmp(obj->obj.name, "SOLID") == 0){
				ent_type = DXF_SOLID;
			}
			else if (strcmp(obj->obj.name, "TEXT") == 0){
				ent_type = DXF_TEXT;
			}
			else if (strcmp(obj->obj.name, "SHAPE") == 0){
				ent_type = DXF_SHAPE;
			}
			else if (strcmp(obj->obj.name, "INSERT") == 0){
				ent_type = DXF_INSERT;
			}
			else if (strcmp(obj->obj.name, "ATTRIB") == 0){
				ent_type = DXF_ATTRIB;
			}
			else if (strcmp(obj->obj.name, "POLYLINE") == 0){
				ent_type = DXF_POLYLINE;
			}
			else if (strcmp(obj->obj.name, "VERTEX") == 0){
				ent_type = DXF_VERTEX;
			}
			else if (strcmp(obj->obj.name, "LWPOLYLINE") == 0){
				ent_type = DXF_LWPOLYLINE;
			}
			else if (strcmp(obj->obj.name, "3DFACE") == 0){
				ent_type = DXF_3DFACE;
			}
			else if (strcmp(obj->obj.name, "VIEWPORT") == 0){
				ent_type = DXF_VIEWPORT;
			}
			else if (strcmp(obj->obj.name, "DIMENSION") == 0){
				ent_type = DXF_DIMENSION;
			}
			else if (strcmp(obj->obj.name, "ELLIPSE") == 0){
				ent_type = DXF_ELLIPSE;
			}
			else if (strcmp(obj->obj.name, "MTEXT") == 0){
				ent_type = DXF_MTEXT;
			}
			else if (strcmp(obj->obj.name, "BLOCK") == 0){
				ent_type = DXF_BLK;
			}
			else if (strcmp(obj->obj.name, "ENDBLK") == 0){
				ent_type = DXF_ENDBLK;
			}
			else if (strcmp(obj->obj.name, "HATCH") == 0){
				ent_type = DXF_HATCH;
			}
			else if (strcmp(obj->obj.name, "DIMSTYLE") == 0){
				ent_type = DXF_DIMSTYLE;
			}
			else if (strcmp(obj->obj.name, "IMAGE") == 0){
				ent_type = DXF_IMAGE;
			}
			else if (strcmp(obj->obj.name, "SPLINE") == 0){
				ent_type = DXF_SPLINE;
			}
		}
	}
	return ent_type;
}
#endif

int dxf_ident_ent_type (dxf_node *obj){
  static int init = 0;
  static STRPOOL_U64 ents[50];
  
  if (!init){
    ents[0] = strpool_inject(&obj_pool, "LINE", (int) strlen("LINE"));
    ents[1] = strpool_inject(&obj_pool, "POINT", (int) strlen("POINT"));
    ents[2] = strpool_inject(&obj_pool, "CIRCLE", (int) strlen("CIRCLE"));
    ents[3] = strpool_inject(&obj_pool, "ARC", (int) strlen("ARC"));
    ents[4] = strpool_inject(&obj_pool, "TRACE", (int) strlen("TRACE"));
    ents[5] = strpool_inject(&obj_pool, "SOLID", (int) strlen("SOLID"));
    ents[6] = strpool_inject(&obj_pool, "TEXT", (int) strlen("TEXT"));
    ents[7] = strpool_inject(&obj_pool, "SHAPE", (int) strlen("SHAPE"));
    ents[8] = strpool_inject(&obj_pool, "INSERT", (int) strlen("INSERT"));
    ents[9] = strpool_inject(&obj_pool, "ATTRIB", (int) strlen("ATTRIB"));
    ents[10] = strpool_inject(&obj_pool, "POLYLINE", (int) strlen("POLYLINE"));
    ents[11] = strpool_inject(&obj_pool, "VERTEX", (int) strlen("VERTEX"));
    ents[12] = strpool_inject(&obj_pool, "LWPOLYLINE", (int) strlen("LWPOLYLINE"));
    ents[13] = strpool_inject(&obj_pool, "3DFACE", (int) strlen("3DFACE"));
    ents[14] = strpool_inject(&obj_pool, "VIEWPORT", (int) strlen("VIEWPORT"));
    ents[15] = strpool_inject(&obj_pool, "DIMENSION", (int) strlen("DIMENSION"));
    ents[16] = strpool_inject(&obj_pool, "ELLIPSE", (int) strlen("ELLIPSE"));
    ents[17] = strpool_inject(&obj_pool, "MTEXT", (int) strlen("MTEXT"));
    ents[18] = strpool_inject(&obj_pool, "BLOCK", (int) strlen("BLOCK"));
    ents[19] = strpool_inject(&obj_pool, "ENDBLK", (int) strlen("ENDBLK"));
    ents[20] = strpool_inject(&obj_pool, "HATCH", (int) strlen("HATCH"));
    ents[21] = strpool_inject(&obj_pool, "DIMSTYLE", (int) strlen("DIMSTYLE"));
    ents[22] = strpool_inject(&obj_pool, "IMAGE", (int) strlen("IMAGE"));
    ents[23] = strpool_inject(&obj_pool, "SPLINE", (int) strlen("SPLINE"));
    ents[24] = strpool_inject(&obj_pool, "ATTDEF", (int) strlen("ATTDEF"));
    
    init = 1;
  }
  
	enum dxf_graph ent_type = DXF_NONE;
	if (obj){
		if (obj->type == DXF_ENT){
			if (obj->obj.id == ents[0]){
				ent_type = DXF_LINE;
			}
			else if (obj->obj.id == ents[1]){
				ent_type = DXF_POINT;
			}
			else if (obj->obj.id == ents[2]){
				ent_type = DXF_CIRCLE;
			}
			else if (obj->obj.id == ents[3]){
				ent_type = DXF_ARC;
			}
			else if (obj->obj.id == ents[4]){
				ent_type = DXF_TRACE;
			}
			else if (obj->obj.id == ents[5]){
				ent_type = DXF_SOLID;
			}
			else if (obj->obj.id == ents[6]){
				ent_type = DXF_TEXT;
			}
			else if (obj->obj.id == ents[7]){
				ent_type = DXF_SHAPE;
			}
			else if (obj->obj.id == ents[8]){
				ent_type = DXF_INSERT;
			}
			else if (obj->obj.id == ents[9]){
				ent_type = DXF_ATTRIB;
			}
			else if (obj->obj.id == ents[10]){
				ent_type = DXF_POLYLINE;
			}
			else if (obj->obj.id == ents[11]){
				ent_type = DXF_VERTEX;
			}
			else if (obj->obj.id == ents[12]){
				ent_type = DXF_LWPOLYLINE;
			}
			else if (obj->obj.id == ents[13]){
				ent_type = DXF_3DFACE;
			}
			else if (obj->obj.id == ents[14]){
				ent_type = DXF_VIEWPORT;
			}
			else if (obj->obj.id == ents[15]){
				ent_type = DXF_DIMENSION;
			}
			else if (obj->obj.id == ents[16]){
				ent_type = DXF_ELLIPSE;
			}
			else if (obj->obj.id == ents[17]){
				ent_type = DXF_MTEXT;
			}
			else if (obj->obj.id == ents[18]){
				ent_type = DXF_BLK;
			}
			else if (obj->obj.id == ents[19]){
				ent_type = DXF_ENDBLK;
			}
			else if (obj->obj.id == ents[20]){
				ent_type = DXF_HATCH;
			}
			else if (obj->obj.id == ents[21]){
				ent_type = DXF_DIMSTYLE;
			}
			else if (obj->obj.id == ents[22]){
				ent_type = DXF_IMAGE;
			}
			else if (obj->obj.id == ents[23]){
				ent_type = DXF_SPLINE;
			}
      else if (obj->obj.id == ents[24]){
				ent_type = DXF_ATTDEF;
			}
		}
	}
	return ent_type;
}

dxf_node * dxf_attr_new (int group, int type, void *value, int pool){
	/* create a new DXF attribute */
	//dxf_node *new_attr = (dxf_node *) malloc(sizeof(dxf_node));
	dxf_node *new_attr = (dxf_node *) dxf_mem_pool(ADD_DXF, pool);
	if (new_attr){
		new_attr->master = NULL;
		new_attr->prev = NULL;
		new_attr->next = NULL;
		new_attr->type = DXF_ATTR;
		
		new_attr->value.group = group;
		new_attr->value.t_data = type;
		
		if(type == DXF_FLOAT){
      new_attr->value.d_data = *((double *)value);
    }
		else if(type == DXF_INT){
      new_attr->value.i_data = *((int *)value);
		}
		else if(type == DXF_STR){
      /* create new string */
      if (group == 2 || (group > 5 && group < 9) ) {
        new_attr->value.str = strpool_inject( &name_pool, (char const*) value, strlen((char *) value) );
      } else {
        new_attr->value.str = strpool_inject( &value_pool, (char const*) value, strlen((char *) value) );
      }
		}
	}
	return new_attr;
}
#if 0
vector_p dxf_find_attr(dxf_node * obj, int attr){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list;
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					if(current->value.group == attr){ /* success */
						size++;
						data = realloc(data, size * sizeof(void *));
						if (data){
							data[size-1] = current;
						}
						else{
							size = 0;
							break;
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_attr2(dxf_node * obj, int attr){
	/* return the first match */
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					if(current->value.group == attr){ /* success */
						return current;
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

dxf_node * dxf_find_attr_i(dxf_node * obj, int attr, int idx){
	/* return the match of  index (idx) */
	/* if the idx is zero, will return the first occurency. If is negative, will return the last occurency.*/
	dxf_node *current;
	dxf_node *found = NULL;
	int i = 0;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					/* verify if matchs */
					if(current->value.group == attr){
						found = current;
						/* and if is the index wanted */
						if (idx == i){
							return found; /* success */
						}
						i++; /* increment and continues the search */
					}
				}
				current = current->next;
			}
		}
	}
	if (idx < 0) return found; /* return the last element found */
	else return NULL;
}

dxf_node * dxf_find_attr_i2(dxf_node * start, dxf_node * end, int attr, int idx){
	/* return the match of  index (idx) */
	/* if the idx is zero, will return the first occurency. If is negative, will return the last occurency.*/
	dxf_node *current;
	dxf_node *found = NULL;
	int i = 0;
	
	if (start != NULL){ /* check if exist */
		
		current = start;
		while (current){
			if (current->type == DXF_ATTR){
				/* verify if matchs */
				if(current->value.group == attr){
					found = current;
					/* and if is the index wanted */
					if (idx == i){
						return found; /* success */
					}
					i++; /* increment and continues the search */
				}
			}
			if (current == end) break;
			current = current->next;
		}
		
	}
	if (idx < 0) return found; /* return the last element found */
	else return NULL;
}

dxf_node * dxf_find_attr_nxt(dxf_node * obj, dxf_node ** next, int attr){
	
	if (obj == NULL) return NULL; /* check if exist */
	if (obj->type != DXF_ENT) return NULL; /* check if valid object */
	
	dxf_node *current = NULL;
	
	if (*next == NULL) current = obj->obj.content->next; /* scan from begining */
	else current = *next; /* or from last point */
	*next = NULL;
	
	while (current){
		if (current->type == DXF_ATTR){
			/* verify if matchs */
			if(current->value.group == attr){
				/* success */
				*next = current->next;
				return current;
			}
		}
		current = current->next;
	}
	
	return NULL;
}

dxf_node * dxf_find_obj_nxt(dxf_node * obj, dxf_node ** next, char *name){
	if (obj == NULL) return NULL; /* check if exist */
	if (obj->type != DXF_ENT) return NULL; /* check if valid object */
  
  STRPOOL_U64 id = strpool_inject(&obj_pool, (const char*) name, (int) strlen(name));
	
	dxf_node *current = NULL;
	
	if (*next == NULL) current = obj->obj.content->next; /* scan from begining */
	else current = *next; /* or from last point */
	*next = NULL;
	
	while (current){
		if (current->type == DXF_ENT){
			/* verify if matchs */
      if (current->obj.id == id){
				/* success */
				*next = current->next;
				return current;
			}
		}
		current = current->next;
	}
	
	return NULL;
}

int dxf_count_attr(dxf_node * obj, int attr){
	int count = 0;
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					if(current->value.group == attr){ /* success */
						count++;
					}
				}
				current = current->next;
			}
		}
	}
	return count;
}

#if 0
vector_p dxf_find_obj(dxf_node * obj, char *name){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list;
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					if(strcmp(current->obj.name, name) == 0){ /* success */
						size++;
						data = realloc(data, size * sizeof(void *));
						if (data){
							data[size-1] = current;
						}
						else{
							size = 0;
							break;
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_obj2(dxf_node * obj, char *name){
	dxf_node *current;
  
  STRPOOL_U64 id = strpool_inject(&obj_pool, (const char*) name, (int) strlen(name));
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
          if (current->obj.id == id){
						return current;
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

int dxf_count_obj(dxf_node * obj, char *name){
	dxf_node *current;
  
  STRPOOL_U64 id = strpool_inject(&obj_pool, (const char*) name, (int) strlen(name));
  
	int count = 0;
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
          if (current->obj.id == id){
						count++;
					}
				}
				current = current->next;
			}
		}
	}
	return count;
}

dxf_node * dxf_find_obj_i(dxf_node * obj, char *name, int idx){
	/* return the match of  index (idx) */
	/* if the idx is zero, will return the first occurency. If is negative, will return the last occurency.*/
	dxf_node *current;
	dxf_node *found = NULL;
	int i = 0;
  
  STRPOOL_U64 id = strpool_inject(&obj_pool, (const char*) name, (int) strlen(name));
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					/* verify if matchs */
          if (current->obj.id == id){
						found = current;
						/* and if is the index wanted */
						if (idx == i){
							return found; /* success */
						}
						i++; /* increment and continues the search */
					}
				}
				current = current->next;
			}
		}
	}
	if (idx < 0) return found; /* return the last element found */
	else return NULL;
}

#if 0
vector_p dxf_find_obj_descr(dxf_node * obj, char *name, char *descr){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list, descr_attr;
	dxf_node *current;
	
	if(obj != NULL){ /* check if obj exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){ /* sweep master content */
				if (current->type == DXF_ENT){ /* look for dxf entities */
					if(strcmp(current->obj.name, name) == 0){ /* match obj's name */
						descr_attr = dxf_find_attr(current, 2); /* look for descriptor in group 2 attribute */
						if (descr_attr.data){ /* found attribute */
							/* match descriptor */
							if(strcmp(((dxf_node **) descr_attr.data)[0]->value.s_data, descr) == 0){
								/* success */
								size++;
								data = realloc(data, size * sizeof(void *));
								if (data){
									data[size-1] = current;
								}
								else{
									size = 0;
									break;
								}
							}
							free (descr_attr.data);
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_obj_descr2(dxf_node * obj, char *name, char *descr){
	dxf_node *current, *descr_attr;
  
  if (!name || !descr) return NULL;
  
  STRPOOL_U64 id = strpool_inject(&obj_pool, (const char*) name, (int) strlen(name));
  STRPOOL_U64 dscr = strpool_inject(&name_pool, (const char*) descr, (int) strlen(descr));
  
	
	if(obj != NULL){ /* check if obj exist */
		if (obj->type == DXF_ENT){
			
			current = obj->obj.content->next;
			while (current){ /* sweep master content */
				if (current->type == DXF_ENT){ /* look for dxf entities */
					/* match obj's name */
          if (current->obj.id == id){
						descr_attr = dxf_find_attr2(current, 2); /* look for descriptor in group 2 attribute */
						if (descr_attr){ /* found attribute */
							/* match descriptor */
              if (descr_attr->value.str == dscr){
								/* success */
								return current;
							}
						}
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

void dxf_layer_assemb (dxf_drawing *drawing){
	int i, flags;
	dxf_node *current = NULL, *curr_layer = NULL;
	
	int color;
  STRPOOL_U64 name, ltype;
	int line_w;
	int frozen;
	int lock;
	int off;
	
	/* always set the index 0 as the default layer*/
	drawing->num_layers = 0;
	drawing->layers[0].name = 0;
	drawing->layers[0].ltype = 0;
	drawing->layers[0].color = 0;
	drawing->layers[0].line_w = 0;
	drawing->layers[0].frozen = 0;
	drawing->layers[0].lock = 0;
	drawing->layers[0].off = 0;
	
	drawing->layers[0].num_el = 0;
	drawing->layers[0].obj = NULL;
	
	i = 0;
	dxf_node *nxt_lay = NULL;
	while (curr_layer = dxf_find_obj_nxt(drawing->t_layer, &nxt_lay, "LAYER")){ /* get the next layer */
	
		name = 0;
		color = 0;
		ltype = 0;
		line_w = 0;
		frozen = 0;
		lock = 0;
		off = 0;
		
		/* and sweep its content */
		if (curr_layer->obj.content) current = curr_layer->obj.content->next;
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* layer name */
            name = current->value.str;
						break;
					case 6: /* layer line type name */
            ltype = current->value.str;
						break;
					case 62: /* layer color */
						color = current->value.i_data;
						if (color < 0) {
							off = 1;
							color = abs(color);
						}
						break;
					case 70: /* flags */
						flags = current->value.i_data;
						if (flags & 1) frozen = 1;
						if (flags & 4) lock = 1;
						break;
					case 370:
						line_w = current->value.i_data;
				}
			}
			current = current->next;
		}
		if (i < DXF_MAX_LAYERS){
			/* set the variables on the current layer in drawing structure */
      drawing->layers[i].name = name;
      drawing->layers[i].ltype = ltype;
			drawing->layers[i].color = color;
			drawing->layers[i].line_w = line_w;
			drawing->layers[i].frozen = frozen;
			drawing->layers[i].lock = lock;
			drawing->layers[i].off = off;
			drawing->layers[i].num_el = 0;
			drawing->layers[i].obj = curr_layer;
		}
		
		i++;
		if (!nxt_lay) break; /* end of LAYERs in table */
	}
	if (i< DXF_MAX_LAYERS) drawing->num_layers += i;
	else drawing->num_layers = DXF_MAX_LAYERS;
}

void dxf_ltype_assemb (dxf_drawing *drawing){
	int i, j, pat_idx;
	dxf_node *current = NULL, *curr_ltype = NULL;
	
  STRPOOL_U64 name, descr;
	int size;
	dxf_ltyp_pat dashes[DXF_MAX_PAT];
	double length, max;
	
	/* always set the index 0 as the default ltype*/
	drawing->num_ltypes = 0;
	drawing->ltypes[0].name = 0;
	drawing->ltypes[0].descr = 0;
	drawing->ltypes[0].size = 1;
	drawing->ltypes[0].dashes[0].dash = 0.0;
	drawing->ltypes[0].dashes[0].type = LTYP_SIMPLE;
	drawing->ltypes[0].length = 0;
	drawing->ltypes[0].num_el = 0;
	drawing->ltypes[0].obj = NULL;
	
	i = 0;
	dxf_node *nxt_ltyp = NULL;
	while (curr_ltype = dxf_find_obj_nxt(drawing->t_ltype, &nxt_ltyp, "LTYPE")){ /* get the next ltype */
		
		/* init the line type */
		name = 0;
		descr = 0;
		size = 0;
		/* init first dash */
		dashes[0].dash = 0;
		dashes[0].type = LTYP_SIMPLE;
		dashes[0].str = 0;
		dashes[0].sty = 0;
		dashes[0].sty_i = -1;
		dashes[0].abs_rot = 0;
		dashes[0].rot = 0.0;
		dashes[0].scale = 0.0;
		dashes[0].ofs_x = 0.0;
		dashes[0].ofs_y = 0.0;
		
		pat_idx = 0;
		length = 0;
		
		if (curr_ltype->obj.content) current = curr_ltype->obj.content->next;
		
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* ltype name */
            name = current->value.str;
						break;
					case 3: /* ltype descriptive text */
            descr = current->value.str;
						break;
					case 40: /* pattern length */
						length = current->value.d_data;
						break;
					case 49: /* pattern element */
						if (pat_idx < DXF_MAX_PAT) {
							dashes[pat_idx].dash = current->value.d_data; /* current dash length */
							dashes[pat_idx].type = LTYP_SIMPLE; /* presume simple regular dash */
							/* init parameters for complex element */
							dashes[pat_idx].str = 0;
							dashes[pat_idx].sty = 0;
							dashes[pat_idx].sty_i = -1;
							dashes[pat_idx].abs_rot = 0;
							dashes[pat_idx].rot = 0.0;
							dashes[pat_idx].scale = 0.0;
							dashes[pat_idx].ofs_x = 0.0;
							dashes[pat_idx].ofs_y = 0.0;
							
							pat_idx++;
						}
						break;
					case 73: /* num of pattern elements */
						size = current->value.i_data;
						if (size > DXF_MAX_PAT) {
							size < DXF_MAX_PAT;}
						break;
					case 74: /* complex flag */
						if (pat_idx > 0){
							if (current->value.i_data & 1) dashes[pat_idx - 1].abs_rot = 1;
							if (current->value.i_data & 2) dashes[pat_idx - 1].type = LTYP_STRING;
							if (current->value.i_data & 4) dashes[pat_idx - 1].type = LTYP_SHAPE;
						}
						break;
					case 75: /* complex - shape num */
						if (pat_idx > 0){
							if (dashes[pat_idx - 1].type == LTYP_SHAPE) {
								dashes[pat_idx - 1].num =current->value.i_data;
							}
						}
						break;
					case 9: /* complex - string */
						if (pat_idx > 0){
							if (dashes[pat_idx - 1].type == LTYP_STRING) {
                dashes[pat_idx - 1].str = current->value.str;
							}
						}
						break;
					case 44: /* complex - offset x */
						if (pat_idx > 0){
							dashes[pat_idx - 1].ofs_x = current->value.d_data;
						}
						break;
					case 45: /* complex - offset y */
						if (pat_idx > 0){
							dashes[pat_idx - 1].ofs_y = current->value.d_data;
						}
						break;
					case 46: /* complex - scale */
						if (pat_idx > 0){
							dashes[pat_idx - 1].scale = current->value.d_data;
						}
						break;
					case 50: /* complex - rotation */
						if (pat_idx > 0){
							dashes[pat_idx - 1].rot = current->value.d_data;
						}
						break;
					case 340: /* complex - style handle */
						if (pat_idx > 0){
							dashes[pat_idx - 1].sty_i = 0; /* init  style index */
							/* convert string handle to integer */
              long int id = strtol(strpool_cstr2( &value_pool, current->value.str), NULL, 16);
							/* look for correspondent style object */
							dxf_node *t_obj = dxf_find_handle(drawing->t_style, id);
							
							for (j=0; j < drawing->num_tstyles; j++){ /* sweep drawing's text styles */
								if (drawing->text_styles[j].obj == t_obj){ /* verify if object matchs */
									dashes[pat_idx - 1].sty_i = j; /* get index */
									/* and style name */
                  dashes[pat_idx - 1].sty = drawing->text_styles[j].name;
									break;
								}
							}
						}
						break;
				}
			}
			current = current->next;
		}
		
		/* set the variables on the current ltype in drawing structure */
		if (i < DXF_MAX_LTYPES){
      drawing->ltypes[i].name = name;
      drawing->ltypes[i].descr = descr;
			for (j = 0; j < size; j++){ /* store dashes parameters */
				drawing->ltypes[i].dashes[j].dash = dashes[j].dash;
				drawing->ltypes[i].dashes[j].type = dashes[j].type;
				drawing->ltypes[i].dashes[j].num = 0;
				if (dashes[j].type == LTYP_SHAPE){
					drawing->ltypes[i].dashes[j].num = dashes[j].num;
				} else if (dashes[j].type == LTYP_STRING){
          drawing->ltypes[i].dashes[j].str = dashes[j].str;
				}
        drawing->ltypes[i].dashes[j].sty = dashes[j].sty;
				drawing->ltypes[i].dashes[j].sty_i = dashes[j].sty_i;
				drawing->ltypes[i].dashes[j].abs_rot = dashes[j].abs_rot;
				drawing->ltypes[i].dashes[j].rot = dashes[j].rot;
				drawing->ltypes[i].dashes[j].scale = dashes[j].scale;
				drawing->ltypes[i].dashes[j].ofs_x = dashes[j].ofs_x;
				drawing->ltypes[i].dashes[j].ofs_y = dashes[j].ofs_y;
			}
			drawing->ltypes[i].size = size;
			drawing->ltypes[i].length = length;
			drawing->ltypes[i].num_el = 0;
			drawing->ltypes[i].obj = curr_ltype;
		}
		
		i++;
		if (!nxt_ltyp) break; /* end of LTYPEs in table */
	}
	if (i < DXF_MAX_LTYPES) drawing->num_ltypes += i;
	else drawing->num_ltypes = DXF_MAX_LTYPES;
}

void dxf_tstyles_assemb (dxf_drawing *drawing){
	int i;
	dxf_node *current = NULL, *curr_tstyle = NULL;
	
  STRPOOL_U64 name, file_name, big_file, subst_file;
	
	int flags1;
	int flags2;
	int num_el;
	
	double fixed_h;
	double width_f;
	double oblique;
	
	drawing->num_tstyles = 0;
	
	struct tfont * font = NULL;
	
	/* open default font */
	//shape *shx_font = shx_font_open("txt.shx");
	//shape *shx_font = NULL;
	
	//if (shx_font){
		/* always set the index 0 as the default font */
		//drawing->num_tstyles = 1;
		drawing->text_styles[0].name = 0;
		drawing->text_styles[0].file = 0;
		drawing->text_styles[0].big_file = 0;
		drawing->text_styles[0].subst_file = 0;
		
		drawing->text_styles[0].flags1 = 0;
		drawing->text_styles[0].flags2 = 0;
		drawing->text_styles[0].fixed_h = 0.0;
		drawing->text_styles[0].width_f = 1.0;
		drawing->text_styles[0].oblique = 0.0;
		
		drawing->text_styles[0].num_el = 0;
		drawing->text_styles[0].obj = NULL;
	//}
	
	i = 0;
	dxf_node *nxt_sty = NULL;
	while (curr_tstyle = dxf_find_obj_nxt(drawing->t_style, &nxt_sty, "STYLE")){ /* get the next style */
	
		name = 0;
		file_name = 0;
		big_file = 0;
		subst_file = 0;
		
		flags1 = 0;
		flags2 = 0;
		fixed_h = 0.0;
		width_f = 1.0;
		oblique = 0.0;
			
		if (curr_tstyle->obj.content) current = curr_tstyle->obj.content->next;
		
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* tstyle name */
            name = current->value.str;
						break;
					case 3: /* file name */
            file_name = current->value.str;
						break;
					case 4: /* bigfont file name */
            big_file = current->value.str;
						break;
					case 40: /* fixed height*/
						fixed_h = current->value.d_data;
						break;
					case 41: /* width factor*/
						width_f = current->value.d_data;
						break;
					case 50: /* oblique angle*/
						oblique = current->value.d_data;
						break;
					case 70: /* flags */
						flags1 = current->value.i_data;
						break;
					case 71: /* flags */
						flags2 = current->value.i_data;
				}
			}
			current = current->next;
		}
		if (i < DXF_MAX_FONTS){
			/* set the variables on the current font in drawing structure */
      drawing->text_styles[i].name = name;
      drawing->text_styles[i].file = file_name;
      drawing->text_styles[i].big_file = big_file;
			
			drawing->text_styles[i].flags1 = flags1;
			drawing->text_styles[i].flags2 = flags2;
			drawing->text_styles[i].fixed_h = fixed_h;
			drawing->text_styles[i].width_f = width_f;
			drawing->text_styles[i].oblique = oblique;
			
			//shx_font = shx_font_open(file_name);
			//drawing->text_styles[i].shx_font = shx_font;
			drawing->text_styles[i].num_el = 0;
			drawing->text_styles[i].obj = curr_tstyle;
			
      drawing->text_styles[i].subst_file = 0;
			
			/* try to load fonts */
			if(font = add_font_list(drawing->font_list,
        (char *) strpool_cstr2( &value_pool, drawing->text_styles[i].file),
        drawing->dflt_fonts_path))
      {
				drawing->text_styles[i].font = font;
			}
			else if (drawing->dflt_font){
				font = drawing->dflt_font;
				drawing->text_styles[i].font = font;
        drawing->text_styles[i].subst_file = strpool_inject( &value_pool,
          (char const*) font->name, strlen(font->name) );
			}
		}
		i++;
		if (!nxt_sty) break; /* end of STYLEs in table */
	}
	if (i < DXF_MAX_FONTS) drawing->num_tstyles += i;
	else drawing->num_tstyles = DXF_MAX_FONTS;
}

int dxf_lay_idx (dxf_drawing *drawing, STRPOOL_U64 name){
	int i;
	if (drawing){
		for (i=1; i < drawing->num_layers; i++){
			
      if ( drawing->layers[i].name == name ){
				return i;
			}
		}
	}
	
	return 0; /*if search fails, return the standard layer */
}


int dxf_ltype_idx (dxf_drawing *drawing, STRPOOL_U64 name){
	int i;
	if (drawing){
		for (i=1; i < drawing->num_ltypes; i++){
			
      if (drawing->ltypes[i].name == name){
				return i;
			}
		}
	}
	
	return 0; /*if search fails, return the standard layer */
}

int dxf_tstyle_idx (dxf_drawing *drawing, STRPOOL_U64 name, STRPOOL_U64 file){
	int i;
  if (!drawing) return 0;
  
  static char file1[DXF_MAX_CHARS + 1], file2[DXF_MAX_CHARS + 1];
  if (file){
    strncpy (file1, strpool_cstr2( &value_pool, file), DXF_MAX_CHARS);
    str_upp(file1);
  }
	
  for (i = 0; i < drawing->num_tstyles; i++){
    if ( name && (drawing->text_styles[i].name == name) ){
      return i;
    }
    else if (!name && file) {
      strncpy (file2, strpool_cstr2( &value_pool, drawing->text_styles[i].file), DXF_MAX_CHARS);
      str_upp(file2);
      if (strcmp(file1, file2) == 0) return i;
      
    }
  }
	
	
	return 0; /*if search fails, return the standard style */
}

int dxf_save (char *path, dxf_drawing *drawing){
	if (!drawing) return 0;
	if (drawing->main_struct == NULL) return 0;
  
  static int init = 0;
  static STRPOOL_U64 section, table;
  
  if (!init){
    section = strpool_inject( &obj_pool, "SECTION", strlen("SECTION") );
    table = strpool_inject( &obj_pool, "TABLE", strlen("TABLE") );
    
    init = 1;
  }
	
	FILE *file;
	list_node *stack, *item ;
	dxf_node *current;
	int ret_success;
	
	/* initialize the stack */
	int stack_size = 0;
	stack = list_new(NULL, ONE_TIME);
	
	ret_success = 0;
	
	file = fopen(path, "w"); /* open the file */
	
	
	if (file != NULL){
		current = drawing->main_struct->obj.content->next;
		while ((current != NULL) || (stack_size > 0)){
			if (current == NULL){ /* end of list sweeping */
				/* try to up in the structure hierarchy */
				item = list_pop(stack);
				if (item){
					stack_size--;
					current = (dxf_node *)item->data;
				}
				//current = stack_pop (&stack);
				if (current){
					/* write the end of complex entities, acording its type */
          if( current->obj.id == section ){
						fprintf(file, "0\nENDSEC\n");
					}
          else if( current->obj.id == table ){
						fprintf(file, "0\nENDTAB\n");
					}
					current = current->next; /* go to the next in the list */
				}
			}
			else if (current->type == DXF_ENT){ /* DXF entity */
				/* down in the structure hierarchy */
				list_push(stack, list_new((void *)current, ONE_TIME));
				stack_size++;
				//stack_push (&stack, current);
				fprintf(file, "0\n%s\n", /* write the start of entity */
          strpool_cstr2( &obj_pool, current->obj.id)); 
				
				if (current->obj.content){
					/* starts the content sweep */
					current = current->obj.content->next;
				}
			}
			else if (current->type == DXF_ATTR){ /* DXF attribute */
				fprintf(file, "%d\n",  current->value.group); /* write the DFX group */
				/* write the value of attribute, acording its type */
				switch (current->value.t_data) {
					case DXF_STR:
            if (current->value.group == 2 || (current->value.group > 5 && current->value.group < 9) ){
              fprintf(file, "%s\n", strpool_cstr2( &name_pool, current->value.str));
            } else {
              fprintf(file, "%s\n", strpool_cstr2( &value_pool, current->value.str));
            }
						break;
					case DXF_FLOAT:
						fprintf(file, "%g\n", current->value.d_data);
						break;
					case DXF_INT:
						fprintf(file, "%d\n",  current->value.i_data);
				}
				current = current->next; /* go to the next in the list */
			}
		}
		ret_success = 1; /* return success */
		fclose(file);
	}
	list_mem_pool(ZERO_LIST, ONE_TIME);
	
	return ret_success;
}

int dxf_read (dxf_drawing *drawing, char *buf, long fsize, int *prog){
	static enum {NONE, INIT, READ, FINISH} state = INIT;
	static char *line, *cur_line, *next_line, line_buf[DXF_MAX_CHARS], line_cpy[DXF_MAX_CHARS];
	static dxf_node *main_struct = NULL;
	static dxf_node *master, *prev, *next, *tmp, *last_obj;
	
	static long f_index = 0;  /*  indexes the file's lines */
	int line_size = 0;

	static dxf_node *new_node = NULL, *blk = NULL, *cplx = NULL, *part = NULL;
	
	static int group = 0; /* current group */
	static int t_data = DXF_STR; /* current type of data */
	static double d_data;
	static int i_data;
	//static vector_p part;
	static int blk_end = 0, cplx_end = 0; /* close block and include ENDBLOCK in structure*/
	static int ins_flag = 0; /* inside INSERT entity - for cascadind ATTRIB ents */
	static int mtext_flag = 0; /* inside MTEXT entity - to avoid trim whitespaces in text */
  
  static int init = 0;
  static STRPOOL_U64 section, table, block, poly, endsec, endtab, endblk;
  static STRPOOL_U64 insert, mtext, seqend;
  STRPOOL_U64 obj_test;
  
  if (!init){
    section = strpool_inject( &obj_pool, "SECTION", strlen("SECTION") );
    table = strpool_inject( &obj_pool, "TABLE", strlen("TABLE") );
    block = strpool_inject( &obj_pool, "BLOCK", strlen("BLOCK") );
    poly = strpool_inject( &obj_pool, "POLYLINE", strlen("POLYLINE") );
    endsec = strpool_inject( &obj_pool, "ENDSEC", strlen("ENDSEC") );
    endtab = strpool_inject( &obj_pool, "ENDTAB", strlen("ENDTAB") );
    endblk = strpool_inject( &obj_pool, "ENDBLK", strlen("ENDBLK") );
    insert = strpool_inject( &obj_pool, "INSERT", strlen("INSERT") );
    mtext = strpool_inject( &obj_pool, "MTEXT", strlen("MTEXT") );
    seqend = strpool_inject( &obj_pool, "SEQEND", strlen("SEQEND") );
    
    init = 1;
  }
	
	if (state == INIT){
		if ((!drawing)||(!buf)){
			goto quit_error;
		}

		/*  the drawing */
		if (!dxf_drawing_clear(drawing)){
			goto quit_error;
		}
		main_struct = drawing->main_struct;
		cur_line = buf;
		
		master = main_struct; /* current master is the main struct */
		last_obj = main_struct;
		prev = main_struct->obj.content; /* the list starts on main_struct's content */
		next = NULL; /* end of list (empty list) */
		f_index = 0;
		state = READ;
	}
	
	if (state == READ){
		int i = 0;
		while((i++ < 10001) && (state == READ)){
			next_line = strpbrk(cur_line, "\r\n");
			if (next_line) {
				line_size = next_line - cur_line;
				if (line_size > DXF_MAX_CHARS - 1) line_size = DXF_MAX_CHARS - 1;
				memcpy(line_buf, cur_line, line_size);
				line_buf[line_size] = '\0';  /*terminate the current line*/
			}
			else{
				strncpy(line_buf, cur_line, DXF_MAX_CHARS);
			}
			/* copy original string to avoid trim spaces */
			strncpy(line_cpy, line_buf, DXF_MAX_CHARS);
			
			/* trim leading/trailing whitespace of line */
			line = trimwhitespace(line_buf);
			
			if((f_index % 2) == 0){ /* check if the line is even */
				/* in DXF files, the even lines identify the groups */
				group = atoi(line);
				
				/* Identifies the data type of the next line,  */
				/* according to the value of the group. (DXF ranges) */
				
				t_data = dxf_ident_attr_type(group);
				
			}
			else {
				/* strips \n of line string */
				//line[strcspn(line, "\r\n")] = 0; /* strips \n or \r of line string */
				
				switch(group){
					case 0:
						if (blk_end) { /* End Block after ENDBLK entity */
							blk_end = 0;
							/* back to the previous level on hierarchy */
							master = blk->master;
							last_obj = master;
							prev = blk;
						} 
						else if (cplx_end) { /* End Complex after SEQEND entity */
							cplx_end = 0;
							/* back to the previous level on hierarchy */
							master = cplx->master;
							last_obj = master;
							prev = cplx;
						} 
						else prev = master->end;
					
						str_upp(line); /* upper case the line */
					
						/* Find the end of the master's content list */
						//tmp = master->obj.content;
						//while(tmp->next != NULL)
						//	tmp = tmp->next;
						//prev = tmp; /* new objs will append here */
						
						ins_flag = 0;
						mtext_flag = 0;
					
						/* new level of hierarchy  */
            obj_test = strpool_inject( &obj_pool, (const char*) line, strlen(line) );
            if (obj_test == section || obj_test == table || obj_test == block || obj_test == poly){
							new_node = dxf_obj_new (line, drawing->pool); /* new object */
							if (new_node){
								/*  append new to master's list */
								new_node->master = master;
								new_node->prev = prev;
								if (prev){
									prev->next = new_node;
								}
								new_node->next = NULL; /* at end of list */
								master->end = new_node;
								last_obj = new_node;
								
                if ( obj_test == block ) blk = new_node;
                if ( obj_test == poly ) cplx = new_node;
								
								/* the new becomes the master */
								master = new_node;
								prev = new_node->obj.content; /* new objs will append here */
							}
						}
						
						/* back to the previous level on hierarchy */
            else if (obj_test == endsec || obj_test == endtab){
							if(master->master){
								/* back to master's master ;) */
								master = master->master;
								last_obj = master;
								/* Find the end of the master's content list */
								//tmp = master->obj.content;
								//while(tmp->next != NULL)
								//	tmp = tmp->next;
								//prev = tmp; /* new items will append here */
								prev = master->end;
							}
						}
						
						/*  new ordinary DXF object */
						else {
              if ( obj_test == insert ) ins_flag = 1;
              if ( obj_test == mtext ) mtext_flag = 1;
							
							new_node = dxf_obj_new (line, drawing->pool); /* new object */
							if (new_node){
								/*  append new to master's list */
								new_node->master = master;
								new_node->prev = prev;
								if (prev){
									prev->next = new_node;
								}
								new_node->next = NULL; /* append at end of list */
								master->end = new_node;
								prev = new_node->obj.content; /* new attrs will append here */
								new_node->end = new_node->obj.content;
								last_obj = new_node;
								
                if ( obj_test == endblk ) {
									blk_end = 1;
									//blk = master;
								}
                if ( obj_test == seqend ) cplx_end = 1;
							}
							
						}
						//new_node = NULL;
						break;
						
					case 66: /* Entities follow flag -  for INSERT ents only*/
						
						i_data = atoi(line);
						new_node = dxf_attr_new (group, t_data, (void *) &i_data, drawing->pool);
						if (new_node){
							/*  append new to last obj's list */
							new_node->master = last_obj;
							new_node->prev = prev;
							if (prev){
								prev->next = new_node;
							}
							new_node->next = NULL; /* append at end of list */
							last_obj->end = new_node;
						}
					
						if ((i_data != 0) && (ins_flag != 0)){
							/* new level of hierarchy */
							cplx = last_obj;
							master = last_obj; /*  the last obj becomes the master */
							
							/* Find the end of the master's content list */
							//tmp = master->obj.content;
							//while(tmp->next != NULL)
							//	tmp = tmp->next;
							//prev = tmp; /* new objs will append here */
							prev = master->end;
						}
						else prev = new_node;
						break;
					default:
						switch(t_data) {
							case DXF_FLOAT :
								d_data = atof(line);
								new_node = dxf_attr_new (group, t_data, (void *) &d_data, drawing->pool);
								break;
							case DXF_INT :
								i_data = atoi(line);
								new_node = dxf_attr_new (group, t_data, (void *) &i_data, drawing->pool);
								break;
							case DXF_STR :
								if (mtext_flag && (group == 1 || group == 3))
									new_node = dxf_attr_new (group, t_data, (void *) line_cpy, drawing->pool);
								else new_node = dxf_attr_new (group, t_data, (void *) line, drawing->pool);
						}
						
						if (new_node){
							/*  append new to last obj's list */
							new_node->master = last_obj;
							new_node->prev = prev;
							if (prev){
								prev->next = new_node;
							}
							new_node->next = NULL; /* append at end of list */
							last_obj->end = new_node;
							prev = new_node;
						}
					
				}
				new_node = NULL;
			}
			
			f_index++; /* next line index */
			if (next_line)
				if (next_line[0] == '\r' && next_line[1] == '\n') next_line++;
			
			cur_line = next_line ? (next_line+1) : NULL;
			if(cur_line == NULL){
				//free(buf);
				//buf = NULL;
				state = FINISH;
			}
		}
		if (cur_line){
			*prog = (int)((double)(cur_line - buf)*100/fsize);
		}
		return 1;
	}
	if (state == FINISH){
		
		/* disassembly the drawing structure */
		/* the main sections */
		part = dxf_find_obj_descr2(main_struct, "SECTION", "HEADER");
		if (part) drawing->head = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "TABLES");
		if (part) drawing->tabs = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "BLOCKS");
		if (part) drawing->blks = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "ENTITIES");
		if (part) drawing->ents = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "OBJECTS");
		if (part) drawing->objs = part;
		
		/* the tables */
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "LTYPE");
		if (part) drawing->t_ltype = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "LAYER");
		if (part) drawing->t_layer = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "STYLE");
		if (part) drawing->t_style = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "VIEW");
		if (part) drawing->t_view = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "UCS");
		if (part) drawing->t_ucs = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "VPORT");
		if (part) drawing->t_vport = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "DIMSTYLE");
		if (part) drawing->t_dimst = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "BLOCK_RECORD");
		if (part) drawing->blks_rec = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "APPID");
		if (part) drawing->t_appid = part;
		
		/* assemble the layers list */
		dxf_layer_assemb (drawing);
		
		/* assemble the text style list */
		dxf_tstyles_assemb(drawing);
		
		/* assemble the ltypes list */
		dxf_ltype_assemb (drawing);
		
		/*find the handle seed */
		drawing->hand_seed = dxf_find_attr2(drawing->head, 5);
		
		/*get drawing version*/
		dxf_node *version, *start = NULL, *end = NULL;
		drawing->version = 0;
		if(dxf_find_head_var(drawing->head, "$ACADVER", &start, &end)){
			version = dxf_find_attr_i2(start, end, 1, 0);
			if (version != NULL){
        const char * str = NULL;
        if (version->value.group == 2 || (version->value.group > 5 && version->value.group < 9) ){
          str = strpool_cstr( &name_pool, version->value.str);
        } else{
          str = strpool_cstr( &value_pool, version->value.str);
        }
        if (str){
          drawing->version = atoi(str + 2);
				}
			}
		}
		
		/* get line type scales */
		drawing->ltscale = 1.0;
		drawing->celtscale = 1.0;
		
		if(dxf_find_head_var(drawing->head, "$LTSCALE", &start, &end)){
			part = dxf_find_attr_i2(start, end, 40, 0);
			if (part != NULL){
				drawing->ltscale = part->value.d_data;
			}
		}
		
		if(dxf_find_head_var(drawing->head, "$CELTSCALE", &start, &end)){
			part = dxf_find_attr_i2(start, end, 40, 0);
			if (part != NULL){
				drawing->celtscale = part->value.d_data;
			}
		}
		
		drawing->dimscale = 1.0;
		drawing->dimlfac = 1.0;
		drawing->dimpost[0] = '<'; drawing->dimpost[1] = '>'; drawing->dimpost[2] = 0;
		strncpy(drawing->dimtxsty, "Standard", DXF_MAX_CHARS);
		strncpy(drawing->dimblk, "Open", DXF_MAX_CHARS);
		drawing->dimdec = 2;
		
		if(dxf_find_head_var(drawing->head, "$DIMSCALE", &start, &end)){
			part = dxf_find_attr_i2(start, end, 40, 0);
			if (part != NULL){
				drawing->dimscale = part->value.d_data;
			}
		}
		
		if(dxf_find_head_var(drawing->head, "$DIMLFAC", &start, &end)){
			part = dxf_find_attr_i2(start, end, 40, 0);
			if (part != NULL){
				drawing->dimlfac = part->value.d_data;
			}
		}
		
		if(dxf_find_head_var(drawing->head, "$DIMPOST", &start, &end)){
			part = dxf_find_attr_i2(start, end, 1, 0);
			if (part != NULL){
        const char * str = NULL;
        if (part->value.group == 2 || (part->value.group > 5 && part->value.group < 9) ){
          str = strpool_cstr2( &name_pool, part->value.str);
        } else{
          str = strpool_cstr2( &value_pool, part->value.str);
        }
        strncpy (drawing->dimpost, str, DXF_MAX_CHARS);
			}
		}
		if(dxf_find_head_var(drawing->head, "$DIMTXSTY", &start, &end)){
			part = dxf_find_attr_i2(start, end, 7, 0);
			if (part != NULL){
        const char * str = NULL;
        if (part->value.group == 2 || (part->value.group > 5 && part->value.group < 9) ){
          str = strpool_cstr2( &name_pool, part->value.str);
        } else{
          str = strpool_cstr2( &value_pool, part->value.str);
        }
        strncpy (drawing->dimtxsty, str, DXF_MAX_CHARS);
			}
		}
		if(dxf_find_head_var(drawing->head, "$DIMBLK", &start, &end)){
			part = dxf_find_attr_i2(start, end, 1, 0);
			if (part != NULL){
        const char * str = NULL;
        if (part->value.group == 2 || (part->value.group > 5 && part->value.group < 9) ){
          str = strpool_cstr2( &name_pool, part->value.str);
        } else{
          str = strpool_cstr2( &value_pool, part->value.str);
        }
        strncpy (drawing->dimblk, str, DXF_MAX_CHARS);
			}
		}
		
		if(dxf_find_head_var(drawing->head, "$DIMDEC", &start, &end)){
			part = dxf_find_attr_i2(start, end, 70, 0);
			if (part != NULL){
				drawing->dimdec = part->value.i_data;
			}
		}
		
		//return drawing;
		state = INIT;
		*prog = 100;
	}
	return 0;
	
quit_error:
	state = INIT;
	return -1;
}

void dxf_append(dxf_node *master, dxf_node *new_node){
	if (master && new_node){
		dxf_node *prev = master->end;
		/*  append new to master's list */
		new_node->master = master;
		new_node->prev = prev;
		if (prev){
			prev->next = new_node;
		}
		new_node->next = NULL; /* append at end of list */
		master->end = new_node;
	}
}

void dxf_list_clear (dxf_node *list){
	if (list){
		list->obj.id = 0;
		list->master = NULL;
		list->prev = NULL;
		list->next = NULL;
		list->type = DXF_ENT;
		list->obj.graphics = NULL;
		if(list->obj.content){
			list->end = list->obj.content;
			list->obj.content->master = list;
			list->obj.content->prev = NULL;
			list->obj.content->next = NULL;
			list->obj.content->type = DXF_ATTR;
			list->obj.content->value.t_data = DXF_INT;
		}
	}
}

int dxf_find_head_var(dxf_node *obj, char *var, dxf_node **start, dxf_node **end){
	/* find the range of attributes of extended data, indicated by var name */
	dxf_node *current;
	int found = 0;
	
	*start = NULL;
	*end = NULL;
  
  STRPOOL_U64 test = strpool_inject( &value_pool, (char const*) var, strlen(var) );
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				/* try to find the first entry, by matching the var name */
				if (!found){
					if (current->type == DXF_ATTR){
						if(current->value.group == 9){
              if (current->value.str == test){
								found = 1; /* var found */
								*start = current;
								*end = current;
							}
						}
					}
				}
				else{
					/* after the first entry, look by end */
					if (current->type == DXF_ATTR){
						/* breaks if is found a new var entry */
						if(current->value.group == 9){
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

int dxf_drawing_clear (dxf_drawing *drawing){
	if (drawing){
		/* init the drawing */
		drawing->head = NULL;
		drawing->tabs = NULL;
		drawing->blks = NULL;
		drawing->ents = NULL;
		drawing->objs = NULL; 
		drawing->t_ltype = NULL;
		drawing->t_layer = NULL;
		drawing->t_style = NULL;
		drawing->t_view = NULL;
		drawing->t_ucs = NULL;
		drawing->t_vport = NULL;
		drawing->t_dimst = NULL;
		drawing->blks_rec = NULL;
		drawing->t_appid = NULL;
		drawing->main_struct = NULL;
		
		drawing->hand_seed = NULL;
		drawing->num_layers = 0;
		drawing->num_ltypes = 0;
		drawing->num_tstyles = 0;
		/*
		drawing->font_list = NULL;
		drawing->dflt_font = NULL;
		drawing->dflt_fonts_path = NULL;
		*/
		
		dxf_image_clear_list(drawing);
		dxf_xref_clear_list(drawing);
		//drawing->img_list = NULL;
		
		/* create a new main_struct */
		dxf_node *main_struct = dxf_obj_new(NULL, drawing->pool);
		if (!main_struct){
			return 0;
		}
		
		drawing->main_struct = main_struct;
		return 1;
	}
	return 0;
}

dxf_drawing *dxf_drawing_new(int pool){
	dxf_drawing *drawing = malloc(sizeof(dxf_drawing));
	if (drawing){
		drawing->img_list = NULL;
		drawing->xref_list = NULL;
		drawing->pool = pool;
		if (!dxf_drawing_clear(drawing)){
			free(drawing);
			return NULL;
		}
		drawing->font_list = NULL;
		drawing->dflt_font = NULL;
		drawing->dflt_fonts_path = NULL;
    
	}
	
	return drawing;
}

int dxf_drawing_term(dxf_drawing *drawing){
  if (!drawing) return 0;
  
  dxf_image_clear_list(drawing);
  dxf_xref_clear_list(drawing);
  
  free(drawing);
  
  return 1;
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

int dxf_attr_append(dxf_node *master, int group, void *value, int pool){
	if (master){
		if (master->type == DXF_ENT){
			int type = dxf_ident_attr_type(group);
			dxf_node *new_attr = dxf_attr_new(group, type, value, pool);
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

int dxf_attr_append_cpy(dxf_node *master, dxf_node *orig, int pool){
	if (!master || !orig) return 0;
  if (master->type != DXF_ENT) return 0;
  if (orig->type != DXF_ATTR) return 0;
  
  dxf_node *new_attr = (dxf_node *) dxf_mem_pool(ADD_DXF, pool);
  if (!new_attr) return 0;
  
  new_attr->type = DXF_ATTR;
  new_attr->value.group = orig->value.group;
  new_attr->value.t_data = orig->value.t_data;
  
  if(orig->value.t_data == DXF_FLOAT){
    new_attr->value.d_data = orig->value.d_data;
  }
  else if(orig->value.t_data == DXF_INT){
    new_attr->value.i_data = orig->value.i_data;
  }
  else if(orig->value.t_data == DXF_STR){
    new_attr->value.str = orig->value.str;
  }
  
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

int dxf_attr_insert_before(dxf_node *attr, int group, void *value, int pool){
	if (attr){
		if (attr->type == DXF_ATTR){
			int type = dxf_ident_attr_type(group);
			dxf_node *new_attr = dxf_attr_new(group, type, value, pool);
			if (new_attr){
				new_attr->master = attr->master;
				
				dxf_node *next = attr, *prev = attr->prev;
				
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
				
				return 1;
			}
		}
	}
	return 0;
}

dxf_node * dxf_attr_insert_after(dxf_node *attr, int group, void *value, int pool){
	if (attr){
		if (attr->type == DXF_ATTR){
			int type = dxf_ident_attr_type(group);
			dxf_node *new_attr = dxf_attr_new(group, type, value, pool);
			if (new_attr){
				new_attr->master = attr->master;
				
				dxf_node *next = attr->next, *prev = attr;
				
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
				if (new_attr->master){
					if (prev == new_attr->master->end){
						new_attr->master->end = new_attr;
					}
				}
				return new_attr;
			}
		}
	}
	return NULL;
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
          if (group == 2 || (group > 5 && group < 9) )
            found_attr->value.str = strpool_inject( &name_pool, (char const*) value, strlen((char*) value) );
          else found_attr->value.str = strpool_inject( &value_pool, (char const*) value, strlen((char*) value) );
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
          if (group == 2 || (group > 5 && group < 9) )
            found_attr->value.str = strpool_inject( &name_pool, (char const*) value, strlen((char*) value) );
          else found_attr->value.str = strpool_inject( &value_pool, (char const*) value, strlen((char*) value) );
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
  
  STRPOOL_U64 test = strpool_inject( &value_pool, (char const*) appid, strlen(appid) );
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				/* try to find the first entry, by matching the APPID */
				if (!found){
					if (current->type == DXF_ATTR){
						if(current->value.group == 1001){
              if (current->value.str == test){
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

int dxf_ext_append(dxf_node *master, char *appid, int group, void *value, int pool){
	/* appdend new attrib on extended data, indicated by APPID */
	if (master){
		if (master->type == DXF_ENT){
			dxf_node *start, *end, *prev = NULL, *next = NULL;
			/* look for appid in master */
			if(dxf_find_ext_appid(master, appid, &start, &end)){
				int type = dxf_ident_attr_type(group);
				dxf_node *new_attr = dxf_attr_new(group, type, value, pool);
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

int ent_handle(dxf_drawing *drawing, dxf_node *element){
	int ok = 0;
	if (drawing && element){
		if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
			/* get current handle and increment the handle seed*/
			long int handle = 0;
			char hdl_str[DXF_MAX_CHARS+1];
      char nxt_hdl_str[DXF_MAX_CHARS+1];
			
			if (drawing->hand_seed){
				/* get the last handle value and convert to integer */
        handle = strtol(strpool_cstr2( &value_pool, drawing->hand_seed->value.str), NULL, 16);
				/* convert back to hexadecimal string, to write in element */
				snprintf(hdl_str, DXF_MAX_CHARS, "%X", handle);
				/* increment value of seed and write back */
        snprintf(nxt_hdl_str, DXF_MAX_CHARS, "%lX", handle + 1);
        drawing->hand_seed->value.str = strpool_inject( &value_pool, (char const*) nxt_hdl_str, strlen(nxt_hdl_str) );
			}
			
			/* change element handle */
			if (handle){
				int typ = dxf_ident_ent_type(element);
				if (typ == DXF_DIMSTYLE)
					ok = dxf_attr_change(element, 105, hdl_str);
				else
					ok = dxf_attr_change(element, 5, hdl_str);
			}
		}
	}
	
	return ok;
}

void drawing_ent_append(dxf_drawing *drawing, dxf_node *element){
	if (drawing && element){
		if (element->type != DXF_ENT) return;
		if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
			/* get current handle and increment the handle seed*/
			ent_handle(drawing, element);
			
			/* set handle to child entities */
			dxf_node *current= element->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					ent_handle(drawing, current);
				}
				current = current->next;
			}
			
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

dxf_node *dxf_find_handle(dxf_node *source, long int handle){
	dxf_node *current = NULL;
	dxf_node *prev = NULL, *curr_ent = NULL;
	long int id = 0;
	
	if (source){ 
		if (source->type == DXF_ENT){
			if (source->obj.content){
				curr_ent = source;
				current = source->obj.content->next;
				prev = current;
			}
		}
	}

	while (current){
		if (current->type == DXF_ENT){
			
			if (current->obj.content){
				/* starts the content sweep */
				curr_ent = current;
				current = current->obj.content->next;
				prev = current;
				
				continue;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			if(current->value.group == 5 || /* found regular handle */
			current->value.group == 105){/* or DIMENSION handle */
        id = strtol(strpool_cstr2( &value_pool, current->value.str), NULL, 16);
				if (id == handle) { /* success */
					return curr_ent;
				}
			}
		}
		
		current = current->next; /* go to the next in the list */
		/* ============================================================= */
		while (current == NULL){
			/* end of list sweeping */
			if ((prev == NULL) || (prev == source)){ /* stop the search if back on initial entity */
				
				current = NULL;
				break;
			}
			/* try to back in structure hierarchy */
			prev = prev->master;
			if (prev){ /* up in structure */
				/* try to continue on previous point in structure */
				current = prev->next;
				if(prev == source){
					current = NULL;
					break;
				}
				
			}
			else{ /* stop the search if structure ends */
				current = NULL;
				break;
			}
		}
	}
	
	return NULL;
}

list_node * dxf_ents_list(dxf_drawing *drawing, int pool_idx){
	dxf_node *current = NULL;
	list_node * list_ret = NULL;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		/* create the vector of returned values */
		list_ret = list_new(NULL, pool_idx);
		
		current = drawing->ents->obj.content->next;
		
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				list_push(list_ret, list_new((void *)current, pool_idx));
			}
			current = current->next;
		}
	}
	
	return list_ret;
}

dxf_drawing * dxf_get_dwg_list(list_node *list, char *xref_path){
	/* get a specific XREF drawing from list, by its path */
	if (list == NULL) return NULL;
	if (!xref_path) return NULL;
	
	struct dxf_xref *xref;
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			xref = (struct dxf_xref *)current->data;
			if (strcmp(xref->path, xref_path) == 0)
				/* image found */
				return xref->drwg;
		}
		current = current->next;
	}
	return NULL; /* fail the search */
}

dxf_drawing * dxf_xref_list(dxf_drawing *drawing, char *xref_path){
	/* load a XREF drawing */
	
	if (!xref_path) return NULL;
	if (!strlen (xref_path)) return NULL;
	if (!drawing) return NULL;
	
	dxf_node *current = NULL;
	
	/* init the drawing image list, if its void */
	if (drawing->xref_list == NULL)
		drawing->xref_list = list_new(NULL, DWG_LIFE);
	
	if (drawing->xref_list == NULL) return NULL; /* exit if fail */
	
	/* verify if xref is already loaded */
	dxf_drawing *drwg = dxf_get_dwg_list(drawing->xref_list, xref_path);
	if (drwg) return drwg; /* return if found */
	
	/* alloc a new xref element to put in list*/
	struct dxf_xref *xref = malloc(sizeof(struct dxf_xref));
	if (!xref) return NULL;
	
	/* alloc a new drawing structure structure */
	drwg = dxf_drawing_new(DWG_LIFE);
	if (!drwg){
		free(xref);
		return NULL;
	}
	
	/* load and apply the fonts required for drawing */
	drwg->font_list = drawing->font_list;
	drwg->dflt_font = drawing->dflt_font;
	drwg->dflt_fonts_path = drawing->dflt_fonts_path;
	
	/* load file in a temporay buffer */
	long file_size = 0;
	int prog = 0;
	struct Mem_buffer *file_buf = load_file_reuse(xref_path, &file_size);
	
	if (!file_buf) goto xref_error;
	
	/* parse the drawing */
	while (dxf_read (drwg, file_buf->buffer, file_size, &prog) > 0){
		
	}
	
	/* clear the file buffer */
	manage_buffer(0, BUF_RELEASE, 0);
	file_buf = NULL;
	file_size = 0;
	
	if (!drwg->ents || !drwg->main_struct) goto xref_error;
	
	/* store in main list */
	xref->drwg = drwg;
	strncpy(xref->path, xref_path, DXF_MAX_CHARS);
	
	list_node * new_node = list_new(xref, DWG_LIFE);
	list_push(drawing->xref_list, new_node);
	
	return drwg;
	
	/* some error */
	xref_error:
	free(xref);
	free(drwg);
	return NULL;
}

int dxf_xref_clear_list(dxf_drawing *drawing){
	/* free memory of loaded xrefs in drawing*/
	if(!drawing) return 0;
	if (drawing->xref_list == NULL) return 0;
	
	list_node *list = drawing->xref_list;
	
	struct dxf_xref *xref;
	dxf_drawing *drwg;
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			xref = (struct dxf_xref *)current->data;
			/* free each xref */
			drwg = xref->drwg;
			if (drwg){
				dxf_drawing_clear(drwg);
				free(drwg);
			}
			free(xref);
		}
		current = current->next;
	}
	drawing->xref_list = NULL; /* clear list head */
	
	return 1;
	
}

void dxf_xref_assemb (dxf_drawing *drawing){
	dxf_node *tmp_obj = NULL, *curr_blk = NULL;
	int xref = 0;
	char xref_path[DXF_MAX_CHARS+1];
	
	dxf_node *nxt_blk = NULL;
	while (curr_blk = dxf_find_obj_nxt(drawing->blks, &nxt_blk, "BLOCK")){ /* get the next block */
	
		/* verify if block is a external reference */
		if (tmp_obj = dxf_find_attr2(curr_blk, 70)){
			xref = tmp_obj->value.i_data & 4;
		}
		if (xref) {
			
			xref_path[0] = 0;
			if (tmp_obj = dxf_find_attr2(curr_blk, 1)){
        strncpy (xref_path, strpool_cstr2( &value_pool, tmp_obj->value.str), DXF_MAX_CHARS);
				dxf_xref_list(drawing, xref_path);
			}
			
		}
		if (!nxt_blk) break; /* end of BLOCKSs in table */
	}
}

int dxf_find_last_blk (dxf_drawing *drawing, char *mark){
	/* try to locate the last numbered block, match name starting with mark chars */
	if (!drawing) return 0;
	if (!drawing->blks) return 0;
	if (!mark) return 0;
  
  STRPOOL_U64 block = strpool_inject( &obj_pool, "BLOCK", strlen("BLOCK") );
	
	int len = 0;
	if (len = strlen(mark) == 0) return 0;
	
	char test_descr[DXF_MAX_CHARS+1];
	
	dxf_node *current, *descr_attr;
	int last = 1;
	int curr_num;
	
	/* sweep blocks list contents */
	current = drawing->blks->obj.content->next;
	while (current){
		if (current->type == DXF_ENT){ /* look for dxf entities */
			/* match blocks */
      if (current->obj.id == block){
				descr_attr = dxf_find_attr2(current, 2); /* look for descriptor in group 2 attribute */
				if (descr_attr){ /* found attribute */
					/* copy strings for secure manipulation */
          strncpy(test_descr, strpool_cstr2( &name_pool, descr_attr->value.str), DXF_MAX_CHARS);
					/* change to upper case */
					str_upp(test_descr);
					/* look for Block name starting by mark chars*/
					if (strlen(test_descr) > len){
						if (strncmp(test_descr, mark, len) == 0){
							/* convert the remain string to number */
							curr_num = atoi(test_descr+len);
							if (curr_num >= last){ /* update the last with greater number */
								last = curr_num + 1;
							}
						}
					}
				}
			}
		}
		current = current->next;
	}
	
	return last;
}