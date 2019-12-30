#ifndef _RREF_LIB
#define _RREF_LIB

#include <stdio.h>
 
struct Matrix {
    int     dim_x, dim_y;
    double m_stor[1000*1000];
    double *mtx[1000];
};


void InitMatrix(struct Matrix *m, int x_dim, int y_dim);

void MtxToReducedREForm(struct Matrix * m);

#endif