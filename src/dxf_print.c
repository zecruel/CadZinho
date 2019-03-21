#include "pdfgen.h"
#include "miniz.h"
#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"
#include "font.h"
#include "dxf_graph.h"

#define PDF_BUF_SIZE 50*1024*1024

struct txt_buf{
	long pos;
	char data[PDF_BUF_SIZE];
};
	

void zoom_ext3(dxf_drawing *drawing, int x, int y, int width, int height, double *zoom, double *ofs_x, double *ofs_y){
	double min_x, min_y, max_x, max_y;
	double zoom_x, zoom_y;
	dxf_ents_ext(drawing, &min_x, &min_y, &max_x, &max_y);
	zoom_x = fabs(max_x - min_x)/width;
	zoom_y = fabs(max_y - min_y)/height;
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	*zoom = 1/(*zoom);
	
	*ofs_x = min_x - ((fabs((max_x - min_x)*(*zoom) - width)/2)+x)/(*zoom);
	*ofs_y = min_y - ((fabs((max_y - min_y)*(*zoom) - height)/2)+y)/(*zoom);
}

void print_draw(graph_obj * master, struct txt_buf *buf, double ofs_x, double ofs_y, double scale, double resolution){
	if (master != NULL){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, i;
			line_node *current = master->list->next;
			int tick = 0, prev_x, prev_y; /* for fill */
			int init = 0;
			
			bmp_color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
			bmp_color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
			
			/* set the pattern */
			//patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			bmp_color color = master->color;
			if (color.r == white.r && color.g == white.g && color.b == white.b)
				color = black;
			
			/* set the tickness */
			//if (master->thick_const) img->tick = (int) round(master->tick);
			//else img->tick = (int) round(master->tick * scale);
			
			/* draw the lines */
			
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				x0 = (int) ((current->x0 - ofs_x) * scale * resolution);
				y0 = (int) ((current->y0 - ofs_y) * scale * resolution);
				x1 = (int) ((current->x1 - ofs_x) * scale * resolution);
				y1 = (int) ((current->y1 - ofs_y) * scale * resolution);
				
				if (init == 0){
					/* set the pattern */
					
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"[");
					
					if(master->patt_size > 1){
						int patt_el = (int) fabs(master->pattern[0] * scale * resolution) + 1;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%d", patt_el);
						for (i = 1; i < master->patt_size; i++){
							patt_el = (int) fabs(master->pattern[i] * scale * resolution) + 1;
							buf->pos +=snprintf(buf->data + buf->pos,
								PDF_BUF_SIZE - buf->pos,
								" %d", patt_el);
						}
					}
					
					
					
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"] 0 d ");
					/* set the tickness */
					if (master->thick_const) tick = (int) round(master->tick * 0.14067 * scale * resolution);
					else tick = (int) round(master->tick * scale * resolution);
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d w ", tick); /*line width */
					/* set the color */
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%0.2f %0.2f %0.2f RG ",
						(float)color.r/255,
						(float)color.g/255,
						(float)color.b/255);
					if (master->fill) /* check if object is filled */
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"%0.2f %0.2f %0.2f rg ",
							(float)color.r/255,
							(float)color.g/255,
							(float)color.b/255);
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d m ", x0, y0);
					init = 1;
				}
				
				else if (((x0 != prev_x)||(y0 != prev_y)))
					buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"%d %d m ", x0, y0);
				buf->pos +=snprintf(buf->data + buf->pos,
					PDF_BUF_SIZE - buf->pos,
					"%d %d l ", x1, y1);
			
				prev_x = x1;
				prev_y = y1;
					
				
				current = current->next; /* go to next */
			}
			
			if (master->fill) /* check if object is filled */
				buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"f*\r\n");
			
			else buf->pos +=snprintf(buf->data + buf->pos,
						PDF_BUF_SIZE - buf->pos,
						"S\r\n");
		}
	}
}

int print_list_draw(list_node *list, struct txt_buf *buf, double ofs_x, double ofs_y, double scale, double resolution){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				print_draw(curr_graph, buf, ofs_x, ofs_y, scale, resolution);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int print_ents_draw(dxf_drawing *drawing, struct txt_buf *buf, double ofs_x, double ofs_y, double scale, double resolution){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		// starts the content sweep 
		while (current != NULL){
			if (current->type == DXF_ENT){ // DXF entity
				/*verify if entity layer is on and thaw */
				//lay_idx = dxf_layer_get(drawing, current);
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
				
					// -------------------------------------------
					if (!init){
						double res;
						if (resolution > 0.0) res = 1/resolution;
						else res = 1.0;
						buf->pos +=snprintf(buf->data + buf->pos,
							PDF_BUF_SIZE - buf->pos,
							"1 J " /*line cap style - round*/
							"%0.2f 0.0 0.0 %0.2f 0.0 0.0 cm\r\n", res, res);
						init = 1;
					}
	
					print_list_draw(current->obj.graphics, buf, ofs_x, ofs_y, scale, resolution);
					
					//---------------------------------------
				}
			}
			current = current->next;
		}
	}
}

int print_pdf(dxf_drawing *drawing, double scale, double ofs_x, double ofs_y, double w, double h){
	
	if (!drawing) return 0;
	
	struct txt_buf *buf = malloc(sizeof(struct txt_buf));
	
	if (!buf) return 0;
	double resolution = 10;
	
	buf->pos = 0;
	
	//zoom_ext3(drawing, 0, 0, PDF_A4_HEIGHT, PDF_A4_WIDTH, &scale, &ofs_x, &ofs_y);
	
	
	print_ents_draw(drawing, buf, ofs_x, ofs_y, scale * 2.83465, resolution);
	
	int cmp_status;
	long src_len = strlen(buf->data);
	long cmp_len = compressBound(src_len);
	// Allocate buffers to hold compressed and uncompressed data.
	mz_uint8 *pCmp = (mz_uint8 *)malloc((size_t)cmp_len);
	// Compress the string.
	cmp_status = compress(pCmp, &cmp_len, (const unsigned char *)buf->data, src_len);
	if (cmp_status == Z_OK){
		printf("Compressed from %u to %u bytes\n", (mz_uint32)src_len, (mz_uint32)cmp_len);
	}
	
	struct pdf_info info = {
		.creator = "CadZinho",
		.producer = "CadZinho",
		.title = "Print Drawing",
		.author = "Ze Cruel",
		.subject = "Print Drawing",
		.date = ""
	};
	struct pdf_doc *pdf = pdf_create((int)(w * 2.83465), (int)(h * 2.83465), &info);
	
	struct pdf_object *page = pdf_append_page(pdf);
	
	//pdf_add_stream(pdf, page, buf->data);
	pdf_add_stream_zip(pdf, page, pCmp, cmp_len);
	pdf_save(pdf, "E:\\documentos\\cadzinho\\output.pdf");
	pdf_destroy(pdf);
	
	free(pCmp);
	free(buf);
	#if(0)
	#endif
}