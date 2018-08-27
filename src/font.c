#include "font.h"

struct tfont * font_load(char *name){
	
}

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

int add_font_list(list_node *list, char *name){
	if (list == NULL) return 0;
	if (name == NULL) return 0;
	
	char nam[DXF_MAX_CHARS], *ext;
	struct tfont * font;
	
	strncpy(nam, name, DXF_MAX_CHARS);
	str_upp(nam);
	
	ext = get_ext(nam);
	
	if (strlen(ext) == 0){
		strncat(nam, ".SHX", DXF_MAX_CHARS);
	}
	
	font = get_font_list(list, nam);
	if (font) return 0;
	
	ext = get_ext(nam);
	
	if (strncmp(ext, "shx", DXF_MAX_CHARS) == 0){
		shape * shx_tfont = shx_font_open(nam);
		if (shx_tfont){
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				shx_font_free(shx_tfont);
				return 0;
			}
			strncpy(font->path, nam, DXF_MAX_CHARS);
			strncpy(font->name, get_filename(nam), DXF_MAX_CHARS);
			font->type = FONT_SHP;
			font->data = shx_tfont;
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				shx_font_free(shx_tfont);
				free(font);
				return 0;
			}
			list_push(list, new_node);
		}
	}
	else if (strncmp(ext, "ttf", DXF_MAX_CHARS) == 0){
		struct tt_font * tt_tfont =  tt_init (nam);
		if (tt_tfont){
			font = malloc(sizeof(struct tfont));
			if (font == NULL) {
				tt_font_free(tt_tfont);
				return 0;
			}
			strncpy(font->path, nam, DXF_MAX_CHARS);
			strncpy(font->name, get_filename(nam), DXF_MAX_CHARS);
			font->type = FONT_TT;
			font->data = tt_tfont;
			list_node *new_node = list_new ((void *)font, PRG_LIFE);
			if (new_node == NULL) {
				tt_font_free(tt_tfont);
				free(font);
				return 0;
			}
			list_push(list, new_node);
		}
	}
	else{
		return 0;
	}
	
	return 1;

}