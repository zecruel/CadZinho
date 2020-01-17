#ifndef _DXF_IMG_LIB
#define _DXF_IMG_LIB

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "list.h"

int dxf_image_clear_list(dxf_drawing *drawing);

bmp_img * dxf_image_def_list(dxf_drawing *drawing, dxf_node *img_def);

#endif