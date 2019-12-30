/* Reduced row echelon form adapted from
https://rosettacode.org/wiki/Reduced_row_echelon_form#C */

#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <ctype.h>
 
struct Matrix {
    int     dim_x, dim_y;
    double m_stor[1000*1000];
    double *mtx[1000];
};
 
void InitMatrix(struct Matrix *m, int x_dim, int y_dim){
	int i;
	m->dim_x = x_dim;
	m->dim_y = y_dim;
	for(i = 0; i < y_dim; i++)
		m->mtx[i] = m->m_stor + i *x_dim;
}
 
void MtxDisplay( struct Matrix * m )
{
    int iy, ix;
    const char *sc;
    for (iy=0; iy<m->dim_y; iy++) {
        printf("   ");
        sc = " ";
        for (ix=0; ix<m->dim_x; ix++) {
            printf("%s %0.9g", sc, m->mtx[iy][ix]);
            sc = ",";
        }
        printf("\n");
    }
    printf("\n");
}
 
void MtxMulAndAddRows(struct Matrix * m, int ixrdest, int ixrsrc, double mplr)
{
    int ix;
    double *drow, *srow;
    drow = m->mtx[ixrdest];
    srow = m->mtx[ixrsrc];
    for (ix=0; ix<m->dim_x; ix++) 
        drow[ix] += mplr * srow[ix];
}
 
void MtxSwapRows( struct Matrix * m, int rix1, int rix2)
{
    double *r1, *r2, temp;
    int ix;
    if (rix1 == rix2) return;
    r1 = m->mtx[rix1];
    r2 = m->mtx[rix2];
    for (ix=0; ix<m->dim_x; ix++)
        temp = r1[ix]; r1[ix]=r2[ix]; r2[ix]=temp;
}
 
void MtxNormalizeRow( struct Matrix * m, int rix, int lead)
{
    int ix;
    double *drow;
    double lv;
    drow = m->mtx[rix];
    lv = drow[lead];
    for (ix=0; ix<m->dim_x; ix++)
        drow[ix] /= lv;
}
 
#define MtxGet( m, rix, cix ) m->mtx[rix][cix]
 
void MtxToReducedREForm(struct Matrix * m)
{
    int lead;
    int rix, iix;
    double lv;
    int rowCount = m->dim_y;
 
    lead = 0;
    for (rix=0; rix<rowCount; rix++) {
        if (lead >= m->dim_x)
            return;
        iix = rix;
        while (0 == MtxGet(m, iix,lead)) {
            iix++;
            if (iix == rowCount) {
                iix = rix;
                lead++;
                if (lead == m->dim_x)
                    return;
            }
        }
        MtxSwapRows(m, iix, rix );
        MtxNormalizeRow(m, rix, lead );
        for (iix=0; iix<rowCount; iix++) {
            if ( iix != rix ) {
                lv = MtxGet(m, iix, lead );
                MtxMulAndAddRows(m,iix, rix, -lv) ;
            }
        }
        lead++;
    }
}

int basis_func(int order, double t, double knot[], double ret[], int num_pts){
	/*  Subroutine to generate rational B-spline basis functions

	Adapted from: An Introduction to NURBS- David F. Rogers - 2000 - Chapter 4, Sec. 4. , p 296

	order        = order of the B-spline basis function
	d        = first term of the basis function recursion relation
	e        = second term of the basis function recursion relation
	num_pts     = number of defining polygon vertices
	num_knots   = constant -- num_pts + order -- maximum number of knot values
	ret[]      = array containing the rationalbasis functions
	       ret[1] contains the basis function associated with B1 etc.
	t        = parameter value
	temp[]   = temporary array
	knot[]      = knot vector
	*/
	
	//int num_pts;
	int num_knots;
	int i,j,k;
	double d,e;
	double sum;
	double temp[1000];

	//num_pts = order + 1;
	num_knots = num_pts + order + 1;
	
	/* initialize temporary storage vector */
	for (i = 0; i< 1000; i++) temp[i] = 0.0;
	
	/* calculate the first order nonrational basis functions n[i]	*/
	for (i = 0; i < num_knots - 2; i++){
		if (( t >= knot[i]) && (t < knot[i+1]))
			temp[i] = 1;
		else
			temp[i] = 0;
	}

	/* calculate the higher order nonrational basis functions */
	for (k = 1; k <= order; k++){
		for (i = 0; i < num_knots - k; i++){
			if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
				d = ((t-knot[i])*temp[i])/(knot[i+k]-knot[i]);
			else d = 0;

			if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
				e = ((knot[i+k+1]-t)*temp[i+1])/(knot[i+k+1]-knot[i+1]);
			else e = 0;

			temp[i] = d + e;
		}
	}
	
	for (i = 0; i < num_pts; i++) ret[i] = temp[i];
}

int main(int argc, char** argv){
	struct Matrix *m1 = malloc(sizeof(struct Matrix));
	
	double t[] = {0, 5.0/17.0, 9.0/17.0, 14.0/17.0, 1};
	double knots[] = {0,0,0,0,28.0/51.0,1,1,1,1};
	double x[] = {0, 3, -1, -4, -4};
	double y[] = {0, 4, 4, 0, -3};
	
	double ret[1000];
	
	int i, j;
	
	InitMatrix(m1, 6, 5 );
	
	for (i = 0; i < 5; i++){
		basis_func(3, t[i], knots, ret, 5);
		for (j = 0; j < 5; j++){
			m1->mtx[i][j] = ret[j];
		}
		m1->mtx[i][5] = x[i];
	}
	
	m1->mtx[0][0] = 1.0;
	m1->mtx[4][4] = 1.0;
	
	//printf("Initial\n");
	//MtxDisplay(m1);
	MtxToReducedREForm(m1);
	printf("X\n");
	MtxDisplay(m1);
	

	
	for (i = 0; i < 5; i++){
		basis_func(3, t[i], knots, ret, 5);
		for (j = 0; j < 5; j++){
			m1->mtx[i][j] = ret[j];
		}
		m1->mtx[i][5] = y[i];
	}
	
	m1->mtx[0][0] = 1.0;
	m1->mtx[4][4] = 1.0;
	
	//printf("Initial\n");
	//MtxDisplay(m1);
	MtxToReducedREForm(m1);
	printf("Y\n");
	MtxDisplay(m1);

	free(m1);
	return 0;
}