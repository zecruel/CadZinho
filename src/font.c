#include "dxf.h"
#include "list.h"

dxf_tfont * font_load(char *name){
	
}

dxf_tfont * get_font_list(list_node *list, char *name){
	if (list == NULL) return NULL;
	if (name == NULL) return NULL;
	
	char nam[DXF_MAX_CHARS];
	dxf_tfont * font;
	
	strncpy(nam, name, DXF_MAX_CHARS);
	str_upp(nam);
	
	/* sweep the list */
	list_node *current = list->next;
	
	while (current != NULL){
		if (current->data){
			font = (dxf_tfont *)current->data;
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
	dxf_tfont * font;
	
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
		
	}
	else if (strncmp(ext, "ttf", DXF_MAX_CHARS) == 0){
		
	}
	else{
		
	}
	
}