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
pthread_mutex_t *bodiesMutexes;

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

double distanceBetweenVectors(vector a, vector b, int a_index, int b_index) {
    //TODO
    //If (distance is counted) {
    // return it    
    //} else {
        return mod(subtractVectors(a, b));
    //}
}

void countAcceleration(int i, int j) {
    int firstMass = masses[i];
    int secondMass = masses[j];
    vector firstPosition = positions[i];
    vector secondPosition = positions[j];

    double distance = distanceBetweenVectors(firstPosition, secondPosition, i, j);
    double force = GravConstant * firstMass * secondMass / (distance * distance);
    //Count force`s vector to correctly count accelerations
    vector forceVector = scaleVector(force, normalize(subtractVectors(secondPosition, firstPosition)));

    vector firstNewAcceleration = scaleVector(1.0 / firstMass, forceVector);
    vector secondNewAcceleration = scaleVector(-1.0 / secondMass, forceVector);
    //Save accelerations
    pthread_mutex_lock(&bodiesMutexes[i]);
    accelerations[i] = addVectors(accelerations[i], firstNewAcceleration);
    pthread_mutex_unlock(&bodiesMutexes[i]);
    //printf("second body mutex lock: %d\n", j);
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

void* routine(void *threadArgs){
    thread_args_t *args = (thread_args_t *)threadArgs;

    printf("Thread working on index range [%d, %d]\n", args->startIndex, args->endIndex);
    for (long t = 0; t < timeSteps; t++) {
        //Count distances to other threads
        for (long i = 0; i < args->startIndex; i++) {
            //count distances
        }
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            for (long j = i+1; j <= bodies - 1; j++) {
                countAcceleration(i, j);
            }
        }
        pthread_barrier_wait(&barrier);
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countPositions(i);
        }
        pthread_barrier_wait(&barrier);
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countVelocities(i);
            resolveCollisions(i);
        }
        int res = pthread_barrier_wait(&barrier);
        //if equal, then last thread finished and now we will fall barrier, like callback onBarrierFell
        if(res == PTHREAD_BARRIER_SERIAL_THREAD) {
            fprintf(outputFile, "\nCycle %d\n", t + 1);
            for (int j = 0; j < bodies; j++) {
                fprintf(outputFile, "Body %d : %lf\t%lf\t%lf\t%lf\n",
                    j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
            }
        }

    }
    free(args);
}

void initiateSystem(char *fileName, int numberOfThreads)
{
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
    bodiesMutexes = (pthread_mutex_t *)malloc(bodies * sizeof(pthread_mutex_t));

    for (int i = 0; i < bodies; i++)
    {
        fscanf(fp, "%lf", &masses[i]);
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y);
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y);
        pthread_mutex_init(&bodiesMutexes[i], NULL);
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

int main(int argC, char *argV[])
{
    printf("%s", &argC);
    if (argC != 3)
        printf("Usage : %s <file name containing system configuration data>", argV[0]);
    else
    {
        int numberOfThreads = atoi(argV[2]);
        initiateSystem(argV[1], numberOfThreads);
        outputFile = fopen("main-output", "w");
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
        for (int i = 0; i < bodies; i++) {
            pthread_mutex_destroy(&bodiesMutexes[i]);
        }
    }
}