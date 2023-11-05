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
int nOfFinishedThreads = 0;

FILE *accOutputFile;
FILE *posOutputFile;
FILE *velOutputFile;

typedef struct {
    int startIndex;
    int endIndex;
} thread_args_t;

int nofThreads;
int extraBodiesToLastThread = 0;
pthread_barrier_t barrier;
pthread_t* pthread_arr;
pthread_mutex_t *bodiesMutexes;
pthread_mutex_t threadsCounterMutex;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void log_vectors(const char *name, vector *array, size_t length, FILE *outputFile) {
    if (outputFile == NULL) {
        printf("Output file is empty\n");
    }
    for (size_t i = 0; i < length; i++) {
        int written = fprintf(outputFile, " (%lf, %lf)", array[i].x, array[i].y);
        if (written < 0) {
            // Handle error
            perror("fprintf failed");
            exit(-2);
        }           
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

    pthread_mutex_lock(&bodiesMutexes[i]);
    firstMass = masses[i];
    firstPosition = positions[i];
    pthread_mutex_unlock(&bodiesMutexes[i]);

    pthread_mutex_lock(&bodiesMutexes[j]);
    secondMass = masses[j];
    secondPosition = positions[j];
    pthread_mutex_unlock(&bodiesMutexes[j]);

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
        pthread_mutex_lock(&threadsCounterMutex);
        nOfFinishedThreads++;
        if(nOfFinishedThreads == nofThreads) {
            nOfFinishedThreads = 0;
            pthread_cond_broadcast(&cond_var);
        } else {
            while(pthread_cond_wait(&cond_var, &threadsCounterMutex) != 0);
        }
        pthread_mutex_unlock(&threadsCounterMutex);

        //Positions
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countPositions(i);
        }
        pthread_mutex_lock(&threadsCounterMutex);
        nOfFinishedThreads++;
        if(nOfFinishedThreads == nofThreads) {
            nOfFinishedThreads = 0;
            pthread_cond_broadcast(&cond_var);
        } else {
            while(pthread_cond_wait(&cond_var, &threadsCounterMutex) != 0);
        }
        pthread_mutex_unlock(&threadsCounterMutex);

        
        //Velocities and collisitons
        for (long i = args->startIndex; i <= args->endIndex; i++) {
            countVelocities(i);
            resolveCollisions(i);
        }
        pthread_mutex_lock(&threadsCounterMutex);
        nOfFinishedThreads++;
        if(nOfFinishedThreads == nofThreads) {
            nOfFinishedThreads = 0;
            fprintf(outputFile, "\nCycle %d\n", t + 1);
            for (int j = 0; j < bodies; j++) {
                fprintf(outputFile, "Body %d : %lf\t%lf\t%lf\t%lf\n",
                    j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
            }
            for (size_t i = 0; i < bodies; ++i) {
                pthread_mutex_lock(&bodiesMutexes[i]);
                accelerations[i].x = 0.0;
                accelerations[i].y = 0.0;
                pthread_mutex_unlock(&bodiesMutexes[i]);
            }
            pthread_cond_broadcast(&cond_var);
        } else {
            while(pthread_cond_wait(&cond_var, &threadsCounterMutex) != 0);
        }
        pthread_mutex_unlock(&threadsCounterMutex);
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
    if (bodies < numberOfThreads) {
        bodiesPerThread = 1;
    } else {
        bodiesPerThread = bodies / numberOfThreads;
        extraBodiesToLastThread = bodies % numberOfThreads;
    }

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

    pthread_mutex_init(&threadsCounterMutex, NULL);
    if (bodies < numberOfThreads) {
        printf("Inputed amount of bodies is less than number of threads, so actualt amout of threads is %d", numberOfThreads);
        nofThreads = bodies;
    } else {
        nofThreads = numberOfThreads;
    }

    pthread_arr = malloc(nofThreads * sizeof(pthread_t));
    int rc = pthread_barrier_init(&barrier, NULL, nofThreads);
    if (rc != 0) {
        perror("Error with barrier init");
        exit(1);
    } 

    accOutputFile = fopen("acccelerationsOutput", "w");
    if (accOutputFile == NULL) {
        printf("Acc error");
        perror("Can`t open file");
    }
    posOutputFile = fopen("positionsOutput", "w");
    velOutputFile = fopen("velocitiesOutput", "w");
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
        for (long long i = 0; i < nofThreads; ++i){
            thread_args_t *args = (thread_args_t *)malloc(sizeof(thread_args_t));
            args->startIndex = i * bodiesPerThread;
            args->endIndex = args->startIndex + bodiesPerThread - 1;
            if (i == nofThreads - 1) {
                args-> endIndex += extraBodiesToLastThread;
            }
            pthread_create(&pthread_arr[i], NULL, routine, (void *)args);
        }

        for (long long i = 0; i < nofThreads; ++i){
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
        pthread_mutex_destroy(&threadsCounterMutex);
        fclose(accOutputFile);
        fclose(posOutputFile);
        fclose(velOutputFile);
    }
}