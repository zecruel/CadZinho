#ifndef _DXF_ATTRACT_LIB
#define _DXF_ATTRACT_LIB

#include "dxf.h"
#include <math.h>
#include "dxf_graph.h"

enum attract_type {
	/* bit coded */
	ATRC_NONE = 0, 	/* atrractor disabled */
	ATRC_END = 1,		/* endpoint */
	ATRC_MID = 2,		/* midpoint */
	ATRC_CENTER = 4,	/* center (elliptical arcs) */
	ATRC_OCENTER = 8,	/* object center */
	ATRC_NODE = 16,	/* node */
	ATRC_QUAD = 32,	/* quadrant (elliptical arcs) */
	ATRC_INTER = 64,	/* intersection */
	ATRC_EXT = 128,	/* extension */
	ATRC_INS = 256,	/* insertion */
	ATRC_PERP = 512,	/* perpenticular */
	ATRC_TAN = 1024,	/* tangent */
	ATRC_PAR = 2048,	/* parallel */
	ATRC_CTRL = 4096,	/* control point (spline) */
	ATRC_AINT = 8192,	/* apparent intersection (3D) */
	ATRC_ANY = 16384	/* any or nearest */
};

struct ins_space{
	dxf_node *ins_ent, *prev;
	double ofs_x, ofs_y, ofs_z;
	double rot, scale_x, scale_y, scale_z;
	double normal[3];
};

struct inter_obj{
	int type;
	union {
		struct {
			double p1x;
			double p1y;
			double p2x;
			double p2y;
			double bulge;
		} line;
		struct {
			double cx;
			double cy;
			double axis;
			double ratio;
			double rot;
			double ang_start;
			double ang_end;
		} arc;
	};
};

int dxf_ent_attract (dxf_drawing *drawing, dxf_node * obj_hilite, enum attract_type type,
double pos_x, double pos_y, double ref_x, double ref_y, double sensi, double *ret_x, double *ret_y);

int seg_inter2(double l1p1x, double l1p1y, double l1p2x, double l1p2y, 
double l2p1x, double l2p1y, double l2p2x, double l2p2y, 
double *ret_x, double *ret_y);

int line_inter(double a1, double b1, double c1,
double a2, double b2, double c2,
double *ret_x, double *ret_y);

int el_ln_inter( double center_x, double center_y, 
double axis, double ratio, double rot,
double ln_a, double ln_b, double ln_c, /*line in general form ax+bx = c*/
double inter_x[2], double inter_y[2]);

double ellipse_par (double ang, double a, double b);

#endif