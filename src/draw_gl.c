#include "draw_gl.h"

#define TOLERANCE 1e-6

/* mantain one unique element in a sorted array - array of integer values */
static int unique (int n, int * a) {
	int dst = 0, i;
	for (i = 1; i < n; ++i) {
	if (a[dst] != a[i])
		a[++dst] = a[i];
	}
	return dst + 1;
}

/* comparation function for qsort - array of integer values */
static int cmp_int (const void * a, const void * b) {
	return (*(int*)a - *(int*)b);
}

/* comparation function for qsort  - array of nodes */
static int cmp_node (const void * a, const void * b) {
	struct p_node *aa, *bb;
	aa = (struct p_node *) a; bb = (struct p_node *) b;
	
	int r = aa->up - bb->up;
	if (r > 0) return 1;
	else if (r < 0) return -1;
	/* if node.up values are same, compare node.low values */
	else return (aa->low - bb->low);
}

int draw_gl_line (struct ogl *gl_ctx, int p0[2], int p1[2], int thick){
	/* emulate drawing a single line with thickness, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	if (thick <= 0) thick = 1; /* one pixel as minimal thickness */
	
	/* get polar parameters of line */
	float dx = p1[0] - p0[0];
	float dy = p1[1] - p0[1];
	float modulus = sqrt(pow(dx, 2) + pow(dy, 2));
	float cosine = 1.0;
	float sine = 0.0;
	if (modulus > TOLERANCE){
		cosine = dx/modulus;
		sine = dy/modulus;
	}
	else {
		/* a dot*/
		p1[0] += thick;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units */
	float tx = (float) thick / gl_ctx->win_w;
	float ty = (float) thick / gl_ctx->win_h;
	float x0 = ((float) p0[0] / gl_ctx->win_w) * 2.0 - 1.0;
	float y0 = ((float) p0[1] / gl_ctx->win_h) * 2.0 - 1.0;
	float x1 = ((float) p1[0] / gl_ctx->win_w) * 2.0 - 1.0;
	float y1 = ((float) p1[1] / gl_ctx->win_h) * 2.0 - 1.0;
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x0 - sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y0 + cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x0 + sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y0 - cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x1 - sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y1 + cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = x1 + sine * tx;
	gl_ctx->verts[j].pos[1] = flip_y * (y1 - cosine * ty);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_quad (struct ogl *gl_ctx, int tl[2], int bl[2], int tr[2], int br[2]){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) bl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) bl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) tl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) br[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) br[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) tr[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tr[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_rect (struct ogl *gl_ctx, int x, int y, int w, int h){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (w > gl_ctx->tex_w || h > gl_ctx->tex_h){
		
		scale_u = 1.0;
		scale_v = 1.0;
		gl_ctx->tex_w = w;
		gl_ctx->tex_h = h;
	}
	else {
		
		scale_u = (float) w / (float) gl_ctx->tex_w;
		scale_v = (float) h / (float) gl_ctx->tex_h;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) x / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) y / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) x / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) (y + h) / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) (x + w) / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) y / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) (x + w) / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) (y + h) / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_triang (struct ogl *gl_ctx, int p0[2], int p1[2], int p2[2]){
	/* add a triangle to openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
		
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 3 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) p0[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) p0[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) p1[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) p1[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) p2[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) p2[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 1.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y);
	gl_ctx->vert_count ++;
	
	/* store vertex indexes in elements buffer - single element  */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count++;
	
	return 1;
}

int draw_gl_image (struct ogl *gl_ctx, int x, int y, int w, int h, bmp_img *img){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	if (!img) return 0;
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (img->width > gl_ctx->tex_w || img->height > gl_ctx->tex_h){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
		scale_u = 1.0;
		scale_v = 1.0;
		gl_ctx->tex_w = img->width;
		gl_ctx->tex_h = img->height;
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
		scale_u = (float) img->width / (float) gl_ctx->tex_w;
		scale_v = (float) img->height / (float) gl_ctx->tex_h;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) x / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) y / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) x / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) (y + h) / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = 0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) (x + w) / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) y / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] = scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) (x + w) / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) (y + h) / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_image2(struct ogl *gl_ctx, int tl[2], int bl[2], int tr[2], int br[2], bmp_img *img){
	/* emulate drawing a filled and convex quadrilateral, using triangles in openGL */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	/* orientation in drawing area */
	float flip_y = (gl_ctx->flip_y) ? -1.0 : 1.0;
	float scale_u = 1.0;
	float scale_v = 1.0;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_ctx->tex);
	
	if (img->width > gl_ctx->tex_w || img->height > gl_ctx->tex_h){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
		scale_u = 1.0;
		scale_v = 1.0;
		gl_ctx->tex_w = img->width;
		gl_ctx->tex_h = img->height;
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->width, img->height, GL_RGBA, GL_UNSIGNED_BYTE, img->buf);
		scale_u = (float) img->width / (float) gl_ctx->tex_w;
		scale_v = (float) img->height / (float) gl_ctx->tex_h;
	}
	
	/* convert input coordinates, in pixles (int), to openGL units and store vertices - 4 vertices */
	/* 0 */
	int j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) bl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) bl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  0.0;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 1 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) tl[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tl[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  0.0;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 2 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) br[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) br[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(1 - gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* 3 */
	j = gl_ctx->vert_count;
	gl_ctx->verts[j].pos[0] = ((float) tr[0] / gl_ctx->win_w) * 2.0 - 1.0;
	gl_ctx->verts[j].pos[1] = flip_y * (((float) tr[1] / gl_ctx->win_h) * 2.0 - 1.0);
	gl_ctx->verts[j].pos[2] = 0.0;
	gl_ctx->verts[j].col[0] = gl_ctx->fg[0];
	gl_ctx->verts[j].col[1] = gl_ctx->fg[1];
	gl_ctx->verts[j].col[2] = gl_ctx->fg[2];
	gl_ctx->verts[j].col[3] = gl_ctx->fg[3];
	gl_ctx->verts[j].uv[0] =  scale_u;
	gl_ctx->verts[j].uv[1] = (float)(gl_ctx->flip_y) * scale_v;
	gl_ctx->vert_count ++;
	/* store vertex indexes in elements buffer - 2 triangles that share vertices  */
	/* 0 */
	j = gl_ctx->elem_count * 3;
	gl_ctx->elems[j] = gl_ctx->vert_count - 4;
	gl_ctx->elems[j+1] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+2] = gl_ctx->vert_count - 2;
	/* 1 */
	gl_ctx->elems[j+3] = gl_ctx->vert_count - 3;
	gl_ctx->elems[j+4] = gl_ctx->vert_count - 2;
	gl_ctx->elems[j+5] = gl_ctx->vert_count - 1;
	gl_ctx->elem_count+= 2;
	
	return 1;
}

int draw_gl_polygon (struct ogl *gl_ctx, int n, struct edge edges[]){
	/* draw a arbitrary and filled polygon, in openGL - use a scanline like algorithm */
	
	/* verify struct and buffers */
	if (!gl_ctx) return 0;
	if (!gl_ctx->verts) return 0;
	if (!gl_ctx->elems) return 0;
	
	if (2 * n > MAX_SCAN_LINES) return 0;
	
	int scan_lines[MAX_SCAN_LINES]; /* scan line defined by y coordinate */
	struct p_node nodes[MAX_P_NODES];
	
	int i = 0, j = 0, scan_count = 0, nodes_count = 0;
	
	/* define a scanline in each edge vertex */
	for (i = 0; i < n; i++){
		scan_lines[scan_count] = edges[i].y0;
		scan_count++;
		scan_lines[scan_count] = edges[i].y1;
		scan_count++;
	}
	
	/* sort and delete duplicates in scanline array */
	qsort(scan_lines, scan_count, sizeof(int), cmp_int);
	scan_count = unique (scan_count, scan_lines);
	
	/* perform polygon fill, by draw quads in scanline pairs */
	for (i = 1; i < scan_count; i++){ /* sweep scanlines by pairs */
		nodes_count = 0;
		
		for(j = 0; j < n; j++){ /* sweep polygon edges */
			if (((edges[j].y0 <= scan_lines[i - 1] && edges[j].y1 >= scan_lines[i - 1]) || 
			(edges[j].y1 <= scan_lines[i - 1] && edges[j].y0 >= scan_lines[i - 1])) &&
			((edges[j].y0 <= scan_lines[i] && edges[j].y1 >= scan_lines[i]) || 
			(edges[j].y1 <= scan_lines[i ] && edges[j].y0 >= scan_lines[i]))){
				/* find interceptions between edges and  current scanlines pair */
				
				/* up scanline */
				if (edges[j].y0 == scan_lines[i - 1]) { /*interception in one vertex */
					nodes[nodes_count].up = edges[j].x0;
				}
				else if (edges[j].y1 == scan_lines[i-1]) {  /*interception in other vertex */
					nodes[nodes_count].up = edges[j].x1;
				}
				else {  /* claculate interception in ege*/
					nodes[nodes_count].up = (int) round(edges[j].x1 + (double) (scan_lines[i - 1] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
				}
				
				/* lower scanline */
				if (edges[j].y0 == scan_lines[i]) { /*interception in one vertex */
					nodes[nodes_count].low = edges[j].x0;
				}
				else if (edges[j].y1 == scan_lines[i]) { /*interception in other vertex */
					nodes[nodes_count].low = edges[j].x1;
				}
				else { /* claculate interception in ege*/
					nodes[nodes_count].low = (int) round(edges[j].x1 + (double) (scan_lines[i] - edges[j].y1)/(edges[j].y0 - edges[j].y1)*(edges[j].x0 - edges[j].x1));
				}
				
				nodes_count++;
			}
		}
		
		/* sort interception nodes, by x coordinates */
		qsort(nodes, nodes_count, sizeof(struct p_node), cmp_node);
		
		/* draw quads (or triangles) between nodes pairs*/
		for (j = 1; j < nodes_count; j+= 2){ /* triangle, if up corners are same */
			if (nodes[j - 1].up == nodes[j].up){
				draw_gl_triang (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1]},
					(int[]){nodes[j - 1].low, scan_lines[i]},
					(int[]){nodes[j].low, scan_lines[i]});
			}
			
			else if (nodes[j - 1].low == nodes[j].low){ /* triangle, if lower corners are same */
				draw_gl_triang (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1]},
					(int[]){nodes[j - 1].low, scan_lines[i]},
					(int[]){nodes[j].up, scan_lines[i-1]});
			}
			else { /* general case - quadrilateral polygon */
				draw_gl_quad (gl_ctx, (int[]){nodes[j - 1].up, scan_lines[i-1]},
					(int[]){nodes[j - 1].low, scan_lines[i]},
					(int[]){nodes[j].up, scan_lines[i-1]},
					(int[]){nodes[j].low, scan_lines[i]});
			}
		}
	}
	
	return 1;
}

int graph_draw_gl(graph_obj * master, struct ogl *gl_ctx, struct draw_param param){
	if ((master == NULL) || (gl_ctx == NULL)) return 0;
	/* check if list is not empty */
	if (master->list == NULL) return 0;
	if (master->list->next == NULL) return 0;
	
	double x0, y0, x1, y1;
	int xd0, yd0, xd1, yd1;
	double dx, dy, modulus, sine, cosine;
	line_node *current = master->list->next;
	int i, iter, thick;
	
	if (!current) return 0;
	
	/*extention corners */
	xd0 = 0.5 + (master->ext_min_x - param.ofs_x) * param.scale;
	yd0 = 0.5 + (master->ext_min_y - param.ofs_y) * param.scale;
	xd1 = 0.5 + (master->ext_max_x - param.ofs_x) * param.scale;
	yd1 = 0.5 + (master->ext_max_y - param.ofs_y) * param.scale;
	
	/* verify if current graph is inside window*/
	if ( (xd0 < 0 && xd1 < 0) || 
		(xd0 > gl_ctx->win_w && xd1 > gl_ctx->win_w) || 
		(yd0 < 0 && yd1 < 0) || 
		(yd0 > gl_ctx->win_h && yd1 > gl_ctx->win_h) ) return 0;
	
	/* define color */
	gl_ctx->fg[0] = master->color.r;
	gl_ctx->fg[1] = master->color.g;
	gl_ctx->fg[2] = master->color.b;
	gl_ctx->fg[3] = master->color.a;
	
	/* check if graph is legible (greater then 5 pixels) */
	if (xd1 - xd0 < 5 && yd1 - yd0 < 5 && current->next != NULL){
		/* draw a single triangle if not legible */
		draw_gl_triang (gl_ctx, (int[]){xd0, yd0}, (int[]){xd0, yd1}, (int[]){xd1, yd0});
		return 0;
	}
	
	/* if has a bitmap image associated */
	if (master->img){
		int tl[2], bl[2], tr[2], br[2];
		/* apply  offset an scale */
		/* first vertice */
		bl[0] = 0.5 + (current->x0 - param.ofs_x) * param.scale;
		bl[1] = 0.5 + (current->y0 - param.ofs_y) * param.scale;
		/* second vertice */
		br[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		br[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		current = current->next;
		if (!current) return 0;
		/* 3# vertice */
		tr[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		tr[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		current = current->next;
		if (!current) return 0;
		/* 4# vertice */
		tl[0] = 0.5 + (current->x1 - param.ofs_x) * param.scale;
		tl[1] = 0.5 + (current->y1 - param.ofs_y) * param.scale;
		
		/* draw bitmap image */
		draw_gl (gl_ctx, 1); /* force draw previous commands and cleanup */
		/* choose blank base color */
		gl_ctx->fg[0] = 255;
		gl_ctx->fg[1] = 255;
		gl_ctx->fg[2] = 255;
		gl_ctx->fg[3] = 255;
		/* prepare for new opengl commands */
		gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		glUniform1i(gl_ctx->tex_uni, 1); /* choose second texture */
		/* finally draw image */
		draw_gl_image2(gl_ctx, tl, bl, tr, br, master->img);
		draw_gl (gl_ctx, 1); /* force draw and cleanup */
		
		return 1;
	}
	
	if (master->flags & FILLED){
		/* filled polyon */
		i = 0;
		struct edge edges[2 * MAX_SCAN_LINES];
		
		while(current){ /*sweep the list content */
			/* apply the scale and offset */
			xd0 = 0.5 + (current->x0 - param.ofs_x) * param.scale;
			yd0 = 0.5 + (current->y0 - param.ofs_y) * param.scale;
			xd1 = 0.5 + (current->x1 - param.ofs_x) * param.scale;
			yd1 = 0.5 + (current->y1 - param.ofs_y) * param.scale;
			
			/* build edges array */
			edges[i] = (struct edge){xd0, yd0, xd1, yd1};
			
			current = current->next; /* go to next */
			if (i < 2 * MAX_SCAN_LINES - 1) i++;
			else current = NULL;
		}
		
		draw_gl_polygon (gl_ctx, i, edges);
	}
	else {
		/* set the thickness */
		if (master->flags & THICK_CONST) thick = (int) round(master->tick) + param.inc_thick;
		else thick = (int) round(master->tick * param.scale) + param.inc_thick;
		
		double patt_len = 0.0;
		/* get the pattern length */
		for (i = 0; i < master->patt_size && i < 20; i++){
			patt_len += fabs(master->pattern[i]);
		}
		
		if (master->patt_size > 1 &&   /* if graph is dashed lines */
			patt_len * param.scale > 3.0  /* and line pattern is legible (more then 3 pixels) */
		) {
			int patt_i = 0, patt_a_i = 0, patt_p_i = 0, draw;
			double patt_int, patt_part, patt_rem = 0.0, patt_acc, patt_rem_n;
			
			double p1x, p1y, p2x, p2y;
			double last;
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				
				x0 = current->x0;
				y0 = current->y0;
				x1 = current->x1;
				y1 = current->y1;
				
				/* get polar parameters of line */
				dx = x1 - x0;
				dy = y1 - y0;
				modulus = sqrt(pow(dx, 2) + pow(dy, 2));
				cosine = 1.0;
				sine = 0.0;
				
				if (modulus > TOLERANCE){
					cosine = dx/modulus;
					sine = dy/modulus;
				}
				
				/* initial point */
				draw = master->pattern[patt_i] >= 0.0;
				p1x = ((x0 - param.ofs_x) * param.scale);
				p1y = ((y0 - param.ofs_y) * param.scale);
				
				if (patt_rem <= modulus){ /* current segment needs some iterations over pattern */
				
					/* find how many interations over whole pattern */ 
					patt_part = modf((modulus - patt_rem)/patt_len, &patt_int);
					patt_part *= patt_len; /* remainder for the next step*/
					
					/* find how many interations over partial pattern */
					patt_a_i = 0;
					patt_p_i = patt_i;
					if (patt_rem > 0) patt_p_i++;
					if (patt_p_i >= master->patt_size) patt_p_i = 0;
					patt_acc = fabs(master->pattern[patt_p_i]);
					
					patt_rem_n = patt_part; /* remainder pattern for next segment continues */
					if (patt_part < patt_acc) patt_rem_n = patt_acc - patt_part;
					
					last = modulus - patt_int*patt_len - patt_rem; /* the last stroke (pattern fractional part) of current segment*/
					for (i = 0; i < master->patt_size && i < 20; i++){
						patt_a_i = i;
						if (patt_part < patt_acc) break;
						
						last -= fabs(master->pattern[patt_p_i]);
						
						patt_p_i++;
						if (patt_p_i >= master->patt_size) patt_p_i = 0;
						
						patt_acc += fabs(master->pattern[patt_p_i]);
						
						patt_rem_n = patt_acc - patt_part;
					}
					
					/* first stroke - remainder of past pattern*/
					p2x = patt_rem * param.scale * cosine + p1x;
					p2y = patt_rem * param.scale * sine + p1y;
					
					if (patt_rem > 0) {
						/*------------- complex line type ----------------*/
						if (master->cmplx_pat[patt_i] != NULL &&  /* complex element */
							p2x > 0 && p2x < gl_ctx->win_w && /* inside bound parameters */
							p2y > 0 && p2y < gl_ctx->win_h )
						{
							list_node *cplx = master->cmplx_pat[patt_i]->next;
							graph_obj *cplx_gr = NULL;
							line_node *cplx_lin = NULL;
							
							/* sweep the main list */
							while (cplx != NULL){
								if (cplx->data){
									cplx_gr = (graph_obj *)cplx->data;
									cplx_lin = cplx_gr->list->next;
									/* draw the lines */
									while(cplx_lin){ /*sweep the list content */
										xd0 = 0.5 + p2x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
										yd0 = 0.5 + p2y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
										xd1 = 0.5 + p2x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
										yd1 = 0.5 + p2y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
										
										//bmp_line(img, xd0, yd0, xd1, yd1);
										draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, 0);
										
										cplx_lin = cplx_lin->next; /* go to next */
									}
								}
								cplx = cplx->next;
							}
						}
						/*------------------------------------------------------*/
						
						patt_i++;
						if (patt_i >= master->patt_size) patt_i = 0;
						
						if (draw){
							//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
							xd0 = p1x + 0.5; yd0 = p1y + 0.5;
							xd1 = p2x + 0.5; yd1 = p2y + 0.5;
							draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, thick);
						}
					}
					
					patt_rem = patt_rem_n; /* for next segment */
					p1x = p2x;
					p1y = p2y;
					
					/* draw pattern */
					iter = (int) (patt_int * (master->patt_size)) + patt_a_i;
					for (i = 0; i < iter; i++){					
						draw = master->pattern[patt_i] >= 0.0;
						p2x = fabs(master->pattern[patt_i]) * param.scale * cosine + p1x;
						p2y = fabs(master->pattern[patt_i]) * param.scale * sine + p1y;
						if (draw){
							//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
							xd0 = p1x + 0.5; yd0 = p1y + 0.5;
							xd1 = p2x + 0.5; yd1 = p2y + 0.5;
							draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, thick);
						}
						p1x = p2x;
						p1y = p2y;
						
						/*------------- complex line type ----------------*/
						if (master->cmplx_pat[patt_i] != NULL &&  /* complex element */
							p1x > 0 && p1x < gl_ctx->win_w && /* inside bound parameters */
							p1y > 0 && p1y < gl_ctx->win_h )
						{
							list_node *cplx = master->cmplx_pat[patt_i]->next;
							graph_obj *cplx_gr = NULL;
							line_node *cplx_lin = NULL;
							
							/* sweep the main list */
							while (cplx != NULL){
								if (cplx->data){
									cplx_gr = (graph_obj *)cplx->data;
									cplx_lin = cplx_gr->list->next;
									/* draw the lines */
									while(cplx_lin){ /*sweep the list content */
										
										xd0 = 0.5 + p1x + ((cplx_lin->x0 * cosine -  cplx_lin->y0 * sine) * param.scale);
										yd0 = 0.5 + p1y + ((cplx_lin->x0 * sine +  cplx_lin->y0 * cosine) * param.scale);
										xd1 = 0.5 + p1x + ((cplx_lin->x1 * cosine -  cplx_lin->y1 * sine) * param.scale);
										yd1 = 0.5 + p1y + ((cplx_lin->x1 * sine +  cplx_lin->y1 * cosine) * param.scale);
										
										//bmp_line(img, xd0, yd0, xd1, yd1);
										draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, 0);
										
										cplx_lin = cplx_lin->next; /* go to next */
									}
								}
								cplx = cplx->next;
							}
						}
						/*------------------------------------------------------*/
						
						patt_i++;
						if (patt_i >= master->patt_size) patt_i = 0;
					}
					
					p2x = last * param.scale * cosine + p1x;
					p2y = last * param.scale * sine + p1y;
					draw = master->pattern[patt_i] >= 0.0;
					if (draw) {//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
						xd0 = p1x + 0.5; yd0 = p1y + 0.5;
						xd1 = p2x + 0.5; yd1 = p2y + 0.5;
						draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, thick);
					}
				}
				else{ /* current segment is in same past iteration pattern */
					p2x = modulus * param.scale * cosine + p1x;
					p2y = modulus * param.scale * sine + p1y;
					
					patt_rem -= modulus;
				
					if (draw){
						//bmp_line_norm(img, p1x, p1y, p2x, p2y, -sine, cosine);
						xd0 = p1x + 0.5; yd0 = p1y + 0.5;
						xd1 = p2x + 0.5; yd1 = p2y + 0.5;
						draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, thick);
					}
					p1x = p2x;
					p1y = p2y;
				}
			
				current = current->next; /* go to next */
			}
		}
		else{ /* for continuous lines*/
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				
				xd0 = 0.5 + (current->x0 - param.ofs_x) * param.scale;
				yd0 = 0.5 + (current->y0 - param.ofs_y) * param.scale;
				xd1 = 0.5 + (current->x1 - param.ofs_x) * param.scale;
				yd1 = 0.5 + (current->y1 - param.ofs_y) * param.scale;
				
				if (master->pattern[0] >= 0.0)
					//bmp_line_norm(img, x0, y0, x1, y1, -sine, cosine);
					draw_gl_line (gl_ctx, (int []){xd0, yd0}, (int []){ xd1, yd1}, thick);
				
				current = current->next; /* go to next */
			}
		}
	}
	
}

int draw_gl (struct ogl *gl_ctx, int force){
	if (!gl_ctx) return 0;
	if (gl_ctx->elem_count > MAX_TRIANG_2 || force ){
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glDrawElements(GL_TRIANGLES, gl_ctx->elem_count*3, GL_UNSIGNED_INT, 0);
		gl_ctx->vert_count = 0;
		gl_ctx->elem_count = 0;
		gl_ctx->elems = NULL;
		gl_ctx->verts = NULL;
		glUniform1i(gl_ctx->tex_uni, 0);
		return 1;
	}
	return 0;
}

int graph_list_draw_gl(list_node *list, struct ogl *gl_ctx, struct draw_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				
				/*gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->vert_count = 0;
				gl_ctx->elem_count = 0;
				glUniform1i(gl_ctx->tex_uni, 0);*/
				
				graph_draw_gl(curr_graph, gl_ctx, param);
				
				/*glUnmapBuffer(GL_ARRAY_BUFFER);
				glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
				glDrawElements(GL_TRIANGLES, gl_ctx->elem_count*3, GL_UNSIGNED_INT, 0);*/
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_draw_gl2(list_node *list, struct ogl *gl_ctx, struct draw_param param){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list == NULL) return 0;
	if (!gl_ctx) return 0;
	
	current = list->next;
	
	/* sweep the main list */
	while (current != NULL){
		if (current->data){
			curr_graph = (graph_obj *)current->data;
			
			if (gl_ctx->elems == NULL){
				gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
			}
			
			graph_draw_gl(curr_graph, gl_ctx, param);
			
			draw_gl (gl_ctx, 0);
		}
		current = current->next;
	}
	ok = 1;
	return ok;
}

int dxf_ents_draw_gl(dxf_drawing *drawing, struct ogl *gl_ctx, struct draw_param param){
	dxf_node *current = NULL;
	//int lay_idx = 0;
		
	if ((drawing->ents != NULL) && (drawing->main_struct != NULL)){
		current = drawing->ents->obj.content->next;
		
		int init = 0;
		/* starts the content sweep  */
		while (current != NULL){
			if (current->type == DXF_ENT){ /* DXF entity */
				/*verify if entity layer is on and thaw */
				if ((!drawing->layers[current->obj.layer].off) && 
					(!drawing->layers[current->obj.layer].frozen)){
					if (!init){
						gl_ctx->verts = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
						gl_ctx->elems = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
						gl_ctx->vert_count = 0;
						gl_ctx->elem_count = 0;
						glUniform1i(gl_ctx->tex_uni, 0);
						
						init = 1;
					}
					/* draw each entity */
					graph_list_draw_gl2(current->obj.graphics, gl_ctx, param);
					
				}
			}
			current = current->next;
		}
		draw_gl (gl_ctx, 1); /* force draw and cleanup */
	}
}