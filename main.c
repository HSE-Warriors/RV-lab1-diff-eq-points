#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "distanceMatrix.h"

#define DT 0.05

typedef struct
{
    double x, y;
} vector;

typedef void (*Callback)(int);

typedef struct {
    int startIndex;
    int endIndex;
} thread_args_t;

int bodies, timeSteps, bodiesPerThread;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;
TriangularMatrix *distancesMatrix;
FILE *outputFile;
int nOfThreads;
int nOfFinishedThreads = 0;
int extraBodiesToLastThread = 0;

pthread_t* pthread_arr;
pthread_mutex_t *bodiesMutexes;
pthread_mutex_t threadsCounterMutex;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void log_vectors(const char *name, vector *array, size_t length, FILE *outputFile) {
    for (size_t i = 0; i < length; i++) {
        fprintf(outputFile, " (%lf, %lf)", array[i].x, array[i].y);
    }
    fprintf(outputFile,"\n");
}

vector addVectors(vector a, vector b)
{
    vector c = {a.x + b.x, a.y + b.y};
    return c;
}

vector scaleVector(double b, vector a)
{
    vector c = {b * a.x, b * a.y};
    return c;
}

vector subtractVectors(vector a, vector b)
{
    vector c = {a.x - b.x, a.y - b.y};
    return c;
}

double mod(vector a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

vector normalize(vector a) {
    double length = mod(a);
    if (length == 0) {
        vector zero = {0.0, 0.0};
        return zero;
    }
    vector c = {a.x / length, a.y / length};
    return c;
}

double distanceBetweenVectors(vector a, vector b) {
    return mod(subtractVectors(a, b));
}

void countAcceleration(int i, int j) {
    double firstMass, secondMass;
    vector firstPosition, secondPosition;

    firstMass = masses[i];
    firstPosition = positions[i];
    secondMass = masses[j];
    secondPosition = positions[j];

    double distance = distanceBetweenVectors(firstPosition, secondPosition);
    double force = GravConstant * firstMass * secondMass / (distance * distance);
    vector forceVector = scaleVector(force, normalize(subtractVectors(secondPosition, firstPosition)));

    vector firstNewAcceleration = scaleVector(1.0 / firstMass, forceVector);
    vector secondNewAcceleration = scaleVector(-1.0 / secondMass, forceVector);


    pthread_mutex_lock(&bodiesMutexes[i]);
    accelerations[i] = addVectors(accelerations[i], firstNewAcceleration);
    pthread_mutex_unlock(&bodiesMutexes[i]);

    pthread_mutex_lock(&bodiesMutexes[j]);
    accelerations[j] = addVectors(accelerations[j], secondNewAcceleration);
    pthread_mutex_unlock(&bodiesMutexes[j]);
}


void countPositions(int i) {
    //х=х0+v*t
    positions[i] = addVectors(positions[i], scaleVector(DT, velocities[i]));
}

void countVelocities(int i) {
    //V=V0+a*t
    velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
}

void resolveCollisions(int i) {
    for (long j = i + 1; j < bodies; j++) {
        if (positions[i].x == positions[j].x && positions[i].y == positions[j].y)
        {
            //If bodies collide, then we just V = V0 * -1, same speed to the opposite direction
            pthread_mutex_lock(&bodiesMutexes[i]);
            pthread_mutex_lock(&bodiesMutexes[j]);
            vector temp = velocities[i];
            velocities[i] = velocities[j];
            velocities[j] = temp;
            pthread_mutex_unlock(&bodiesMutexes[i]);
            pthread_mutex_unlock(&bodiesMutexes[j]);
        }
    }
}


//analog of pthread barrier on cond var
void condVarWait(int value, Callback callback) {
    pthread_mutex_lock(&threadsCounterMutex);
    nOfFinishedThreads++;
    if(nOfFinishedThreads == nOfThreads) {
        callback(value);
        nOfFinishedThreads = 0;
        pthread_cond_broadcast(&cond_var);
    } else {
        while(pthread_cond_wait(&cond_var, &threadsCounterMutex) != 0);
    }
    pthread_mutex_unlock(&threadsCounterMutex);
}

void printCoordsToFile_CleanAccelerations(int timestampIndex) {
    fprintf(outputFile, "\nCycle %d\n", timestampIndex + 1);
    for (int j = 0; j < bodies; j++) {
        fprintf(outputFile, "Body %d : %lf\t%lf\t%lf\t%lf\n",
            j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
    }
    for (size_t i = 0; i < bodies; ++i) {
        accelerations[i].x = 0.0;
        accelerations[i].y = 0.0;
    }
}

void emptyCallback(int value) {}

void* routine(void *threadArgs) {
    thread_args_t *args = (thread_args_t *)threadArgs;

    printf("Thread working on index range [%d, %d]\n", args->startIndex, args->endIndex);
    for (long t = 0; t < timeSteps; t++) {
        //Count distances to other threads
        for (long i = 0; i < args->startIndex; i++) {
            //count distances
        }
        //Accelerations
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            for (long j = i+1; j <= bodies - 1; j++) {
                countAcceleration(i, j);
            }
        }
        condVarWait(0, emptyCallback);

        //Positions
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countPositions(i);
        }
        condVarWait(0, emptyCallback);

        //Velocities and collisitons
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countVelocities(i);
            resolveCollisions(i);
        }
        condVarWait(t, printCoordsToFile_CleanAccelerations);
    }
    free(args);
}

void initiateSystem(char *fileName, int numberOfThreads)
{
    FILE *fp = fopen(fileName, "r");
    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps);

    bodiesMutexes = (pthread_mutex_t *)malloc(bodies * sizeof(pthread_mutex_t));
    masses = (double *)malloc(bodies * sizeof(double));
    positions = (vector *)malloc(bodies * sizeof(vector));
    velocities = (vector *)malloc(bodies * sizeof(vector));
    accelerations = (vector *)malloc(bodies * sizeof(vector));
    distancesMatrix = CreateTriangularMatrix(bodies);
    
    for (int i = 0; i < bodies; i++)
    {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
        pthread_mutex_init(&bodiesMutexes[i], NULL);
    }
    fclose(fp);

    if (bodies < numberOfThreads) {
        bodiesPerThread = 1; //no reason to spread to more threads than bodies, so limit to 1:1
    } else {
        bodiesPerThread = bodies / numberOfThreads;
        extraBodiesToLastThread = bodies % numberOfThreads;
    }

    pthread_mutex_init(&threadsCounterMutex, NULL);
    if (bodies < numberOfThreads) {
        printf("Amount of bodies is less than number of threads, so actualt amout of threads is %d", numberOfThreads);
        nOfThreads = bodies;
    } else {
        nOfThreads = numberOfThreads;
    }
    pthread_arr = malloc(nOfThreads * sizeof(pthread_t));
}

int main(int argC, char *argV[])
{
    printf("%d", argC);
    if (argC != 3)
        printf("Usage : %s <file name containing system configuration data>\n", argV[0]);
    else
    {
        initiateSystem(argV[1], atoi(argV[2]));
        outputFile = fopen("main-output", "w");
        fprintf(outputFile, "Body   :     x              y           vx              vy");
        for (long long i = 0; i < nOfThreads; ++i){
            thread_args_t *args = (thread_args_t *)malloc(sizeof(thread_args_t));
            args->startIndex = i * bodiesPerThread;
            args->endIndex = args->startIndex + bodiesPerThread - 1;
            if (i == nOfThreads - 1) {
                args-> endIndex += extraBodiesToLastThread;
            }

            pthread_create(&pthread_arr[i], NULL, routine, (void *)args);
        }
        for (long long i = 0; i < nOfThreads; ++i){
            pthread_join(pthread_arr[i], NULL);
        }

        printf("End of program");
        fclose(outputFile);
        free(pthread_arr);
        free(masses);
        free(positions);
        free(velocities);
        free(accelerations);
        for (int i = 0; i < bodies; i++) {
            pthread_mutex_destroy(&bodiesMutexes[i]);
        }
        pthread_mutex_destroy(&threadsCounterMutex);
    }
}