#ifndef _DXF_PRINT_LIB
#define _DXF_PRINT_LIB

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

struct print_param{
	double w;
	double h;
	double ofs_x;
	double ofs_y;
	double scale;
	double resolution;
	int mono;
	int inch;
	bmp_color *list;
	bmp_color *subst;
	int len;
};

int print_pdf(dxf_drawing *drawing, struct print_param param, char *dest);

#endif