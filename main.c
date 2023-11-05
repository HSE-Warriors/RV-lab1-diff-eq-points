#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define DT 0.05

typedef struct
{
    double x, y;
} vector;

int bodies, timeSteps, bodiesPerThread;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;
FILE *outputFile;

typedef struct {
    int startIndex;
    int endIndex;
} thread_args_t;

int threadsCount;
int extraBodiesToLastThread;
pthread_barrier_t barrier;
pthread_t* pthread_arr;



void initiateSystem(char *fileName, int numberOfThreads)
{
    int i;
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);
    bodiesPerThread = bodies / numberOfThreads;
    extraBodiesToLastThread = bodies % numberOfThreads;
    printf("bodiesPerThread =%d, extraBodiesToLastThread=%d\n", bodiesPerThread, extraBodiesToLastThread);

    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));


    for (i = 0; i < bodies; i++)
    {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
    }
    fclose(fp);

    threadsCount = numberOfThreads;
    pthread_arr = malloc(threadsCount * sizeof(pthread_t));
    int rc = pthread_barrier_init(&barrier, NULL, threadsCount);
    if (rc != 0) {
        perror("Error with barrier init");
        exit(1);
    }
    
}

void* routine(void *args){
    thread_args_t *thread_args = (thread_args_t *)args;

    printf("Thread working on index range [%d, %d]\n", thread_args->startIndex, thread_args->endIndex);
    for (long long i = 0; i < timeSteps; i++) {
        
    }
    free(args);
}


int main(int argC, char *argV[])
{
    int i, j;
    printf("%s", &argC);
    if (argC != 3)
        printf("Usage : %s <file name containing system configuration data>", argV[0]);
    else
    {
        int numberOfThreads = atoi(argV[2]);
        initiateSystem(argV[1], numberOfThreads);
        outputFile = fopen("intro/oneThreadExample-output", "w");
        if (outputFile == NULL) {
            perror("Error opening output file");
            return 1; 
        }
        fprintf(outputFile, "Body   :     x              y           vx              vy");
        for (long long i = 0; i < threadsCount; ++i){
            thread_args_t *args = (thread_args_t *)malloc(sizeof(thread_args_t));
            args->startIndex = i * bodiesPerThread;
            args->endIndex = args->startIndex + bodiesPerThread - 1;
            if (i == threadsCount - 1) {
                args-> endIndex += extraBodiesToLastThread;
            }
            pthread_create(&pthread_arr[i], NULL, routine, (void *)args);
        }

        for (long long i = 0; i < threadsCount; ++i){
            pthread_join(pthread_arr[i], NULL);
        }
        printf("End of program, free and close everything");
        fclose(outputFile);
        free(pthread_arr);
        free(masses);
        free(positions);
        free(velocities);
        free(accelerations);
        pthread_barrier_destroy(&barrier);
    }
}