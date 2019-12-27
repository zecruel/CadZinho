/* Reduced row echelon form adapted from
https://rosettacode.org/wiki/Reduced_row_echelon_form#C */

#include "rref.h"
 
void MtxSetRow(struct Matrix * m, int irow, double *v){
    int ix;
    double *mr;
    mr = m->mtx[irow];
    for(ix=0; ix<m->dim_x; ix++)
        mr[ix] = v[ix];
}
 
void InitMatrix(struct Matrix *m, int x_dim, int y_dim, double **v){
	int i;
	m->dim_x = x_dim;
	m->dim_y = y_dim;
	for(i = 0; i < y_dim; i++)
		m->mtx[i] = m->m_stor + i *x_dim;
	for (i = 0; i < y_dim; i++) 
		MtxSetRow(m, i, v[i]);
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