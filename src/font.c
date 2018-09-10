#include "font.h"

struct tfont * get_font_list(list_node *list, char *name){
	if (list == NULL) return NULL;
	if (name == NULL) return NULL;
	
	char nam[DXF_MAX_CHARS];
	struct tfont * font;
	
	strncpy(nam, name, DXF_MAX_CHARS);
	str_upp(nam);
	
	/* sweep the list */
	list_node *current = list->next;
	
	while (current != NULL){
		if (current->data){
			font = (struct tfont *)current->data;
			if (strncmp(nam, font->name, DXF_MAX_CHARS) == 0)
				return font;
			
		}
		current = current->next;
	}
	
	return NULL;
	
}


struct tfont * add_font_list(list_node *list, char *path, char *opt_dirs){
	if (list == NULL) return NULL;
	if (path == NULL) return NULL;
	
	char *name, *ext, full_path[DXF_MAX_CHARS];
	struct tfont * font = NULL;
	
	strncpy(full_path, path, DXF_MAX_CHARS);
	ext = get_ext(full_path);
	
	if (strlen(ext) == 0){
		strncat(full_path, ".SHX", DXF_MAX_CHARS);
	}
	
	if (!file_exists(full_path)){
		if (opt_dirs){
			char dirs[DXF_MAX_CHARS];
			char try_path[DXF_MAX_CHARS] = "";
			char s[2] = {PATH_SEPARATOR, 0};
			char *token;
			int ok = 0;
			strncpy(dirs, opt_dirs, DXF_MAX_CHARS);
			/* get the first token */
			token = strtok(dirs, s);
			
			/* walk through other tokens */
			while( token != NULL ) {
				try_path[0] = 0;
				strncat(try_path, token, DXF_MAX_CHARS);
				
				if (try_path[strlen(try_path) - 1] != DIR_SEPARATOR){
					char sep[2] = {DIR_SEPARATOR, 0};
					strncat(try_path, sep, DXF_MAX_CHARS);
				}
				
				strncat(try_path, full_path, DXF_MAX_CHARS);
				if (file_exists(try_path)){
					strncpy(full_path, try_path, DXF_MAX_CHARS);
					ok = 1;
					break;
				}
				
				token = strtok(NULL, s);
			}
			if (!ok) return NULL;
			
		}
		else return NULL;
	}
	
	name = get_filename(full_path);
	str_upp(name);
	
	font = get_font_list(list, name);
	if (font) return font;
	
	ext = get_ext(full_path);
	
	if (strncmp(ext, "shx", DXF_MAX_CHARS) == 0){
		shape * shx_tfont = shx_font_open(full_path);
		if (shx_tfont){
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				shx_font_free(shx_tfont);
				return NULL;
			}
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_SHP;
			font->data = shx_tfont;
			font->std_size = 12.0;
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				shx_font_free(shx_tfont);
				free(font);
				return NULL;
			}
			list_push(list, new_node);
		}
	}
	else if (strncmp(ext, "ttf", DXF_MAX_CHARS) == 0){
		struct tt_font * tt_tfont =  tt_init (full_path);
		if (tt_tfont){
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				tt_font_free(tt_tfont);
				return NULL;
			}
			strncpy(font->path, full_path, DXF_MAX_CHARS);
			strncpy(font->name, name, DXF_MAX_CHARS);
			font->type = FONT_TT;
			font->data = tt_tfont;
			font->std_size = 12.0;
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				tt_font_free(tt_tfont);
				free(font);
				return NULL;
			}
			list_push(list, new_node);
		}
	}
	else{
		return NULL;
	}
	
	return font;

}

int free_font_list(list_node *list){
	if (list == NULL) return 0;
	
	struct tfont * font;
	int type;
	
	/* sweep the list */
	list_node *current = list->next;
	
	while (current != NULL){
		if (current->data){
			font = (struct tfont *)current->data;
			type = font->type;
			
			if(type == FONT_SHP){
				shx_font_free((shape *)font->data);
			}
			else if (type == FONT_TT){
				tt_font_free((struct tt_font *)font->data);
			}
			
			free(font);
			
		}
		current = current->next;
	}
	
	return 1;

}

int font_parse_str(struct tfont * font, list_node *list_ret, int pool_idx, char *txt, double *w){
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
					graph_modify(graph, 0.0, 0.0, txt_size, txt_size, 0.0);
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
	if (!font || !w|| !txt) return 0;
	
	int ok = 0;
	int type = font->type;
	list_node * graph = list_new(NULL, FRAME_LIFE);
	
	if (!graph) return 0;
	
	*w = 0.0;
	
	font_parse_str(font, graph, FRAME_LIFE, txt, w);
	if(type == FONT_SHP){
		shape *shx_font = font->data;
		/* find the dimentions of SHX font */
		if(shx_font->next){ /* the font descriptor is stored in first iten of list */
			if(shx_font->next->cmd_size > 1){ /* check if the font is valid */
				 double fnt_above = shx_font->next->cmds[0]; /* size above the base line of text */
				//double fnt_below = shx_font->next->cmds[1]; /* size below the base line of text */
				
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