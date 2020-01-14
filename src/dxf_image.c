#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct dxf_img_def {
	long id;
	bmp_img *img;
};

struct dxf_img_def * get_img_list(list_node *list, long id){
/* get a specific image from list, by its id */
	if (list == NULL) return NULL;
	if (!id) return NULL;
	
	char nam[DXF_MAX_CHARS];
	struct dxf_img_def * img_def;
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			img_def = (struct dxf_img_def *)current->data;
			if (img_def->id == id)
				/* image found */
				return img_def;
		}
		current = current->next;
	}
	
	return NULL; /* fail the search */
	
}

struct dxf_img_def * dxf_image_def_list(dxf_drawing *drawing, dxf_node *img_def){
	if(!img_def) return NULL;
	if(!drawing) return NULL;
	long id = 0;
	
	dxf_node *current = NULL;

	if (drawing->img_list == NULL)
		drawing->img_list = list_new(NULL, DWG_LIFE);
	
	if (drawing->img_list == NULL) return NULL;
	
	current = dxf_find_attr2(img_def, 5); /* get id handle */
	if (current)
		id = strtol(current->value.s_data, NULL, 16);
	if (!id) return NULL;
	
	current = dxf_find_attr2(img_def, 1); /* get image path */
	if (!current) return NULL;
		//id = strtol(current->value.s_data, NULL, 16);

	int w, h, n;
	
	unsigned char *data = stbi_load(current->value.s_data, &w, &h, &n, 4);
	
}