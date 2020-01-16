#ifndef _BMP_LIB
#define _BMP_LIB

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



/* position of a point in rectangle */
enum Rect_pos{  /* is bit coded */
	INSIDE = 0, /* 0000 */
	LEFT = 1,   /* 0001 */
	RIGHT = 2,  /* 0010 */
	BOTTOM = 4, /* 0100 */
	TOP = 8    /* 1000 */
};
typedef enum Rect_pos rect_pos;

struct Bmp_color {
	/* the color depth is 8 bit per component */
	unsigned char r, g, b, a;
};
typedef struct Bmp_color bmp_color;

struct Bmp_img {
	unsigned int width;
	unsigned int height;
	bmp_color bkg; /* background color */
	bmp_color frg; /* current foreground color */
	
	int pattern[20]; /* pattern information */
	int patt_size;
	int patt_i; /* current pattern index */
	double pat_scale;
	int pix_count;

	unsigned int tick; /* current brush tickness */
	
	/* index of each color component in buffer order */
	unsigned int r_i, g_i, b_i, a_i;
	
	unsigned int zero_tl; /* flag of zero in top left corner of image */
	
	
	/* the pixmap */
	unsigned char * buf; /* the color depth is 8 bit per component */
	
	/*clipping rectangle */
	int clip_x;
	int clip_y;
	int clip_w;
	int clip_h;
	
	/* lines conection parameters */
	int prev_x, prev_y;
	int end_x[4];
	int end_y[4];
	
};
typedef struct Bmp_img bmp_img;


int line_clip(bmp_img *img, double *x0, double *y0, double *x1, double *y1);

rect_pos rect_find_pos(double x, double y, double xmin, double ymin, double xmax, double ymax);

void bmp_fit(bmp_img *img, double min_x, double min_y, double max_x, double max_y, double *zoom, double *ofs_x, double *ofs_y);

int bmp_fill (bmp_img *img, bmp_color color);

int bmp_fill_clip (bmp_img *img, bmp_color color);

bmp_img * bmp_new (unsigned int width, unsigned int height, bmp_color bkg, bmp_color frg);

void bmp_free(bmp_img *img);

int bmp_save (char *path, bmp_img *img);

void bmp_point_raw (bmp_img *img, int x, int y);

int patt_change(bmp_img *img, double patt[], int size);

int patt_check(bmp_img *img);

void bmp_line(bmp_img *img, double x0, double y0, double x1, double y1);

void bmp_line_norm(bmp_img *img, double x0, double y0, double x1, double y1, double normal_x, double normal_y);

void bmp_rect_fill(bmp_img *img, int vert_x[4], int vert_y[4]);

void bmp_copy(bmp_img *src, bmp_img *dst, int x, int y);

void bmp_thick_line(bmp_img *img, int x0, int y0, int x1, int y1);

void bmp_thin_line(bmp_img *img, int x0, int y0, int x1, int y1) ;

void bmp_poly_fill(bmp_img *img, int verts, int vert_x[], int vert_y[], int stroke[]);

bmp_img * bmp_sub_img(bmp_img *orig, int x, int y, int w, int h);

bmp_img * bmp_load_img(char *url);

bmp_img * bmp_load_img2(unsigned char *data, int w, int h);

void bmp_circle(bmp_img *img, int x0, int y0, int radius);

void bmp_circle_fill(bmp_img *img, int x0, int y0, int radius);

void bmp_put(bmp_img *src, bmp_img *dst, int x, int y, double u[3], double v[3]);

#endif