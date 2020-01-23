#ifndef _DXF_IMG_LIB
#define _DXF_IMG_LIB

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"

struct dxf_img_def { /* struct to store images indexed by DXF handle */
	long id;
	bmp_img *img;
};

int dxf_image_clear_list(dxf_drawing *drawing);

bmp_img * dxf_image_def_list(dxf_drawing *drawing, dxf_node *img_def);

#endif