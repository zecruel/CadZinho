#include "dxf_export.h"

void export_graph_hpgl(graph_obj * master, FILE *file, struct export_param param){
	/* convert a single graph object to HPGL commands*/
	
	if (master == NULL) return;
	if (file == NULL) return;
	
	
	if ((master->list->next) /* check if list is not empty */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !(master->flags & FILLED) ))//!master->fill))
	){
		int x0, y0, x1, y1;
		line_node *current = master->list->next;
		int prev_x, prev_y;
		int init = 0;
		
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = (int) ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = (int) ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){
				/* move to first point */
				fprintf(file, "PU;PA%d,%d;PD;", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y)))
				fprintf(file, "PU;PA%d,%d;PD;", x0, y0);

			fprintf(file, "PA%d,%d;", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		
	}
}

int export_list_hpgl(list_node *list, FILE *file, struct export_param param){
	/* convert a list of graph objects to HPGL commands */
	
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			/* proccess each element */
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				export_graph_hpgl(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int export_ents_hpgl(dxf_drawing *drawing, FILE *file , struct export_param param){
	/* convert the entities section of a drawing to HPGL commands*/
	
	dxf_node *current = NULL;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					/* init the HPGL file */
					if (!init){
						/* HPGL header */
						fprintf(file, "IN;DF;SP0;"
							"PU;"
							
						);
						init = 1;
					}
					/* fill file with HPGL drawing commands */
					export_list_hpgl(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the HPGL file */
		if (init) fprintf(file,"PU;");
	}
}

int export_hpgl(dxf_drawing *drawing, struct export_param param, char *dest){
	/* export a drawing to a HPGL file */
	
	if (!drawing) return 0;
	
	FILE *file = fopen(dest, "w"); /* open the file */
	
	if (!file) return 0; /* error on creating/opening file */
	
	/* scale to improve resolution on integer units */
	param.resolution = 1.0;
	
	/* write drawing HPGL commands to file */
	export_ents_hpgl(drawing, file, param);
	
	/* end */
	fclose(file);
	
	return 1;
}

void export_graph_gcode(graph_obj * master, FILE *file, struct export_param param){
	/* convert a single graph object to gcode commands*/
	
	if (master == NULL) return;
	if (file == NULL) return;
	
	
	if ((master->list->next) /* check if list is not empty */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !(master->flags & FILLED) ))//!master->fill))
	){
		double x0, y0, x1, y1;
		line_node *current = master->list->next;
		double prev_x, prev_y;
		int init = 0;
		
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (current->x0 - param.ofs_x) * param.scale;
			y0 = (current->y0 - param.ofs_y) * param.scale;
			x1 = (current->x1 - param.ofs_x) * param.scale;
			y1 = (current->y1 - param.ofs_y) * param.scale;
			
			if (init == 0){
				/* move to first point */
				fprintf(file, "%s\n", param.move);
				fprintf(file, "G00 X%g Y%g\n", x0, y0);
				fprintf(file, "%s\nG01 ", param.stroke);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y))){
				fprintf(file, "%s\n", param.move);
				fprintf(file, "G00 X%g Y%g\n", x0, y0);
				fprintf(file, "%s\nG01 ", param.stroke);
			}

			fprintf(file, "X%g Y%g\n", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		
	}
}

int export_list_gcode(list_node *list, FILE *file, struct export_param param){
	/* convert a list of graph objects to gcode commands */
	
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			/* proccess each element */
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				export_graph_gcode(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int export_ents_gcode(dxf_drawing *drawing, FILE *file , struct export_param param){
	/* convert the entities section of a drawing to gcode commands*/
	
	dxf_node *current = NULL;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					/* init the gcode file */
					if (!init){
						/* gcode header */
						if (strlen(param.init)){
							fprintf(file, param.init);
						}
						init = 1;
					}
					/* fill file with gcode drawing commands */
					export_list_gcode(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the gcode file */
		if (init) {
			if (strlen(param.end)){
				fprintf(file, param.end);
			}
		}
	}
}

int export_gcode(dxf_drawing *drawing, struct export_param param, char *dest){
	/* export a drawing to a gcode file */
	
	if (!drawing) return 0;
	
	FILE *file = fopen(dest, "w"); /* open the file */
	
	if (!file) return 0; /* error on creating/opening file */
	
	/* scale to improve resolution on integer units */
	param.resolution = 1.0;
	
	/* write drawing gcode commands to file */
	export_ents_gcode(drawing, file, param);
	
	/* end */
	fclose(file);
	
	return 1;
}