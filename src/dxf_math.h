#ifndef _DXF_MATH_LIB
#define _DXF_MATH_LIB

#include "dxf.h"
#include <math.h>

double dot_product(double a[3], double b[3]);

void cross_product(double a[3], double b[3], double c[3]);

void unit_vector(double a[3]);

void arc_bulge(double pt1_x, double pt1_y,
double pt2_x, double pt2_y, double bulge,
double *radius, double *ang_start, double *ang_end,
double *center_x, double *center_y);

double ellipse_par (double ang, double a, double b);

void angle_range(double *ang);

double ang_adjust_360(double x);

void mod_axis(double result[3], double normal[3] , double elev);

#endif