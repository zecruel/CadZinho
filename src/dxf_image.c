#include "dxf_image.h"

/* load and store raster images in drawing */

struct dxf_img_def { /* struct to store images indexed by DXF handle */
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
	/* free memory of loaded images in drawing*/
	if(!drawing) return 0;
	if (drawing->img_list == NULL) return 0;
	
	list_node *list = drawing->img_list;
	
	struct dxf_img_def * img_def;
	bmp_img * img;
	
	list_node *current = list->next;
	while (current != NULL){ /* sweep the list */
		if (current->data){
			img_def = (struct dxf_img_def *)current->data;
			/* free each image */
			img = img_def->img;
			if (img){
				free(img->buf);
				free(img);
			}
			free(img_def);
		}
		current = current->next;
	}
	drawing->img_list = NULL; /* clear list head */
	
	return 1;
	
}

bmp_img * dxf_image_def_list(dxf_drawing *drawing, dxf_node *img_def){
	/* load a raster image refered  in DXF IMAGE_DEF object */
	
	if(!img_def) return NULL;
	if(!drawing) return NULL;
	long id = 0;
	dxf_node *current = NULL;
	
	/* init the drawing image list, if its void */
	if (drawing->img_list == NULL)
		drawing->img_list = list_new(NULL, DWG_LIFE);
	
	if (drawing->img_list == NULL) return NULL; /* exit if fail */
	
	/* get IMAGE_DEF id handle */
	current = dxf_find_attr2(img_def, 5); 
	if (current)
		id = strtol(current->value.s_data, NULL, 16);
	if (!id) return NULL;
	
	/* verify if image is already loaded */
	struct dxf_img_def * img = get_img_list(drawing->img_list, id);
	if (img) return img->img; /* return if found */
	
	current = dxf_find_attr2(img_def, 1); /* get image path */
	if (!current) return NULL;
	
	/* alloc the node structure */
	img = malloc(sizeof(struct dxf_img_def));
	if (!img) return NULL;
	
	/* load image */
	img->img = bmp_load_img(current->value.s_data);
	if (!img->img){ 
		/*fail */
		free(img);
		return NULL;
	}
	
	/* store in main list */
	img->id = id;
	list_node * new_node = list_new(img, DWG_LIFE);
	list_push(drawing->img_list, new_node);
	
	return img->img;
}