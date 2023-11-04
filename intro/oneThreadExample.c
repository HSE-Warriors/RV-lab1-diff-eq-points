#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DT 0.05

typedef struct {
    double x, y;
} vector;

int bodies, timeSteps;
double *masses, GravConstant;
vector *positions, *velocities, *accelerations;

vector addVectors(vector a, vector b) {
    vector c = {a.x + b.x, a.y + b.y};

    return c;
}

vector scaleVector(double b, vector a) {
    vector c = {b * a.x, b * a.y};

    return c;
}

vector subtractVectors(vector a, vector b) {
    vector c = {a.x - b.x, a.y - b.y};

    return c;
}

double mod(vector a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

void computeAccelerations() {
    int i, j;

    for (i = 0; i < bodies; i++) {
        accelerations[i].x = 0;
        accelerations[i].y = 0;
        for (j = 0; j < bodies; j++) {
            if (i != j) {
                //a = G * m_j / |r|^3 * (r_j - r_i)
                //where ∣r∣- the distance between the two bodies, so mod(sub(i,j)) - scalar distance between bodies i and j
                accelerations[i] = addVectors(accelerations[i], scaleVector(
                        GravConstant * masses[j] / pow(mod(subtractVectors(positions[i], positions[j])), 3),
                        subtractVectors(positions[j], positions[i])));
            }
        }
    }
}

void computePositions() {
    int i;

    for (i = 0; i < bodies; i++)
        //х=х0+v*t
        positions[i] = addVectors(positions[i], scaleVector(DT, velocities[i]));
}

void computeVelocities() {
    int i;

    for (i = 0; i < bodies; i++)
        //V = V0+a*t
        velocities[i] = addVectors(velocities[i], scaleVector(DT, accelerations[i]));
}

void resolveCollisions() {
    int i, j;

    for (i = 0; i < bodies - 1; i++)
        for (j = i + 1; j < bodies; j++) {
            if (positions[i].x == positions[j].x && positions[i].y == positions[j].y) {
                //If bodies collide, then we just V = V0 * -1, same speed to the opposite direction
                vector temp = velocities[i];
                velocities[i] = velocities[j];
                velocities[j] = temp;
            }
        }
}

void simulate() {
    computeAccelerations();
    computePositions();
    computeVelocities();
    resolveCollisions();
}

void initiateSystem(char *fileName) {
    int i;
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fscanf(fp, "%lf%d%d", &GravConstant, &bodies, &timeSteps); //TODO: strtod

    masses = (double *) malloc(bodies * sizeof(double));
    positions = (vector *) malloc(bodies * sizeof(vector));
    velocities = (vector *) malloc(bodies * sizeof(vector));
    accelerations = (vector *) malloc(bodies * sizeof(vector));

    for (i = 0; i < bodies; i++) {
        fscanf(fp, "%lf", &masses[i]); //TODO: strtod
        fscanf(fp, "%lf%lf", &positions[i].x, &positions[i].y); //TODO: strtod
        fscanf(fp, "%lf%lf", &velocities[i].x, &velocities[i].y); //TODO: strtod
    }

    fclose(fp);
}

int main(int argC, char *argV[]) {
    int i, j;
    printf("%s", &argC); //TODO: "Format specifies type 'char *' but the argument has type 'int *'"
    if (argC != 2)
        printf("Usage : %s <file name containing system configuration data>", argV[0]);
    else {
        initiateSystem(argV[1]);
        FILE *outputFile = fopen("intro/oneThreadExample-output", "w");
        if (outputFile == NULL) {
            perror("Error opening output file");
            return 1;
        }
        fprintf(outputFile, "Body   :     x              y           vx              vy");
        for (i = 0; i < timeSteps; i++) {
            fprintf(outputFile, "\nCycle %d\n", i + 1);
            simulate();
            for (j = 0; j < bodies; j++) {
                fprintf(outputFile, "Body %d : %lf\t%lf\t%lf\t%lf\n",
                        j + 1, positions[j].x, positions[j].y, velocities[j].x, velocities[j].y);
            }
        }
        fclose(outputFile);
    }
    return 0;
}