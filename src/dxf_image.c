#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"

struct dxf_img_def {
	long id;
	bmp_img *img;
};

struct dxf_img_def * get_img_list(list_node *list, long id){
/* get a specific image from list, by its id */
	if (list == NULL) return NULL;
	if (!id) return NULL;
	
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

int dxf_image_clear_list(dxf_drawing *drawing){
	if(!drawing) return 0;
	if (drawing->img_list == NULL) return 0;
	
	list_node *list = drawing->img_list;
	
	struct dxf_img_def * img_def;
	bmp_img * img;
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			img_def = (struct dxf_img_def *)current->data;
			img = img_def->img;
			if (img){
				free(img->buf);
				free(img);
			}
			free(img_def);
		}
		current = current->next;
	}
	
	drawing->img_list = NULL;
	
	return 1;
	
}

bmp_img * dxf_image_def_list(dxf_drawing *drawing, dxf_node *img_def){
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
	
	struct dxf_img_def * img = get_img_list(drawing->img_list, id);
	
	if (img) return img->img;
	
	current = dxf_find_attr2(img_def, 1); /* get image path */
	if (!current) return NULL;
	
	img = malloc(sizeof(struct dxf_img_def));
	if (!img) return NULL;
	
	img->img = bmp_load_img(current->value.s_data);
	if (!img->img){
		free(img);
		return NULL;
	}
	
	img->id = id;
	
	list_node * new_node = list_new(img, DWG_LIFE);
	list_push(drawing->img_list, new_node);
	
	return img->img;
}