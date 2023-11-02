#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int compareFiles(FILE *fp1, FILE *fp2) {
    char line1[MAX_LINE_LENGTH];
    char line2[MAX_LINE_LENGTH];

    int lineNum = 0;
    int diffCount = 0;

    while (fgets(line1, MAX_LINE_LENGTH, fp1) != NULL &&
           fgets(line2, MAX_LINE_LENGTH, fp2) != NULL) {
        lineNum++;
        if (strcmp(line1, line2) != 0) {
            diffCount++;
            printf("Difference found at line %d:\n", lineNum);
            printf("< %s> %s\n", line1, line2);
        }
    }

    if (!feof(fp1) || !feof(fp2)) {
        puts("The files have different number of lines.");
        diffCount++;
    }

    return diffCount;
}

int main(int argc, char *argv[]) {

    // Open the two files
    FILE *fp1 = fopen("intro/output-sample", "r");
    FILE *fp2 = fopen("intro/oneThreadExample-output", "r");

    if (fp1 == NULL || fp2 == NULL) {
        perror("Error opening files");
        return 1;
    }

    int result = compareFiles(fp1, fp2);

    fclose(fp1);
    fclose(fp2);

    if (result == 0) {
        printf("Files are identical.\n");
    } else {
        printf("Files have %d different lines.\n", result);
    }

    return 0;
}
