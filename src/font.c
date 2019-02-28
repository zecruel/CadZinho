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

struct tfont * get_font_list2(list_node *list, char *name){
/* get a specific font from list, by its name without extension*/
	if (list == NULL) return NULL;
	if (name == NULL) return NULL;
	
	char nam[DXF_MAX_CHARS], nam_cmp[DXF_MAX_CHARS];
	struct tfont * font;
	
	strncpy(nam, name, DXF_MAX_CHARS); /* preserve original string */
	strip_ext(nam); /* strip extension, if exist */
	str_upp(nam); /* change to upper case */
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			font = (struct tfont *)current->data;
			strncpy(nam_cmp, font->name, DXF_MAX_CHARS); /* preserve original string */
			strip_ext(nam_cmp); /* strip extension */
			if (strncmp(nam, nam_cmp, DXF_MAX_CHARS) == 0)
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
	
	name = get_filename(full_path); /* font name is the filename*/
	str_upp(name); /* upper case the name */
	
	font = get_font_list(list, name); /* verify if font was previously loaded */
	if (font) return font;
	
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
	
	ext = get_ext(full_path); /* get file extension to determine type of font */
	
	if (strncmp(ext, "shx", DXF_MAX_CHARS) == 0){ /* Shape font - binary format */
		shp_typ * shp_tfont = shp_font_open(full_path);
		if (shp_tfont){
			/* alloc the structures */
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				/* fail in allocation*/
				shp_font_free(shp_tfont);
				return NULL;
			}
			
			if(shp_tfont->next){ /* check if the font is valid */
				double fnt_above = shp_tfont->next->cmds[0]; /* size above the base line of text */
				double fnt_below = shp_tfont->next->cmds[1]; /* size below the base line of text */
				
				if(fnt_above > 0) font->below = fabs(fnt_below/fnt_above);
				font->above = 1.0;
			}
			
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_SHP;
			font->data = shp_tfont;
			font->std_size = 12.0;
			font->unicode = shp_tfont->unicode;
			/* add to list */
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				/* fail to add in list*/
				shp_font_free(shp_tfont);
				free(font);
				return NULL;
			}
			list_push(list, new_node);
		}
	}
	else if (strncmp(ext, "shp", DXF_MAX_CHARS) == 0){ /* Shape font - text format */
		long fsize = 0;
		char *buf = dxf_load_file(full_path, &fsize);
		shp_typ * shp_tfont = shp_font_load(buf);
		if (buf) free(buf);
		if (shp_tfont){
			/* alloc the structures */
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				/* fail in allocation*/
				shp_font_free(shp_tfont);
				return NULL;
			}
			
			if(shp_tfont->next){ /* check if the font is valid */
				double fnt_above = shp_tfont->next->cmds[0]; /* size above the base line of text */
				double fnt_below = shp_tfont->next->cmds[1]; /* size below the base line of text */
				
				if(fnt_above > 0) font->below = fabs(fnt_below/fnt_above);
				font->above = 1.0;
			}
			
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_SHP;
			font->data = shp_tfont;
			font->std_size = 12.0;
			font->unicode = shp_tfont->unicode;
			/* add to list */
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				/* fail to add in list*/
				shp_font_free(shp_tfont);
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
			font->unicode = 1;
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

struct tfont * add_shp_font_list(list_node *list, char *name, char *buf){
/* add to list a shape font from string*/
	if (list == NULL) return NULL;
	if (buf == NULL) return NULL;
	if (name == NULL) return NULL;
	
	struct tfont * font = NULL;
	
	font = get_font_list(list, name); /* verify if font was previously loaded */
	if (font) return font;
	
	shp_typ * shp_tfont = shp_font_load(buf);
	if (shp_tfont){
		/* alloc the structures */
		font = malloc(sizeof(struct tfont));
		if (font == NULL) {
			/* fail in allocation*/
			shp_font_free(shp_tfont);
			return NULL;
		}
		
		font->above = 1.0;
		font->below = 0.0;
		
		/* find the dimentions of SHX font */
		shp_typ *fnt_descr = shp_font_find(shp_tfont, 0); /* font descriptor is in 0 codepoint */
		if (fnt_descr){
			if(fnt_descr->cmd_size > 1){ /* check if the font is valid */
				double fnt_above = fnt_descr->cmds[0]; /* size above the base line of text */
				double fnt_below = fnt_descr->cmds[1]; /* size below the base line of text */
		
				if(fnt_above > 0) font->below = fabs(fnt_below/fnt_above);
			}
		}
		
		strncpy(font->path, "internal", DXF_MAX_CHARS);
		strncpy(font->name, name, DXF_MAX_CHARS);
		str_upp(font->name); /* upper case the name */
		font->type = FONT_SHP;
		font->data = shp_tfont;
		font->std_size = 12.0;
		font->unicode = shp_tfont->unicode;
		/* add to list */
		list_node *new_node = list_new ((void *)font, PRG_LIFE);
		if (new_node == NULL) {
			/* fail to add in list*/
			shp_font_free(shp_tfont);
			free(font);
			return NULL;
		}
		list_push(list, new_node);
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
				shp_font_free((shp_typ *)font->data);
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

int font_parse_str(struct tfont * font, list_node *list_ret, int pool_idx, char *txt, double *w, int force_ascii){
/* parse string and return list of graphs */
	if (!font || !list_ret || !txt) return 0;
	
	int num_graph = 0;
	int type = font->type;
	list_node *new_node = NULL;
	
	double fnt_size, fnt_above, fnt_below, txt_size;
	
	if(type == FONT_SHP){
		shp_typ *shp_font = font->data;
		num_graph += shp_parse_str(shp_font, list_ret, pool_idx, txt, w, force_ascii);
	}
	else if (type == FONT_TT){
		num_graph += tt_parse_str((struct tt_font *) font->data, list_ret, pool_idx, txt, w, force_ascii);
	}
	
	return num_graph;
}

graph_obj * font_parse_cp(struct tfont * font, int cp, int prev_cp, int pool_idx, double *w){
/* parse single code point to graph*/
	if (!font || !cp) return NULL;
	int type = font->type;
	graph_obj *curr_graph = NULL;
	
	//double fnt_size, fnt_above, fnt_below, txt_size;
	
	if(type == FONT_SHP){
		/* find the dimentions of SHX font */
		double fnt_above = 1.0 , fnt_below = 0.0, fnt_size = 1.0, txt_size = 1.0;
		shp_typ *fnt_descr = shp_font_find((shp_typ *) font->data, 0); /* font descriptor is in 0 codepoint */
		
		if (fnt_descr){
			if(fnt_descr->cmd_size > 1){ /* check if the font is valid */
				fnt_above = fnt_descr->cmds[0]; /* size above the base line of text */
				fnt_below = fnt_descr->cmds[1]; /* size below the base line of text */
				if((fnt_above + fnt_below) > 0){
					fnt_size = fnt_above + fnt_below;
				}
				if(fnt_above > 0) txt_size = 1/fnt_above;
			}
		}
		curr_graph = shp_parse_cp((shp_typ *) font->data, pool_idx, cp, w);
		*w *= txt_size;
		if (curr_graph){
			graph_modify(curr_graph, 0.0, 0.0, txt_size, txt_size, 0.0);
		}
	}
	else if (type == FONT_TT){
		curr_graph =  tt_parse_cp((struct tt_font *) font->data, cp, prev_cp, pool_idx, w);
	}
	
	return curr_graph;
}

int font_str_w(struct tfont * font, char *txt, double *w, int force_ascii){
/* get width of an string */
	if (!font || !w|| !txt) return 0;
	
	int type = font->type;
	
	if(type == FONT_SHP){
		list_node * graph = list_new(NULL, FRAME_LIFE);
		
		if (!graph) return 0;
		
		*w = 0.0;
		
		font_parse_str(font, graph, FRAME_LIFE, txt, w, force_ascii); /* temporary list */
	}
	else if (type == FONT_TT)
		tt_w_str((struct tt_font *) font->data, txt, w, force_ascii);
	
	if(fabs(*w) > 1e-9) return 1;
	else return 0;
}