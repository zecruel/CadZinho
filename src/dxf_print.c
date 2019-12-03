#include "dxf_print.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void print_graph_pdf(graph_obj * master, struct txt_buf *buf, struct print_param param){
	/* convert a single graph object to pdf commands*/
	
	if (master == NULL) return;
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !master->fill))
	){
		int x0, y0, x1, y1, i;
		line_node *current = master->list->next;
		int tick = 0, prev_x, prev_y;
		int init = 0;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = (int) ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = (int) ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){
				/* set the pattern */
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"[");
				if(master->patt_size > 1){
					int patt_el = (int) fabs(master->pattern[0] * param.scale * param.resolution) + 1;
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d", patt_el);
					for (i = 1; i < master->patt_size; i++){
						patt_el = (int) fabs(master->pattern[i] * param.scale * param.resolution) + 1;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							" %d", patt_el);
					}
				}
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"] 0 d ");
				
				/* set the tickness */
				if (master->thick_const) 
					tick = (int) round(master->tick * param.resolution);
				else tick = (int) round(master->tick * param.scale * param.resolution);
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d w ", tick); /*line width */
				
				/* set the color */
				if (!param.mono){
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%.4g %.4g %.4g RG ",
						(float)color.r/255,
						(float)color.g/255,
						(float)color.b/255);
					if (master->fill) /* check if object is filled */
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%.4g %.4g %.4g rg ",
							(float)color.r/255,
							(float)color.g/255,
							(float)color.b/255);
				}
				
				/* move to first point */
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d m ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y)))
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d m ", x0, y0);

			buf->pos +=snprintf(buf->data + buf->pos,
				PDF_BUF_SIZE - buf->pos,
				"%d %d l ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		if (master->fill) /* check if object is filled */
			buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"f*\r\n");
		
		else buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"S\r\n");
	}
}

int print_list_pdf(list_node *list, struct txt_buf *buf, struct print_param param){
	/* convert a list of graph objects to pdf commands*/
	
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
				print_graph_pdf(curr_graph, buf, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_pdf(dxf_drawing *drawing, struct txt_buf *buf , struct print_param param){
	/* convert the entities section of a drawing to pdf commands*/
	
	dxf_node *current = NULL;
	
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* sweep entities section */
		while (current != NULL){
			if (current->type == DXF_ENT){ /* DXF entity */
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					if (!init){ /* init pdf object */
						double res;
						if (param.resolution > 0.0) res = 1/param.resolution;
						else res = 1.0;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"0.0 0.0 0.0 RG " /*set to black */
							//"1 J " /*line cap style - round*/
							/* transformation matrix */
							"%.4g 0.0 0.0 %.4g 0.0 0.0 cm\r\n", res, res);
						init = 1;
					}
					/* proccess each element */
					print_list_pdf(current->obj.graphics, buf, param);
				}
			}
			current = current->next;
		}
	}
}

int print_pdf(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a pdf file */
	
	if (!drawing) return 0;
	
	/* buffer to hold the pdf commands from drawing convertion */
	struct txt_buf *buf = malloc(sizeof(struct txt_buf));
	if (!buf) return 0;
	buf->pos = 0; /* init buffer */
	
	/* resolution -> multiplier factor over integer units in pdf */
	param.resolution = 20;
	
	/* multiplier to fit pdf parameters in final output units */
	double mul = 1.0;
	if (param.unit == PRT_MM)
		mul = 72.0 / 25.4;
	else if (param.unit == PRT_IN)
		mul = 72.0;
	else if (param.unit == PRT_PX)
		mul = 72.0/96.0;
	
	param.w = mul * param.w + 0.5;
	param.h = mul * param.h + 0.5;
	param.scale *= mul;
	
	/* fill buffer with pdf drawing commands */
	print_ents_pdf(drawing, buf, param);
	
	/* -------------- compress the command buffer stream (deflate algorithm)*/
	int cmp_status;
	long src_len = strlen(buf->data);
	long cmp_len = compressBound(src_len);
	/* Allocate buffers to hold compressed and uncompressed data. */
	mz_uint8 *pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	if (!pCmp) {
		free(buf);
		return 0;
	}
	/* Compress buffer string. */
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf->data, src_len);
	if (cmp_status != Z_OK){
		free(pCmp);
		free(buf);
		return 0;
	}
	/*-------------------------*/
	
	/*--------------- assemble the pdf structure to a file */
	/* pdf info header */
	struct pdf_info info = {
		.creator = "CadZinho",
		.producer = "CadZinho",
		.title = "Print Drawing",
		.author = "Ze Cruel",
		.subject = "Print Drawing",
		.date = ""
	};
	/* pdf main struct */
	struct pdf_doc *pdf = pdf_create((int)param.w, (int)param.h, &info);
	
	/* add a page to pdf file */
	struct pdf_object *page = pdf_append_page(pdf);
	
	/* add the print objet to pdf page */
	//pdf_add_stream(pdf, page, buf->data); /* non compressed stream */
	pdf_add_stream_zip(pdf, page, pCmp, cmp_len); /* compressed stream */
	
	/* save pdf file */ 
	int e = pdf_save(pdf, dest);
	/*-------------------------*/
	
	/* clear and safe quit */
	pdf_destroy(pdf);
	free(pCmp);
	free(buf);
	if (e) return 0; /* error in file creation */
	
	return 1;
}


void print_graph_svg(graph_obj * master, FILE *file, struct print_param param){
	/* convert a single graph object to svg commands*/
	
	if (master == NULL) return;
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !master->fill))
	){
		double x0, y0, x1, y1;
		line_node *current = master->list->next;
		double tick = 0, prev_x, prev_y;
		int init = 0, i;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = param.h * param.resolution - ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = param.h * param.resolution - ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){ /* init the svg path */
				fprintf(file,"<path ");
				
				/* set the pattern */
				if(master->patt_size > 1){
					/* get pattern lenght */
					double patt_len = 0;
					for (i = 0; i < master->patt_size; i++){
						patt_len += fabs(master->pattern[i] * param.scale * param.resolution);
					}
					
					/* assemble pattern array */
					if ((patt_len / master->patt_size) >= 1.0){
					
						fprintf(file,"stroke-dasharray=\"");
						
						double patt_el = fabs(master->pattern[0] * param.scale * param.resolution);
						if (patt_el < 1.0) patt_el = 1.0;
						fprintf(file, "%.4g", patt_el);
						for (i = 1; i < master->patt_size; i++){
							patt_el = fabs(master->pattern[i] * param.scale * param.resolution);
							if (patt_el < 1.0) patt_el = 1.0;
							fprintf(file, ",%.4g", patt_el);
						}
						
						fprintf(file,"\" ");
					}
				}
				
				/* set the tickness */
				if (master->thick_const) tick = (master->tick);// * param.resolution);
				else tick = (master->tick * param.scale);// * param.resolution);
				if (tick >= 1.0) fprintf(file, "stroke-width=\"%0.1f\" ", tick);
				
				/* set the color */
				if (!param.mono){
					
					if (master->fill) /* check if object is filled */
						fprintf(file, "fill=\"rgb(%d, %d, %d)\" stroke=\"none\" ",
							color.r, color.g, color.b);
					else
						fprintf(file, "stroke=\"rgb(%d, %d, %d)\" fill=\"none\" ",
							color.r, color.g, color.b);
				}
				else {
					
					if (master->fill) /* check if object is filled */
						fprintf(file, "fill=\"black\" stroke=\"none\" ");
					else
						fprintf(file, "stroke=\"black\" fill=\"none\" ");
				}
				
				/* move to first point */
				fprintf(file, "d=\"M%.4g %.4g ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if ((fabs(x0 - prev_x) > 1e-4) || (fabs(y0 - prev_y) > 1e-4))
				fprintf(file, "M%.4g %.4g ", x0, y0);

			fprintf(file, "L%.4g %.4g ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		fprintf(file, "\"/>\n");
	}
}

int print_list_svg(list_node *list, FILE *file, struct print_param param){
	/* convert a list of graph objects to svg commands*/
	
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
				print_graph_svg(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_svg(dxf_drawing *drawing, FILE *file , struct print_param param){
	/* convert the entities section of a drawing to svg commands*/
	
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
					/* init the main svg object */
					if (!init){
						/* write the svg header */
						fprintf(file, "<svg width=\"%0.4f\" height=\"%0.4f\" "
							"version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" "
							"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
							param.w * param.resolution,
							param.h * param.resolution);
						init = 1;
					}
					/* proccess each entity */
					print_list_svg(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the main svg object */
		if (init) fprintf(file,"</svg>");
	}
}

int print_svg(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a svg file */
	
	if (!drawing) return 0;
	
	/* open the file for writing */
	FILE *file = fopen(dest, "w");
	
	if (!file) return 0; /* error on creating/opening file */
	param.resolution = 4.0;
	
	/* fill buffer with svg drawing commands */
	print_ents_svg(drawing, file, param);
	
	fclose(file);
	
	return 1;
}

void print_graph_ps(graph_obj * master, FILE *file, struct print_param param){
	/* convert a single graph object to postscript commands*/
	
	if (master == NULL) return;
	
	/* verify if graph bounds are in visible page area */
	double b_x0, b_y0, b_x1, b_y1;
	b_x0 = (master->ext_min_x - param.ofs_x) * param.scale * param.resolution;
	b_y0 = (master->ext_min_y - param.ofs_y) * param.scale * param.resolution;
	b_x1 = (master->ext_max_x - param.ofs_x) * param.scale * param.resolution;
	b_y1 = (master->ext_max_y - param.ofs_y) * param.scale * param.resolution;
	
	rect_pos pos_p0 = rect_find_pos(b_x0, b_y0, 0, 0, param.w * param.resolution, param.h * param.resolution);
	rect_pos pos_p1 = rect_find_pos(b_x1, b_y1, 0, 0, param.w * param.resolution, param.h * param.resolution);
	
	if ((master->list->next) /* check if list is not empty */
		&& (!(pos_p0 & pos_p1)) /* and in bounds of page */
		/* and too if is drawable pattern */
		&& (!(master->patt_size <= 1 && master->pattern[0] < 0.0 && !master->fill))
	){
		int x0, y0, x1, y1, i;
		line_node *current = master->list->next;
		int tick = 0, prev_x, prev_y;
		int init = 0;
		
		/* verify if color is substituted  */
		bmp_color color = validate_color(master->color, param.list, param.subst, param.len);
		
		while(current){ /* draw the lines - sweep the list content */
			
			/* apply the scale and offset */
			x0 = (int) ((current->x0 - param.ofs_x) * param.scale * param.resolution);
			y0 = (int) ((current->y0 - param.ofs_y) * param.scale * param.resolution);
			x1 = (int) ((current->x1 - param.ofs_x) * param.scale * param.resolution);
			y1 = (int) ((current->y1 - param.ofs_y) * param.scale * param.resolution);
			
			if (init == 0){
				/* set the pattern */
				fprintf(file, "n [");
				if(master->patt_size > 1){
					int patt_el = (int) fabs(master->pattern[0] * param.scale * param.resolution);
					if(patt_el < param.resolution) patt_el = param.resolution;
					fprintf(file, "%d", patt_el);
					for (i = 1; i < master->patt_size; i++){
						patt_el = (int) fabs(master->pattern[i] * param.scale * param.resolution) + 1;
						if(patt_el < param.resolution) patt_el = param.resolution;
						fprintf(file, " %d", patt_el);
					}
				}
				fprintf(file, "] 0 d ");
				
				/* set the tickness */
				if (master->thick_const) 
					tick = (int) round(master->tick * param.resolution);
				else tick = (int) round(master->tick * param.scale * param.resolution);
				fprintf(file, "%d lw ", tick); /*line width */
				
				/* set the color */
				if (!param.mono){
					fprintf(file, "%.4g %.4g %.4g rg ",
						(float)color.r/255,
						(float)color.g/255,
						(float)color.b/255);
				}
				
				/* move to first point */
				fprintf(file, "%d %d m ", x0, y0);
				prev_x = x0;
				prev_y = y0;
				init = 1;
			}
			/*finaly, draw current line */
			else if (((x0 != prev_x)||(y0 != prev_y)))
				fprintf(file, "%d %d m ", x0, y0);

			fprintf(file, "%d %d l ", x1, y1);
		
			prev_x = x1;
			prev_y = y1;
			
			current = current->next; /* go to next line */
		}
		/* stroke the graph */
		if (master->fill) /* check if object is filled */
			fprintf(file, "f\n");
		
		else fprintf(file, "s\n");
	}
}


int print_list_ps(list_node *list, FILE *file, struct print_param param){
	/* convert a list of graph objects to postscript commands */
	
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
				print_graph_ps(curr_graph, file, param);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_ps(dxf_drawing *drawing, FILE *file , struct print_param param){
	/* convert the entities section of a drawing to postscript commands*/
	
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
				
					/* init the postscript file */
					if (!init){
						/* postscript header */
						fprintf(file, "%%!PS\n\n"
							"/W %.9g def\n"
							"/H %.9g def\n"
							/* set some alias to minimize text size */
							"/n {newpath} bind def\n"
							"/m {moveto} bind def\n"
							"/l {lineto} bind def\n"
							"/cp {closepath} bind def\n"
							"/s {stroke} bind def\n"
							"/f {fill} bind def\n"
							"/d {setdash} bind def\n"
							"/lw {setlinewidth} bind def\n"
							"/rg {setrgbcolor} bind def\n"
							/* set page size (for convertion programs) */
							"<< /PageSize [W H] >> setpagedevice\n"
							/* set clip area */
							"n 0 0 m W 0 l W H l 0 H l cp clip\n\n"
							/* scale to improve resolution on integer units */
							"%.9g dup scale\n\n",
							param.w , param.h,
							1/param.resolution
						);
						init = 1;
					}
					/* fill file with postscript drawing commands */
					print_list_ps(current->obj.graphics, file, param);
				}
			}
			current = current->next;
		}
		/* end the postscript file */
		if (init) fprintf(file,"\nshowpage\n");
	}
}

int print_ps(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a postscript file */
	
	if (!drawing) return 0;
	
	FILE *file = fopen(dest, "w"); /* open the file */
	
	if (!file) return 0; /* error on creating/opening file */
	
	/* scale to improve resolution on integer units */
	param.resolution = 20;
	
	/* multiplier to fit postscript parameters in final output units */
	double mul = 1.0;
	if (param.unit == PRT_MM)
		mul = 72.0 / 25.4;
	else if (param.unit == PRT_IN)
		mul = 72.0;
	else if (param.unit == PRT_PX)
		mul = 72.0/96.0;
	
	param.w = (int) (mul * param.w + 0.5);
	param.h = (int) (mul * param.h + 0.5);
	param.scale *= mul;
	
	/* write drawing postscript commands to file */
	print_ents_ps(drawing, file, param);
	
	/* end */
	fclose(file);
	
	return 1;
}

int print_img(dxf_drawing *drawing, struct print_param param, char *dest){
	/* print a drawing to a image file */
	/* use the same main engine to draw on the screen */
	/* write to especific data format is provided by stb_image_write library */
	
	if (!drawing) return 0;
	
	/* multiplier to fit image parameters in final output units */
	param.resolution = 1.0;
	if (param.unit == PRT_MM){
		param.resolution = 96.0 / 25.4;
	}
	else if (param.unit == PRT_IN){
		param.resolution = 96.0;
	}
	
	int w = (int) param.w * param.resolution;
	int h = (int) param.h * param.resolution;
	int ret = 0;
	
	/* main colors */
	bmp_color transp = { .r = 255, .g = 255, .b = 255, .a = 0 };
	bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
	bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
	
	/* buffer to write the raw image */
	bmp_img * img = bmp_new(w, h, transp, black);
	
	if (img == NULL) return 0;
	
	/*order of color components in buffer.*/
	img->r_i = 0;
	img->g_i = 1;
	img->b_i = 2;
	img->a_i = 3;
	
	/* init draw parameters */
	struct draw_param d_param;
	d_param.ofs_x = param.ofs_x;
	d_param.ofs_y = param.ofs_y;
	d_param.scale = param.scale * param.resolution;
	d_param.list = param.list;
	d_param.subst = param.subst;
	d_param.len_subst = param.len;
	d_param.inc_thick = 0;
	
	/* clear image with background color */
	if (param.out_fmt == PRT_PNG)
		bmp_fill(img, transp); /* in PNG format, background is transparent */
	else
		bmp_fill(img, white); /* white background */
	
	/* draw image */
	dxf_ents_draw(drawing, img, d_param);
	
	/* save file in specified format */
	if (param.out_fmt == PRT_PNG)
		ret = stbi_write_png((char const *)dest, w, h, 4, img->buf, w * 4);
	else if (param.out_fmt == PRT_BMP)
		ret = stbi_write_bmp((char const *)dest, w, h, 4, img->buf);
	else if (param.out_fmt == PRT_JPG)
		ret = stbi_write_jpg((char const *)dest, w, h, 4, img->buf, 80);
	
	/* end */
	bmp_free(img);
	
	return ret;
}