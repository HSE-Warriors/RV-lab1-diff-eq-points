#include <distanceMatrix.h>
#include <stdlib.h>
#include <stdio.h>

pthread_mutex_t *bodiesDistancesMutexes;

int getIndex(int i, int j, int n) {
    return (i * n - (i - 1) * i / 2 + j - i);
}

TriangularMatrix* CreateTriangularMatrix(int matrixDimension) {
    TriangularMatrix* matrix = (TriangularMatrix*)malloc(sizeof(TriangularMatrix));
    int size = matrixDimension * (matrixDimension + 1) / 2;

    matrix->dimension = matrixDimension;
    matrix->elements = (double*)malloc(size * sizeof(double));
    if (matrix->elements == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    bodiesDistancesMutexes = (pthread_mutex_t *)malloc(size * sizeof(pthread_mutex_t));
    for (int i = 0; i < size; i++) {    
        pthread_mutex_init(&bodiesDistancesMutexes[i], NULL);
    }

    return matrix;
}

void SetTriangularMatrixElement(TriangularMatrix* matrix, double element, int i, int j) {
    if (i > j || i >= matrix->dimension || j >= matrix->dimension) {
        printf("It is impossible to insert an element outside the upper triangular matrix.\n");
        return;
    }
    int index = getIndex(i, j, matrix->dimension);
    pthread_mutex_lock(&bodiesDistancesMutexes[index]);
    matrix->elements[getIndex(i, j, matrix->dimension)] = element;
    pthread_mutex_unlock(&bodiesDistancesMutexes[index]);
}

double GetTriangularMatrixElement(TriangularMatrix* matrix, int i, int j) {
    if (i > j || i >= matrix->dimension || j >= matrix->dimension) {
        printf("It is impossible to take an element outside the upper triangular matrix.\n");
        return 0;
    }
    int index = getIndex(i, j, matrix->dimension);
    pthread_mutex_lock(&bodiesDistancesMutexes[index]);
    double result = matrix->elements[index];
    pthread_mutex_unlock(&bodiesDistancesMutexes[index]);
    return result;
}

void ClearTriangularMatrix(TriangularMatrix* matrix) {
    if (matrix != NULL) {
        if (matrix->elements != NULL) {
            free(matrix->elements);
            matrix->elements = NULL;
        }
        matrix->dimension = 0;
    }
    free(matrix);
}