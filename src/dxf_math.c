#include "dxf_math.h"

double dot_product(double a[3], double b[3]){
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
 
void cross_product(double a[3], double b[3], double c[3]){
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}

void unit_vector(double a[3]){
	double mod;
	
	mod = sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
	if (mod > 0.0) {
		a[0] /= mod;
		a[1] /= mod;
		a[2] /= mod;
	}
}


void arc_bulge(double pt1_x, double pt1_y,
double pt2_x, double pt2_y, double bulge,
double *radius, double *ang_start, double *ang_end,
double *center_x, double *center_y){
	
	double theta, alfa, d, ang_c, start, end;
	/* some math to find radius and center point */
	
	theta = 2 * atan(bulge);
	alfa = atan2(pt2_y-pt1_y, pt2_x-pt1_x);
	d = sqrt((pt2_y-pt1_y)*(pt2_y-pt1_y) + (pt2_x-pt1_x)*(pt2_x-pt1_x)) / 2;
	*radius = d*(bulge*bulge + 1)/(2*bulge);
	
	ang_c = M_PI+(alfa - M_PI/2 - theta);
	*center_x = *radius*cos(ang_c) + pt1_x;
	*center_y = *radius*sin(ang_c) + pt1_y;
	
	/* start and end angles found by points coordinates*/
	start = atan2(pt1_y - *center_y, pt1_x - *center_x);
	end = atan2(pt2_y - *center_y, pt2_x - *center_x);
	
	/* set angle range to 0-2*pi */
	if (start < 0){
		start += 2*M_PI;
	}
	if (end < 0){
		end += 2*M_PI;
	}
	if (*radius >= 0){
		*ang_start = start;
		*ang_end = end;
	}
	else{
		*ang_start = end;
		*ang_end = start;
		*radius = fabs(*radius);
	}
}

double ellipse_par (double ang, double a, double b){
	/* find the polar parameter (t) for ellipse */
	double t = atan(a*tan(ang)/b);
	if ((ang > M_PI/2) && (ang < M_PI)){
		t += M_PI; 
	}
	else if ((ang >= M_PI) && (ang <= 3*M_PI/2)){
		t -= M_PI; 
	}
	if (t < 0 ) t += 2*M_PI;
	
	return t;
}

//Normalize to [0,360):

double ang_adjust_360(double x){
    x = fmod(x,360.0);
    if (x < 0)
        x += 360.0;
    return x;
}
/*Normalize to [-180,180):

double constrainAngle(double x){
    x = fmod(x + 180,360);
    if (x < 0)
        x += 360;
    return x - 180;
}*/

void mod_axis(double result[3], double normal[3] , double elev){
	
	double x_axis[3], y_axis[3], point[3], x_col[3], y_col[3], z_col[3], norm[3];
	double wy_axis[3] = {0.0, 1.0, 0.0};
	double wz_axis[3] = {0.0, 0.0, 1.0};
	double x0, y0, z0;
	
	/* axis tranform */
	/* choose absolute world axis, according DXF spec, and obtain x axis */
	if ((fabs(normal[0] < 0.015625)) && (fabs(normal[1] < 0.015625))){
		cross_product(wy_axis, normal, x_axis);
	}
	else{
		cross_product(wz_axis, normal, x_axis);
	}
	cross_product(normal, x_axis, y_axis); /*obtain y axis */
	
	/* normalize axis */
	norm[0] = normal[0]; norm[1] = normal[1]; norm[2] = normal[2]; /* preserve original vector */
	unit_vector(x_axis); 
	unit_vector(y_axis);
	unit_vector(norm);
	
	x_col[0] = x_axis[0];
	x_col[1] = y_axis[0];
	x_col[2] = norm[0];
	
	y_col[0] = x_axis[1];
	y_col[1] = y_axis[1];
	y_col[2] = norm[1];
	
	z_col[0] = x_axis[2];
	z_col[1] = y_axis[2];
	z_col[2] = norm[2];
	
	/*obtain result point */
	point[0] = result[0];
	point[1] = result[1];
	point[2] = result[2];
	if (fabs(point[2]) < 1e-9) point[2] = elev;
	x0 = dot_product(point, x_col);
	y0 = dot_product(point, y_col);
	z0 = dot_product(point, z_col);
	
	/* save results */
	result[0] = x0;
	result[1] = y0;
	result[2] = z0;
	
}