#include "font.h"

struct tfont * get_font_list(list_node *list, char *name){
/* get a specific font from list, by its name */
	if (list == NULL) return NULL;
	if (name == NULL) return NULL;
	
	char nam[DXF_MAX_CHARS];
	struct tfont * font;
	
	strncpy(nam, name, DXF_MAX_CHARS); /* preserve original string */
	str_upp(nam); /* change to upper case */
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			font = (struct tfont *)current->data;
			if (strncmp(nam, font->name, DXF_MAX_CHARS) == 0)
				/* font found */
				return font;
			
		}
		current = current->next;
	}
	
	return NULL; /* fail the search */
	
}


struct tfont * add_font_list(list_node *list, char *path, char *opt_dirs){
/* add to list a font from path or name*/
	if (list == NULL) return NULL;
	if (path == NULL) return NULL;
	
	char *name, *ext, full_path[DXF_MAX_CHARS];
	struct tfont * font = NULL;
	
	strncpy(full_path, path, DXF_MAX_CHARS); /* preserve original string */
	ext = get_ext(full_path); /* get file extension to determine type of font */
	
	if (strlen(ext) == 0){ /* if not have extension */
		/* presume SHX font */
		strncat(full_path, ".SHX", DXF_MAX_CHARS);
	}
	
	/* try to open file */
	if (!file_exists(full_path)){
		/* if file not found */
		if (opt_dirs){ /* try to explore optional font directories */
			char dirs[DXF_MAX_CHARS];
			char try_path[DXF_MAX_CHARS] = "";
			/* font directories are in single string, separated by system mark */
			char s[2] = {PATH_SEPARATOR, 0};
			char *token;
			int ok = 0;
			strncpy(dirs, opt_dirs, DXF_MAX_CHARS);
			
			/* process each directory */
			token = strtok(dirs, s); /* get the first token */
			while( token != NULL ) { /* walk through other tokens */
				try_path[0] = 0;
				strncat(try_path, token, DXF_MAX_CHARS);
				
				/* add directory separator from filename, if its not present */
				if (try_path[strlen(try_path) - 1] != DIR_SEPARATOR){
					char sep[2] = {DIR_SEPARATOR, 0};
					strncat(try_path, sep, DXF_MAX_CHARS);
				}
				
				strncat(try_path, full_path, DXF_MAX_CHARS);
				if (file_exists(try_path)){ /* try to open file */
					strncpy(full_path, try_path, DXF_MAX_CHARS);
					ok = 1;
					break; /*success */
				}
				
				token = strtok(NULL, s);
			}
			if (!ok) return NULL; /* fail open font */
			
		}
		else return NULL; /* fail open font */
	}
	
	name = get_filename(full_path); /* font name is the filename*/
	str_upp(name); /* upper case the name */
	
	font = get_font_list(list, name); /* verify if font was previously loaded */
	if (font) return font;
	
	ext = get_ext(full_path); /* get file extension to determine type of font */
	
	if (strncmp(ext, "shx", DXF_MAX_CHARS) == 0){ /* SHX font */
		shape * shx_tfont = shx_font_open(full_path);
		if (shx_tfont){
			/* alloc the structures */
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				/* fail in allocation*/
				shx_font_free(shx_tfont);
				return NULL;
			}
			
			if(shx_tfont->next){ /* check if the font is valid */
				double fnt_above = shx_tfont->next->cmds[0]; /* size above the base line of text */
				double fnt_below = shx_tfont->next->cmds[1]; /* size below the base line of text */
				
				if(fnt_above > 0) font->below = fabs(fnt_below/fnt_above);
				font->above = 1.0;
			}
			
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_SHP;
			font->data = shx_tfont;
			font->std_size = 12.0;
			/* add to list */
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				/* fail to add in list*/
				shx_font_free(shx_tfont);
				free(font);
				return NULL;
			}
			list_push(list, new_node);
		}
	}
	else if (strncmp(ext, "ttf", DXF_MAX_CHARS) == 0){ /* True type font */
		struct tt_font * tt_tfont =  tt_init (full_path);
		if (tt_tfont){
			/* alloc the structures */
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				/* fail in allocation*/
				tt_font_free(tt_tfont);
				return NULL;
			}
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_TT;
			font->data = tt_tfont;
			
			font->above = fabs(tt_tfont->ascent);
			font->below = fabs(tt_tfont->descent);
			
			font->std_size = 12.0;
			/* add to list */
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				/* fail to add in list*/
				tt_font_free(tt_tfont);
				free(font);
				return NULL;
			}
			list_push(list, new_node);
		}
	}
	else{ /* type not reconized */
		return NULL;
	}
	
	return font;

}

int free_font_list(list_node *list){
/* free fonts in list */
	if (list == NULL) return 0;
	
	struct tfont * font;
	int type;
	
	/* sweep the list */
	list_node *current = list->next;
	while (current != NULL){
		if (current->data){
			/* select type of font */
			font = (struct tfont *)current->data;
			type = font->type;
			
			if(type == FONT_SHP){
				shx_font_free((shape *)font->data);
			}
			else if (type == FONT_TT){
				tt_font_free((struct tt_font *)font->data);
			}
			
			/* free main struct*/
			free(font);
			
		}
		current = current->next;
	}
	
	return 1;

}

int font_parse_str(struct tfont * font, list_node *list_ret, int pool_idx, char *txt, double *w){
/* parse string and return list of grpahs */
	if (!font || !list_ret || !txt) return 0;
	
	int num_graph = 0;
	int type = font->type;
	list_node *new_node = NULL;
	
	double fnt_size, fnt_above, fnt_below, txt_size;
	
	if(type == FONT_SHP){
		shape *shx_font = font->data;
		/* find the dimentions of SHX font */
		if(shx_font->next){ /* the font descriptor is stored in first iten of list */
			if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
				fnt_above = shx_font->next->cmds[0]; /* size above the base line of text */
				fnt_below = shx_font->next->cmds[1]; /* size below the base line of text */
				if((fnt_above + fnt_below) > 0){
					fnt_size = fnt_above + fnt_below;
				}
				if(fnt_above > 0) txt_size = 1/fnt_above;

				graph_obj * graph = shx_font_parse(shx_font , pool_idx, txt, w);
				
				if (graph) new_node = list_new ((void *)graph, pool_idx);
				if (new_node){
					/*scale the result graph to unit size */
					graph_modify(graph, 0.0, 0.0, txt_size, txt_size, 0.0);
					/* add to list */
					list_push(list_ret, new_node);
					num_graph++;
				}
			}
		}
		
	}
	else if (type == FONT_TT){
		num_graph += tt_parse_str((struct tt_font *) font->data, list_ret, pool_idx, txt, w);
	}
	
	return num_graph;
}

int font_str_w(struct tfont * font, char *txt, double *w){
/* get width of an string*/
	if (!font || !w|| !txt) return 0;
	
	int ok = 0;
	int type = font->type;
	list_node * graph = list_new(NULL, FRAME_LIFE);
	
	if (!graph) return 0;
	
	*w = 0.0;
	
	font_parse_str(font, graph, FRAME_LIFE, txt, w); /* temporary list */
	if(type == FONT_SHP){
		shape *shx_font = font->data;
		/* find the dimentions of SHX font */
		if(shx_font->next){ /* the font descriptor is stored in first iten of list */
			if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
				 double fnt_above = shx_font->next->cmds[0]; /* size above the base line of text */
				//double fnt_below = shx_font->next->cmds[1]; /* size below the base line of text */
				
				/* scale width to unit size */
				if(fnt_above > 0){
					*w /= fnt_above;
					ok =1;
				}
				
			}
		}
		
	}
	else if (type == FONT_TT){
		ok =1;
	}
	
	
	
	return ok;
}