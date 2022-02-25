#ifndef _DXF_EXPORT_LIB
#define _DXF_EXPORT_LIB

#include "dxf.h"
#include "graph.h"
#include "list.h"
#include "font.h"
#include "dxf_graph.h"


enum export_fmt{
	EXPORT_HPGL,
	EXPORT_GCODE,
	
	EXPORT_NONE,
	EXPORT_SIZE
};

struct export_param{
	double ofs_x;
	double ofs_y;
	double scale;
	double resolution;
	char init[DXF_MAX_CHARS+1];
	char end[DXF_MAX_CHARS+1];
	char move[DXF_MAX_CHARS+1];
	char stroke[DXF_MAX_CHARS+1];
	enum export_fmt out_fmt;
};

int export_hpgl(dxf_drawing *drawing, struct export_param param, char *dest);

int export_gcode(dxf_drawing *drawing, struct export_param param, char *dest);

#endif