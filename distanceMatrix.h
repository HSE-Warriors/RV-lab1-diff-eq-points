#ifndef TRIANGULAR_MATRIX_H
#define TRIANGULAR_MATRIX_H

#include <pthread.h>


typedef struct {
    int dimension;
    double* elements;
} TriangularMatrix;

int getIndex(int i, int j, int n);
TriangularMatrix* CreateTriangularMatrix(int matrixDimension);
void SetTriangularMatrixElement(TriangularMatrix* matrix, double element, int i, int j);
double GetTriangularMatrixElement(TriangularMatrix* matrix, int i, int j);
void ClearTriangularMatrix(TriangularMatrix* matrix);

#endif /* TRIANGULAR_MATRIX_H */
