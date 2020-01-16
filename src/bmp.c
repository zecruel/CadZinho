#include "bmp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TOL 1e-12

// Quick sort function
void quick_sort(int *a, int left, int right) {
    int i, j, x, y;
     
    i = left;
    j = right;
    x = a[(left + right) / 2];
     
    while(i <= j) {
        while(a[i] < x && i < right) {
            i++;
        }
        while(a[j] > x && j > left) {
            j--;
        }
        if(i <= j) {
            y = a[i];
            a[i] = a[j];
            a[j] = y;
            i++;
            j--;
        }
    }
     
    if(j > left) {
        quick_sort(a, left, j);
    }
    if(i < right) {
        quick_sort(a, i, right);
    }
}


rect_pos rect_find_pos(double x, double y, double xmin, double ymin, double xmax, double ymax){
/* Compute the bit code for a point (x, y) using the clip rectangle
 bounded diagonally by (xmin, ymin), and (xmax, ymax) */
	rect_pos code;
	code = INSIDE;          /* initialised as being inside of [[clip window]]*/

	if (x < xmin)           /* to the left of clip window*/
		code |= LEFT;
	else if (x > xmax)      /* to the right of clip window*/
		code |= RIGHT;
	if (y < ymin)           /* below the clip window*/
		code |= BOTTOM;
	else if (y > ymax)      /* above the clip window*/
		code |= TOP;
	
	return code;
}

int line_clip(bmp_img *img, double *x0, double *y0, double *x1, double *y1) {

	/* 
	clip the line on the window area
	
	from: https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
	Cohen–Sutherland clipping algorithm clips a line from
	P0 = (*x0, *y0) to P1 = (*x1, *y1) against a rectangle of bitmap image */
	
	/* compute outcodes for P0, P1, and whatever point lies outside the clip rectangle */
	rect_pos pos_p0 = rect_find_pos(*x0, *y0, img->clip_x, img->clip_y, img->clip_x + img->clip_w, img->clip_y + img->clip_h);
	rect_pos pos_p1 = rect_find_pos(*x1, *y1, img->clip_x, img->clip_y, img->clip_x + img->clip_w, img->clip_y + img->clip_h);
	
	int accept = 0;

	while (1) {
		if (!(pos_p0 | pos_p1)) { /* Bitwise OR is 0. Trivially accept and get out of loop */
			accept = 1;
			break;
		}
		else if (pos_p0 & pos_p1) { /* Bitwise AND is not 0. 
			(implies both end points are in the same region outside the 
			window). Reject and get out of loop */
			break;
		}
		else {
			/* failed both tests, so calculate the line segment to clip
			from an outside point to an intersection with clip edge*/
			double x, y;
			double den;

			/* At least one endpoint is outside the clip rectangle; pick it. */
			rect_pos pos_out = pos_p0 ? pos_p0 : pos_p1;

			/* Now find the intersection point;
			use formulas y = *y0 + slope * (x - *x0), x = *x0 + (1 / slope) * (y - *y0) */
			if (pos_out & TOP) {           /* point is above the clip rectangle */
				den = (*y1 - *y0);
				x = *x0;
				if (fabs(den) > TOL)
				x = *x0 + (*x1 - *x0) * (img->height - *y0) / den;
				y = img->height;
			} else if (pos_out & BOTTOM) { /* point is below the clip rectangle*/
				den = (*y1 - *y0);
				x = *x0;
				if (fabs(den) > TOL)
				x = *x0 + (*x1 - *x0) * (0 - *y0) / den;
				y = 0;
			} else if (pos_out & RIGHT) {  /* point is to the right of clip rectangle */
				den = (*x1 - *x0);
				y = *y0;
				if (fabs(den) > TOL)
				y = *y0 + (*y1 - *y0) * (img->width - *x0) / den;
				x = img->width;
			} else if (pos_out & LEFT) {   /* point is to the left of clip rectangle */
				den = (*x1 - *x0);
				y = *y0;
				if (fabs(den) > TOL)
				y = *y0 + (*y1 - *y0) * (0 - *x0) / den;
				x = 0;
			}

			/* Now we move outside point to intersection point to clip
			and get ready for next pass.*/
			if (pos_out == pos_p0) {
				*x0 = x;
				*y0 = y;
				pos_p0 = rect_find_pos(*x0, *y0, 0, 0, img->width, img->height);
			} else {
				*x1 = x;
				*y1 = y;
				pos_p1 = rect_find_pos(*x1, *y1, 0, 0, img->width, img->height);
			}
		}
	}
	return accept;
}

void bmp_fit(bmp_img *img, double min_x, double min_y, double max_x, double max_y, double *zoom, double *ofs_x, double *ofs_y){
	double zoom_x, zoom_y;
	
	zoom_x = fabs(max_x - min_x)/img->width;
	zoom_y = fabs(max_y - min_y)/img->height;
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	*zoom = 1/(1.1 * (*zoom));
	
	*ofs_x = min_x - (fabs((max_x - min_x)*(*zoom) - img->width)/2);
	*ofs_y = min_y - (fabs((max_y - min_y)*(*zoom) - img->height)/2);
}

int bmp_fill (bmp_img *img, bmp_color color){
	/* fill the bmp image with passed color */
	
	unsigned int i, n, r_i, g_i, b_i, a_i;
	
	if (img){
		/* get the order of color components */
		r_i = img->r_i;
		g_i = img->g_i;
		b_i = img->b_i;
		a_i = img->a_i;
		
		n = 4 * img->width * img->height; /* four components: RGBA */
		for (i=0; i < n; i += 4) /* increment by 4 in each step */
		{ /* store each component in memory buffer */
			img->buf[i+r_i] = color.r;
			img->buf[i+g_i] = color.g;
			img->buf[i+b_i] = color.b;
			img->buf[i+a_i] = color.a;
		}
		return 1; /*return success*/
	}
	return 0; /* return fail */
}

int bmp_fill_clip (bmp_img *img, bmp_color color){
	/* fill the bmp image with passed color */
	
	unsigned int i, j, r_i, g_i, b_i, a_i, ofs_x, ofs_y, ofs_src;
	
	if (img){
		/* get the order of color components */
		r_i = img->r_i;
		g_i = img->g_i;
		b_i = img->b_i;
		a_i = img->a_i;
		
		
		/* sweep the new image */
		for (i=0; i < img->clip_w; i++){
			for (j=0; j < img->clip_h; j++){
				ofs_x = i +  img->clip_x;
				if (img->zero_tl != 0){
					ofs_y = j +  img->clip_y;
				}
				else{
					ofs_y = img->height - 1 - (j + img->clip_y);
				}
				//ofs_y = j ;
				
				if((ofs_x >= 0) && (ofs_x < img->width) && 
				(ofs_y >= 0) && (ofs_y < img->height)){
					/* find the position on destination buffer */
					ofs_src = 4 * ((ofs_y * img->width) + ofs_x);
					/* store each component in memory buffer */
					img->buf[ofs_src + r_i] = color.r;
					img->buf[ofs_src + g_i] = color.g;
					img->buf[ofs_src + b_i] = color.b;
					img->buf[ofs_src + a_i] = color.a;
				}
			}
		}
		
		return 1; /*return success*/
	}
	return 0; /* return fail */
}

bmp_img * bmp_new (unsigned int width, unsigned int height, bmp_color bkg, bmp_color frg){
	/* create new bmp image */
	
	/* alloc the structure */
	bmp_img * img = (bmp_img *) malloc (sizeof (bmp_img));
	if (img){
		/* initialize */
		/* dimensions */
		img->width = width;
		img->height = height;
		/* colors */
		img->bkg = bkg;
		img->frg = frg;
		/* line pattern generation vars  */
		img->tick = 0;
		img->patt_i = 0;
		img->pix_count = 0;
		/* line pattern */
		/* initialize the image with a solid line pattern */
		img->patt_size = 1;
		img->pattern[0] = 1;
		img->pat_scale = 15;
		
		img->zero_tl = 0; /* zero in botton left corner */
		
		/*clipping rectangle */
		img->clip_x = 0;
		img->clip_y = 0;
		img->clip_w = width;
		img->clip_h = height;
		
		img->prev_x = 0;
		img->prev_y = 0;
		
		img->end_x[0] = 0; img->end_x[1] = 0; img->end_x[2] = 0; img->end_x[3] = 0;
		img->end_y[0] = 0; img->end_y[1] = 0; img->end_y[2] = 0; img->end_y[3] = 0;
		
		/*order of color components in buffer. Init with ARGB */
		img->r_i = 2;
		img->g_i = 1;
		img->b_i = 0;
		img->a_i = 3;
		
		/* alloc the pix map buffer
		the image will have 4 chanels: R, G, B and alfa */
		img->buf = malloc(4 * width * height * sizeof(unsigned char));
		if (img->buf == NULL){ /* fail in memory allocation */
			free (img);
			img = NULL;
		}
		else{
			bmp_fill (img, bkg); /* fill the image with the background color */
		}
	}
	return img;
}

void bmp_free(bmp_img *img){
	if(img){
		free(img->buf);
		free(img);
	}
}

int bmp_save (char *path, bmp_img *img){
	/* save the pixmap in a PPM file */
	
	FILE *file;
	int ret_success;
	unsigned int i, n, r_i, g_i, b_i, a_i;
	
	ret_success = 0;
	file = fopen(path, "w"); /* open the file */
	if ((file != NULL) && (img != NULL)){
		/* get the order of color components */
		r_i = img->r_i;
		g_i = img->g_i;
		b_i = img->b_i;
		a_i = img->a_i;
		
		/* the header of PPM: binary file with 8 bit deph */
		fprintf(file, "P6 %d %d 255\r",  img->width, img->height);
		
		/* sweep the buffer of image*/
		n = 4 * img->width * img->height;
		for (i=0; i < n; i += 4){
			/* only the RGB components are writen */
			fputc(img->buf[i+r_i], file);
			fputc(img->buf[i+g_i], file);
			fputc(img->buf[i+b_i], file);
		}
		ret_success = 1; /* return success */
	}
	
	fclose(file);
	
	return ret_success;
}

void bmp_point_raw (bmp_img *img, int x, int y){
	/* draw one pixel on x,y coordinates in bmp image */
	
	unsigned int ofs, r_i, g_i, b_i, a_i;
	unsigned char *buf_r, *buf_g, *buf_b, *buf_a;
	bmp_color frg;
	
	if(img != NULL){
		/* get the order of color components */
		r_i = img->r_i;
		g_i = img->g_i;
		b_i = img->b_i;
		a_i = img->a_i;
		
		/* check if point is in image bounds */
		if((x >= 0) && (x < img->width) && 
		(y >= 0) && (y < img->height) &&
		/*and in clip rectangle*/
		(x >= img->clip_x) && (x < (img->clip_x + img->clip_w)) && 
		(y >= img->clip_y) && (y < (img->clip_y + img->clip_h))){
			/* find the initial position on image buffer */
			/* (y = img->height - y) emulate the cartesian coordinates */
			if (img->zero_tl != 0){
				ofs = 4 * ((y* img->width) + x);
			}
			else{
				ofs = 4 * (((img->height - 1 - y) * img->width) + x);
			}
			
			/* get the buffer components */
			
			buf_r = img->buf + ofs + r_i;
			buf_g = img->buf + ofs + g_i;
			buf_b = img->buf + ofs + b_i;
			buf_a = img->buf + ofs + a_i;
			frg = img->frg;
			
			if (frg.a < 255){
				/* Alpha compositing */
				struct {double r,g,b,a;} out;
				double dst_a, src_a;
				
				/*normalize alfa channels */
				src_a = (double) frg.a / 255;
				dst_a = ((double)(*buf_a) / 255)*(1 - src_a);
				//dst_a = ((double)img->buf[ofs+a_i] / 255)*(1 - src_a);
				
				out.a = src_a + dst_a;
				if (out.a == 0){
					out.r = 0;
					out.g = 0;
					out.b = 0;
				}
				else{
					out.r = (frg.r*src_a + *buf_r*dst_a)/out.a;
					out.g = (frg.g*src_a + *buf_g*dst_a)/out.a;
					out.b = (frg.b*src_a + *buf_b*dst_a)/out.a;
					out.a *= 255;
				}
				*buf_r = (unsigned char) (out.r);
				*buf_g = (unsigned char) (out.g );
				*buf_b = (unsigned char) (out.b );
				*buf_a = (unsigned char) (out.a );
			}
			else{
				/* store each component in memory buffer 
				with the image´s foreground color*/
				*buf_r = frg.r;
				*buf_g = frg.g;
				*buf_b = frg.b;
				*buf_a = 255;
			}
		}
	}
}

int patt_change(bmp_img *img, double patt[], int size){
	/* change the current pattern in image to draw lines */
	if(img != NULL){
		int i;
		/*set size */
		img->patt_size = size;
		/*TODO: verifiy the maximum size */
		
		/* restart the pattern generation*/
		img->patt_i = 0;
		img->pix_count = 0;
		
		/*set the pattern data*/
		for (i = 0; i < size; i++){
			img->pattern[i] = (int) (img->pat_scale * patt[i]);
		}
		
		return 1; /* return success */
	}
	return 0; /* fail */
}

int patt_check(bmp_img *img){
	/* auxiliary function to line pattern generation
	check if current position on line will be draw ("pen down") or will be blank ("pen up") */
	
	int pen = 0; /* pen up by default*/
	
	if(img != NULL){
		img->pix_count += 1; /* increment the position */
		/* check if current pattern data is positive */
		if (img->pattern[img->patt_i] >=0){ 
			/* its means the draw command */
			pen = 1; /* pen down */
		}
		/*check the end of current pattern data */
		if (img->pix_count >= abs(img->pattern[img->patt_i])){
			img->pix_count = 0; /* restart the position */
			img->patt_i += 1; /* go to the next pattern data */
			/*check the end of pattern */
			if (img->patt_i >= img->patt_size){
				img->patt_i = 0; /* restart the pattern */
			}
		}
	}
	return pen;
}

void bmp_circle(bmp_img *img, int x0, int y0, int radius){
	int x = radius;
	int y = 0;
	int err = 0;
	int x_start, x_end, i, signal;

	while (x >= y){
		bmp_point_raw (img, x0 + x, y0 + y);
		bmp_point_raw (img, x0 - x, y0 + y);
		
		bmp_point_raw (img, x0 + y, y0 + x);
		bmp_point_raw (img, x0 - y, y0 + x);
		
		
		bmp_point_raw (img, x0 + x, y0 - y);
		bmp_point_raw (img, x0 - x, y0 - y);
		
		
		bmp_point_raw (img, x0 - y, y0 - x);
		bmp_point_raw (img, x0 + y, y0 - x);
		
		

		if (err <= 0){
			y += 1;
			err += 2*y + 1;
		}
		if (err > 0){
			x -= 1;
			err -= 2*x + 1;
		}
	}
}

void bmp_circle_fill(bmp_img *img, int x0, int y0, int radius){
	int x = radius;
	int y = 0;
	int err = 0;
	int x_start, x_end, i, signal;

	while (x >= y){
		//putpixel(x0 + x, y0 + y);
		//putpixel(x0 - x, y0 + y);
		x_start = x0 + x;
		x_end = x0 - x;
		signal = x_start < x_end ? 1 : -1;
		for(i = x_start; i != x_end; i += signal){
			bmp_point_raw (img, i, y0 + y);
		}
		
		//putpixel(x0 + y, y0 + x);
		//putpixel(x0 - y, y0 + x);
		x_start = x0 + y;
		x_end = x0 - y;
		signal = x_start < x_end ? 1 : -1;
		for(i = x_start; i != x_end; i += signal){
			bmp_point_raw (img, i, y0 + x);
		}
		
		//putpixel(x0 + x, y0 - y);
		//putpixel(x0 - x, y0 - y);
		x_start = x0 + x;
		x_end = x0 - x;
		signal = x_start < x_end ? 1 : -1;
		for(i = x_start; i != x_end; i += signal){
			bmp_point_raw (img, i, y0 - y);
		}
		
		//putpixel(x0 - y, y0 - x);
		//putpixel(x0 + y, y0 - x);
		x_start = x0 + y;
		x_end = x0 - y;
		signal = x_start < x_end ? 1 : -1;
		for(i = x_start; i != x_end; i += signal){
			bmp_point_raw (img, i, y0 - x);
		}
		

		if (err <= 0){
			y += 1;
			err += 2*y + 1;
		}
		if (err > 0){
			x -= 1;
			err -= 2*x + 1;
		}
	}
}

void bmp_poly_fill(bmp_img *img, int verts, int vert_x[], int vert_y[], int stroke[]){
	int i, w = img->width, h = img->height;
	int x0, y0, x1, y1;
	int min_x, max_x, min_y, max_y;
	int nodes = 0, node_x[1000], swap;
	int pix_x, pix_y, strk = 0;
	
	
	/* find min and max in coordinates*/
	min_x = vert_x[0];
	max_x = vert_x[0];
	min_y = vert_y[0];
	max_y = vert_y[0];
	for (i = 1; i < verts; i++){
		x0 = vert_x[i];
		y0 = vert_y[i];
		/* update min and max vars*/
		min_x = (min_x < x0) ? min_x : x0;
		max_x = (max_x > x0) ? max_x : x0;
		min_y = (min_y < y0) ? min_y : y0;
		max_y = (max_y > y0) ? max_y : y0;
	}
	
	if((min_x < w) && (max_x > 0) && (min_y < h) && (max_y > 0 )){
		
		min_x = (min_x > 0) ? min_x : 0;
		max_x = (max_x < w) ? max_x : w;
		min_y = (min_y > 0) ? min_y : 0;
		max_y = (max_y < h) ? max_y : h;
		
		
		for (pix_y = min_y; pix_y < max_y; pix_y++){ /* sweep line in y coordinate*/
			nodes = 0;
			x0 = vert_x[0];
			y0 = vert_y[0];
			for (i = 1; i < verts; i++){
				/* create a list of nodes (intersections of sweep line on polygon) */
				x1 = vert_x[i];
				y1 = vert_y[i];
				
				strk = (stroke)? stroke[i] : 1; 
				/* verify intersection */
				if ((nodes < 1000) && (strk != 0)){
					if(((y0 < pix_y) && (y1 >= pix_y)) || 
					((y1 < pix_y) && (y0 >= pix_y))){
						/* find x coord of intersection and add to list */
						node_x[nodes] = (int) round(x1 + (double) (pix_y - y1)/(y0 - y1)*(x0 - x1));
						node_x[nodes] = (node_x[nodes] >= 0) ? node_x[nodes] : -1;
						node_x[nodes] = (node_x[nodes] <= max_x) ? node_x[nodes] : max_x + 1;
						nodes++;
					}
				}
				x0 = vert_x[i];
				y0 = vert_y[i];
			}
			/* last vertice -> close polygon by default or if stroke[0] is 1 */
			x1 = vert_x[0];
			y1 = vert_y[0];
			strk = (stroke)? stroke[0] : 1; 
			if ((nodes < 1000) && (strk != 0)){
				if(((y0 < pix_y) && (y1 >= pix_y)) || 
				((y1 < pix_y) && (y0 >= pix_y))){
					/* find x coord of intersection and add to list */
					node_x[nodes] = (int) round(x1 + (double) (pix_y - y1)/(y0 - y1)*(x0 - x1));
					node_x[nodes] = (node_x[nodes] >= 0) ? node_x[nodes] : -1;
					node_x[nodes] = (node_x[nodes] <= max_x) ? node_x[nodes] : max_x + 1;
					nodes++;
				}
			}
			
			/* sort the nodes, via quick sort*/
			quick_sort(node_x, 0, nodes - 1);
				
			/*fill the pixels between node pairs*/
			for (i = 0; i < nodes ; i += 2){
				if (i+1 < nodes){
					for(pix_x = node_x[i]; pix_x < node_x[i + 1]; pix_x++){
						bmp_point_raw(img, pix_x, pix_y);
					}
				}
			}
		}
	}
}

void bmp_rect_fill(bmp_img *img, int vert_x[4], int vert_y[4]){
	int i, w = img->width, h = img->height;
	int x0, y0, x1, y1;
	int min_x, max_x, min_y, max_y;
	int nodes = 0, node_x[4], swap;
	int pix_x, pix_y;
	
	
	/* find min and max in coordinates*/
	min_x = vert_x[0];
	max_x = vert_x[0];
	min_y = vert_y[0];
	max_y = vert_y[0];
	for (i = 1; i < 4; i++){
		x0 = vert_x[i];
		y0 = vert_y[i];
		/* update min and max vars*/
		min_x = (min_x < x0) ? min_x : x0;
		max_x = (max_x > x0) ? max_x : x0;
		min_y = (min_y < y0) ? min_y : y0;
		max_y = (max_y > y0) ? max_y : y0;
	}
	
	if((min_x < w) && (max_x > 0) && (min_y < h) && (max_y > 0 )){
		
		min_x = (min_x > 0) ? min_x : 0;
		max_x = (max_x < w) ? max_x : w;
		min_y = (min_y > 0) ? min_y : 0;
		max_y = (max_y < h) ? max_y : h;
		
		
		for (pix_y = min_y; pix_y < max_y; pix_y++){ /* sweep line in y coordinate*/
			nodes = 0;
			x0 = vert_x[3];
			y0 = vert_y[3];
			for (i = 0; i < 4; i++){
				/* create a list of nodes (intersections of sweep line on polygon) */
				x1 = vert_x[i];
				y1 = vert_y[i];
				
				/* verify intersection */
				if(((y0 < pix_y) && (y1 >= pix_y)) || 
				((y1 < pix_y) && (y0 >= pix_y))){
					/* find x coord of intersection and add to list */
					node_x[nodes] = (int) round(x1 + (double) (pix_y - y1)/(y0 - y1)*(x0 - x1));
					node_x[nodes] = (node_x[nodes] >= 0) ? node_x[nodes] : -1;
					node_x[nodes] = (node_x[nodes] <= max_x) ? node_x[nodes] : max_x + 1;
					nodes++;
				}
				
				x0 = vert_x[i];
				y0 = vert_y[i];
			}
			
			/* sort the nodes*/
			swap = node_x[0];
			if (swap > node_x[1]){
				node_x[0] = node_x[1];
				node_x[1] = swap;
			}
				
			/*fill the pixels between node pairs*/
			for (i = 0; i < nodes ; i += 2){
				if (i+1 < nodes){
					for(pix_x = node_x[i]; pix_x < node_x[i + 1]; pix_x++){
						bmp_point_raw(img, pix_x, pix_y);
					}
				}
			}
		}
	}
}


void bmp_thick_line(bmp_img *img, int p1x, int p1y, int p2x, int p2y) {
/* Draw a line on bmp image
Bitmap/Bresenham's line algorithm
from: http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C */
	
	int x0 = p1x, y0 = p1y, x1 = p2x, y1 = p2y;
	int half_x, half_y; /* to determine line's center*/
	double x, y, modulus;
	int normal_x, normal_y, start_x, start_y, end_x, end_y;
	
	/* get the vector of line */
	x = x1 - x0;
	y = y1 - y0;
	modulus = sqrt(pow(x, 2) + pow(y, 2));
	if (modulus > 0){ /* change to unit */
		x /= modulus;
		y /= modulus;
	}
	/* normal vector is xn = -y, yn = x */
	normal_x = (int)(round((double)img->tick * -y));
	normal_y = (int)(round((double)img->tick * x));
	
	half_x = normal_x/2;
	half_y = normal_y/2;
	
	/*update line conection parameters */
	img->end_x[2] = x0 - half_x;
	img->end_y[2] = y0 - half_y;
	img->end_x[3]  = img->end_x[2] + normal_x;
	img->end_y[3] = img->end_y[2] + normal_y;
	if ((img->tick > 1) && (img->prev_x == x0) && (img->prev_y == y0)){ /* if the point have a tickness */
		/* draw a filled  polygon as line conection*/
		bmp_poly_fill(img, 4, img->end_x, img->end_y, NULL);
	}
	
	/*update line conection parameters for next line*/
	img->prev_x = x1;
	img->prev_y = y1;
	img->end_x[1] = x1 - half_x;
	img->end_y[1] = y1 - half_y;
	img->end_x[0]  = img->end_x[1] + normal_x;
	img->end_y[0] = img->end_y[1] + normal_y;
	
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
	for(;;){
		e2 = err;
		if (patt_check(img)){ /* image's line pattern generation*/
			if (img->tick > 1){ /* if the point have a tickness */
				/* draw a normal line (with length of thickness)
				in each point of main line */
				start_x = x0 - half_x;
				start_y = y0 - half_y;
				end_x = start_x + normal_x;
				end_y = start_y + normal_y;
				bmp_thin_line(img, start_x, start_y, end_x, end_y);
				
				if ((e2 >-dx) && (e2 < dy)) { /* diagonal movement */
					/* in this case, a new line is draw to fill blank spaces */
					if (dx>dy){
						start_x += sx;
						end_x += sx;
					}
					else{
						start_y += sy;
						end_y += sy;
					}
					bmp_thin_line(img, start_x, start_y, end_x, end_y);
				}
			}
			else{ /* a skinny point */
				bmp_point_raw (img, x0, y0);
			}
		}
		if (x0==x1 && y0==y1) break;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void bmp_thick_line_norm(bmp_img *img, int p1x, int p1y, int p2x, int p2y, double norm_x, double norm_y) {
/* Draw a line on bmp image
	Uses normal vector as auxiliary calculation.
	--NOT CHECK PATTERN--
Bitmap/Bresenham's line algorithm
from: http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C */
	
	int x0 = p1x, y0 = p1y, x1 = p2x, y1 = p2y;
	int normal_x, normal_y, half_x, half_y; /* to determine line's center*/
	double x, y, modulus;
	int start_x, start_y, end_x, end_y;
	
	if (img->tick > 1) {
		normal_x = (int)(round((double)img->tick * norm_x));
		normal_y = (int)(round((double)img->tick * norm_y));
		
		half_x = normal_x/2;
		half_y = normal_y/2;
		
		/*update line conection parameters */
		img->end_x[2] = x0 - half_x;
		img->end_y[2] = y0 - half_y;
		img->end_x[3]  = img->end_x[2] + normal_x;
		img->end_y[3] = img->end_y[2] + normal_y;
		
		if ((img->prev_x == x0) && (img->prev_y == y0)){ /* if the point have a tickness */
			/* draw a filled  polygon as line conection*/
			bmp_rect_fill(img, img->end_x, img->end_y);
		}
		
		/*update line conection parameters for next line*/
		img->prev_x = x1;
		img->prev_y = y1;
		img->end_x[1] = x1 - half_x;
		img->end_y[1] = y1 - half_y;
		img->end_x[0]  = img->end_x[1] + normal_x;
		img->end_y[0] = img->end_y[1] + normal_y;
	}
	
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
	for(;;){
		e2 = err;
		//if (patt_check(img)){ /* image's line pattern generation*/
			if (img->tick > 1){ /* if the point have a tickness */
				/* draw a normal line (with length of thickness)
				in each point of main line */
				start_x = x0 - half_x;
				start_y = y0 - half_y;
				end_x = start_x + normal_x;
				end_y = start_y + normal_y;
				bmp_thin_line(img, start_x, start_y, end_x, end_y);
				
				if ((e2 >-dx) && (e2 < dy)) { /* diagonal movement */
					/* in this case, a new line is draw to fill blank spaces */
					if (dx>dy){
						start_x += sx;
						end_x += sx;
					}
					else{
						start_y += sy;
						end_y += sy;
					}
					bmp_thin_line(img, start_x, start_y, end_x, end_y);
				}
			}
			else{ /* a skinny point */
				bmp_point_raw (img, x0, y0);
			}
		//}
		if (x0==x1 && y0==y1) break;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void bmp_thin_line(bmp_img *img, int x0, int y0, int x1, int y1) {
/* Draw a line on bmp image
Bitmap/Bresenham's line algorithm
from: http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C */
	
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		if (x0==x1 && y0==y1) break;
		bmp_point_raw(img, x0, y0);
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void bmp_line(bmp_img *img, double x0, double y0, double x1, double y1) {

	/* 
	Draw a line on bmp image - clip the line on the window area
	*/
	
	if (line_clip(img, &x0, &y0, &x1, &y1)) {
		int x_0 = (int) x0, y_0 = (int) y0;
		int x_1 = (int) x1, y_1 = (int) y1;
		if ((x_0 == x_1) && (y_0 == y_1)){ /* a dot*/
			int thick = 0;
			if (img != NULL) thick = img->tick/2;
			if (thick <= 0) thick = 1; /* minimal thick */
			bmp_circle_fill(img, x_0, y_0, thick);
		}
		else bmp_thick_line(img, x_0, y_0, x_1, y_1);
	}
}

void bmp_line_norm(bmp_img *img, double x0, double y0, double x1, double y1, double normal_x, double normal_y) {

	/* 
	Draw a line on bmp image. Uses normal vector as auxiliary calculation.
	*/
	
	if (line_clip(img, &x0, &y0, &x1, &y1)) { /*clip the line on the window area*/
		int x_0 = (int) round(x0);
		int y_0 = (int) round(y0);
		int x_1 = (int) round(x1);
		int y_1 = (int) round(y1);
		
		bmp_thick_line_norm(img, x_0, y_0, x_1, y_1, normal_x, normal_y);
	}
}

void bmp_copy(bmp_img *src, bmp_img *dst, int x, int y){
	
	unsigned int i, j, ofs_src, ofs_dst, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	int ofs_x, ofs_y;
	double alpha_d, alpha_s;
	struct {double r,g,b,a;} out;
	
	unsigned char *buf_r, *buf_g, *buf_b, *buf_a;
	
	if((src != NULL) && (dst != NULL)){
		/* get the order of color components */
		src_r= src->r_i;
		src_g = src->g_i;
		src_b = src->b_i;
		src_a = src->a_i;
		dst_r= dst->r_i;
		dst_g = dst->g_i;
		dst_b = dst->b_i;
		dst_a = dst->a_i;
		
		/* sweep the source image */
		for (i=0; i < src->width; i++){
			for (j=0; j < src->height; j++){
				/* check if point is in destination bounds */
				ofs_x = i + x;
				ofs_y = j + y;
				if((ofs_x >= 0) && (ofs_x < dst->width) && 
					(ofs_y >= 0) && (ofs_y < dst->height) &&
					/*and in clip rectangle*/
					(ofs_x >= dst->clip_x) && (ofs_x < (dst->clip_x + dst->clip_w)) && 
					(ofs_y >= dst->clip_y) && (ofs_y < (dst->clip_y + dst->clip_h))){
					/* find the position on destination buffer */
					/* (y = dst->height - y) emulate the cartesian coordinates */
						
					if (dst->zero_tl != 0){
						ofs_dst = 4 * ((ofs_y * dst->width) + ofs_x);
					}
					else{
						ofs_dst = 4 * (((dst->height - 1 - ofs_y) * dst->width) + ofs_x);
					}
					if (src->zero_tl != 0){
						ofs_src = 4 * ((j * src->width) + i);
					}
					else{
						ofs_src = 4 * (((src->height - 1 - j) * src->width) + i);
					}
					
					buf_r = dst->buf + ofs_dst + dst_r;
					buf_g = dst->buf + ofs_dst + dst_g;
					buf_b = dst->buf + ofs_dst + dst_b;
					buf_a = dst->buf + ofs_dst + dst_a;
					
					if (src->buf[ofs_src + src_a] < 255){
						/* Alpha compositing */
						/*normalize alfa channels */
						alpha_s = (double) src->buf[ofs_src + src_a] / 255;
						alpha_d = ((double)*buf_a / 255)*(1 - alpha_s);
						
						out.a = alpha_s + alpha_d;
						if (out.a == 0){
							out.r = 0;
							out.g = 0;
							out.b = 0;
						}
						else{
							out.r = (src->buf[ofs_src + src_r]*alpha_s + *buf_r*alpha_d)/out.a;
							out.g = (src->buf[ofs_src + src_g]*alpha_s + *buf_g*alpha_d)/out.a;
							out.b = (src->buf[ofs_src + src_b]*alpha_s + *buf_b*alpha_d)/out.a;
							out.a *= 255;
						}
						*buf_r = (unsigned char) (out.r);
						*buf_g = (unsigned char) (out.g);
						*buf_b = (unsigned char) (out.b);
						*buf_a = (unsigned char) (out.a);
					}
					else{
						/* store each component in memory buffer */
						*buf_r = src->buf[ofs_src + src_r];
						*buf_g = src->buf[ofs_src + src_g];
						*buf_b = src->buf[ofs_src + src_b];
						*buf_a = src->buf[ofs_src + src_a];
					}
				}
			}
		}
	}
}

bmp_img * bmp_sub_img(bmp_img *orig, int x, int y, int w, int h){
	
	if (orig){
		bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
		bmp_img *ret = bmp_new (w, h, transp, transp);
		if (ret){
			/* get the order of color components */
			int r_r= ret->r_i;
			int r_g = ret->g_i;
			int r_b = ret->b_i;
			int r_a = ret->a_i;
			int o_r= orig->r_i;
			int o_g = orig->g_i;
			int o_b = orig->b_i;
			int o_a = orig->a_i;
			
			int i, j, ofs_x, ofs_y, ofs_dst, ofs_src;
			
			/* sweep the new image */
			for (i=0; i < w; i++){
				for (j=0; j < h; j++){
					ofs_x = i + x;
					ofs_y = j + y;
					if((ofs_x >= 0) && (ofs_x < orig->width) && 
					(ofs_y >= 0) && (ofs_y < orig->height)){
						/* find the position on destination buffer */
						ofs_src = 4 * ((ofs_y * orig->width) + ofs_x);
						ofs_dst = 4 * ((j * w) + i);
						/* store each component in memory buffer */
						ret->buf[ofs_dst + r_r] = orig->buf[ofs_src + o_r];
						ret->buf[ofs_dst + r_g] = orig->buf[ofs_src + o_g];
						ret->buf[ofs_dst + r_b] = orig->buf[ofs_src + o_b];
						ret->buf[ofs_dst + r_a] = orig->buf[ofs_src + o_a];
					}
				}
			}
		}
		
		return ret;
	}
	return NULL;
}

bmp_img * bmp_load_img(char *url){
	int num_ch, w, h;
	/* use the stb_image to load file in buffer */
	unsigned char *data = stbi_load(url, &w, &h, &num_ch, 4);
	if (data){
		bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
		/* alloc the structure */
		bmp_img * img = (bmp_img *) malloc (sizeof (bmp_img));
		if (img){
			/* initialize */
			img->buf = data;
			/* dimensions */
			img->width = w;
			img->height = h;
			/* colors */
			img->bkg = transp;
			img->frg = transp;
			/* line pattern generation vars  */
			img->tick = 0;
			img->patt_i = 0;
			img->pix_count = 0;
			/* line pattern */
			/* initialize the image with a solid line pattern */
			img->patt_size = 1;
			img->pattern[0] = 1;
			img->pat_scale = 15;
			
			img->zero_tl = 0; /* zero in botton left corner */
			
			/*clipping rectangle */
			img->clip_x = 0;
			img->clip_y = 0;
			img->clip_w = w;
			img->clip_h = h;
			
			img->prev_x = 0;
			img->prev_y = 0;
			
			img->end_x[0] = 0; img->end_x[1] = 0; img->end_x[2] = 0; img->end_x[3] = 0;
			img->end_y[0] = 0; img->end_y[1] = 0; img->end_y[2] = 0; img->end_y[3] = 0;
	
			/*order of color components in buffer. Init with RGBA */
			img->r_i = 0;
			img->g_i = 1;
			img->b_i = 2;
			img->a_i = 3;
		}
		else{
			free(data);
		}
		return img;
	}
	return NULL;
}


bmp_img * bmp_load_img2(unsigned char *data, int w, int h){
	
	if (data){
		bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
		/* alloc the structure */
		bmp_img * img = (bmp_img *) malloc (sizeof (bmp_img));
		if (img){
			/* initialize */
			img->buf = data;
			/* dimensions */
			img->width = w;
			img->height = h;
			/* colors */
			img->bkg = transp;
			img->frg = transp;
			/* line pattern generation vars  */
			img->tick = 0;
			img->patt_i = 0;
			img->pix_count = 0;
			/* line pattern */
			/* initialize the image with a solid line pattern */
			img->patt_size = 1;
			img->pattern[0] = 1;
			img->pat_scale = 15;
			
			img->zero_tl = 0; /* zero in botton left corner */
			
			/*clipping rectangle */
			img->clip_x = 0;
			img->clip_y = 0;
			img->clip_w = w;
			img->clip_h = h;
			
			img->prev_x = 0;
			img->prev_y = 0;
			
			img->end_x[0] = 0; img->end_x[1] = 0; img->end_x[2] = 0; img->end_x[3] = 0;
			img->end_y[0] = 0; img->end_y[1] = 0; img->end_y[2] = 0; img->end_y[3] = 0;
	
			/*order of color components in buffer. Init with RGBA */
			img->r_i = 0;
			img->g_i = 1;
			img->b_i = 2;
			img->a_i = 3;
		}
		return img;
	}
	return NULL;
}

void bmp_put(bmp_img *src, bmp_img *dst, int x, int y, double u[3], double v[3]){
	
	unsigned int i, j, ofs_src, ofs_dst, src_r, src_g, src_b, src_a, dst_r, dst_g, dst_b, dst_a;
	int ofs_x, ofs_y;
	double alpha_d, alpha_s;
	struct {double r,g,b,a;} out;
	
	unsigned char *buf_r, *buf_g, *buf_b, *buf_a;
	
	int w, h;
	double dx, dy, sx, sy;
	
	w = (int) ceil(u[0] * src->width);
	h = (int) ceil(v[1] * src->height);
	
	if((src != NULL) && (dst != NULL)){
		/* get the order of color components */
		src_r= src->r_i;
		src_g = src->g_i;
		src_b = src->b_i;
		src_a = src->a_i;
		dst_r= dst->r_i;
		dst_g = dst->g_i;
		dst_b = dst->b_i;
		dst_a = dst->a_i;
		
		dx = 1.0/u[0];
		dy = 1.0/v[1];
		
		/* sweep the source image */
		sx = 0.0;
		for (i=0; i < w; i++){
			sy = 0.0;
			for (j=0; j < h; j++){
				/* check if point is in destination bounds */
				ofs_x = i + x;
				ofs_y = j + y;
				if((ofs_x >= 0) && (ofs_x < dst->width) && 
					(ofs_y >= 0) && (ofs_y < dst->height) &&
					/*and in clip rectangle*/
					(ofs_x >= dst->clip_x) && (ofs_x < (dst->clip_x + dst->clip_w)) && 
					(ofs_y >= dst->clip_y) && (ofs_y < (dst->clip_y + dst->clip_h))){
					/* find the position on destination buffer */
					/* (y = dst->height - y) emulate the cartesian coordinates */
						
					if (dst->zero_tl != 0){
						ofs_dst = 4 * ((ofs_y * dst->width) + ofs_x);
					}
					else{
						ofs_dst = 4 * (((dst->height - 1 - ofs_y) * dst->width) + ofs_x);
					}
					if (src->zero_tl != 0){
						ofs_src = 4 * ((int)sy * src->width + (int)sx);
					}
					else{
						ofs_src = 4 * ((src->height - 1 - (int)sy) * src->width + (int)sx);
					}
					
					buf_r = dst->buf + ofs_dst + dst_r;
					buf_g = dst->buf + ofs_dst + dst_g;
					buf_b = dst->buf + ofs_dst + dst_b;
					buf_a = dst->buf + ofs_dst + dst_a;
					
					if (src->buf[ofs_src + src_a] < 255){
						/* Alpha compositing */
						/*normalize alfa channels */
						alpha_s = (double) src->buf[ofs_src + src_a] / 255;
						alpha_d = ((double)*buf_a / 255)*(1 - alpha_s);
						
						out.a = alpha_s + alpha_d;
						if (out.a == 0){
							out.r = 0;
							out.g = 0;
							out.b = 0;
						}
						else{
							out.r = (src->buf[ofs_src + src_r]*alpha_s + *buf_r*alpha_d)/out.a;
							out.g = (src->buf[ofs_src + src_g]*alpha_s + *buf_g*alpha_d)/out.a;
							out.b = (src->buf[ofs_src + src_b]*alpha_s + *buf_b*alpha_d)/out.a;
							out.a *= 255;
						}
						*buf_r = (unsigned char) (out.r);
						*buf_g = (unsigned char) (out.g);
						*buf_b = (unsigned char) (out.b);
						*buf_a = (unsigned char) (out.a);
					}
					else{
						/* store each component in memory buffer */
						*buf_r = src->buf[ofs_src + src_r];
						*buf_g = src->buf[ofs_src + src_g];
						*buf_b = src->buf[ofs_src + src_b];
						*buf_a = src->buf[ofs_src + src_a];
					}
				}
				sy += dy;
			}
			sx += dx;
		}
		
	}
}
	
/*If you scan along octants as explained for the Midpoint circle algorithm, your major coordinate y will always increase by one. You can then draw two circles at once, because their major coordinates are in sync.

Instead of placing pixels, you draw horizontal (and vertical) lines between the points of the inner and outer circle, which have the same y (or x) coordinate. You do so until the outer circle reaches the diagonal.

You keep the state with x and err for two circles, the inner circle i and the outer circle o. After the inner circle has reached the diagonal, the inner point lies on that diagonal. This means you are drawing eight adjoining octant sectors.

This idea is very similar to what @oakad proposed in the comments, but without the need for keeping a list. The Midpoint circle algorithm might be slower than the Bresenham algorithm, so there's probably room for improvement, but the low memory footprint is an advantage.

The code below will draw a hollow circle with the given inner and outer radii. The line width is ro - ri + 1, so that even equal radii will print a circle that is one pixel wide. It won't print anything if the inner radius is smaller than the outer radius.

void xLine(int x1, int x2, int y, int colour)
{
    while (x1 <= x2) setPixel(x1++, y, colour);
}

void yLine(int x, int y1, int y2, int colour)
{
    while (y1 <= y2) setPixel(x, y1++, colour);
}

void circle2(int xc, int yc, int inner, int outer, int colour)
{
    int xo = outer;
    int xi = inner;
    int y = 0;
    int erro = 1 - xo;
    int erri = 1 - xi;

    while(xo >= y) {
        xLine(xc + xi, xc + xo, yc + y,  colour);
        yLine(xc + y,  yc + xi, yc + xo, colour);
        xLine(xc - xo, xc - xi, yc + y,  colour);
        yLine(xc - y,  yc + xi, yc + xo, colour);
        xLine(xc - xo, xc - xi, yc - y,  colour);
        yLine(xc - y,  yc - xo, yc - xi, colour);
        xLine(xc + xi, xc + xo, yc - y,  colour);
        yLine(xc + y,  yc - xo, yc - xi, colour);

        y++;

        if (erro < 0) {
            erro += 2 * y + 1;
        } else {
            xo--;
            erro += 2 * (y - xo + 1);
        }

        if (y > inner) {
            xi = y;
        } else {
            if (erri < 0) {
                erri += 2 * y + 1;
            } else {
                xi--;
                erri += 2 * (y - xi + 1);
            }
        }
    }
}*/