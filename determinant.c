#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

// Функция для получения минорной матрицы
void getMinorMatrix(double **matrix, double **minor, int row, int col, int order) {
    for (int i = 0, minorRow = 0; i < order; i++) {
        if (i == row) continue;
        for (int j = 0, minorCol = 0; j < order; j++) {
            if (j == col) continue;
            minor[minorRow][minorCol++] = matrix[i][j];
        }
        minorRow++;
    }
}

// Функция для вычисления определителя матрицы
double determinant(double **matrix, int order) {
    if (order == 1) return matrix[0][0];
    if (order == 2) return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];

    double det = 0;
    int sign = 1;
    double **minor = (double **)malloc((order - 1) * sizeof(double *));
    
    for (int i = 0; i < order - 1; i++) {
        minor[i] = (double *)malloc((order - 1) * sizeof(double));
    }

    for (int i = 0; i < order; i++) {
        getMinorMatrix(matrix, minor, 0, i, order);
        det += sign * matrix[0][i] * determinant(minor, order - 1);
        sign = -sign;
    }

    for (int i = 0; i < order - 1; i++) {
        free(minor[i]);
    }
    free(minor);

    return det;
}

// Функция для чтения матрицы из многострочного файла и определения её размера
double** readMatrixFromFile(const char *filename, int *order) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int rows = 0;
    int cols = 0;

    while (fgets(line, sizeof(line), file)) {
        rows++;
        char *ptr = strtok(line, " ");
        int currentCols = 0;

        while (ptr != NULL) {
            currentCols++;
            ptr = strtok(NULL, " ");
        }

        if (cols == 0) {
            cols = currentCols;
        } else if (currentCols != cols) {
            fprintf(stderr, "Error: Inconsistent number of columns in rows.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    if (rows != cols) {
        fclose(file);
        fprintf(stderr, "Error: Matrix is not square (%d x %d).\n", rows, cols);
        exit(EXIT_FAILURE);
    }

    *order = rows;

    rewind(file);
    double **matrix = (double **)malloc(*order * sizeof(double *));
    
    for (int i = 0; i < *order; i++) {
        matrix[i] = (double *)malloc(*order * sizeof(double));
        fgets(line, sizeof(line), file);
        char *ptr = strtok(line, " ");
        for (int j = 0; j < *order; j++) {
            matrix[i][j] = atof(ptr);
            ptr = strtok(NULL, " ");
        }
    }

    fclose(file);
    return matrix;
}

// Функция для чтения матрицы из однострочного файла
double** readMatrixFromSingleLineFile(const char *filename, int *order) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    if (!fgets(line, sizeof(line), file)) {
        perror("Error reading file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Подсчет числа элементов в строке
    int elements = 0;
    char *lineCopy = strdup(line);
    char *ptr = strtok(lineCopy, " ");
    while (ptr != NULL) {
        elements++;
        ptr = strtok(NULL, " ");
    }

    *order = (int)sqrt(elements);
    if (*order * *order != elements) {
        fprintf(stderr, "Error: The number of elements is not a perfect square.\n");
        free(lineCopy);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Создание матрицы
    double **matrix = (double **)malloc(*order * sizeof(double *));
    for (int i = 0; i < *order; i++) {
        matrix[i] = (double *)malloc(*order * sizeof(double));
    }

    // Снова чтение элементов и их распределение по матрице
    int elementIndex = 0;
    ptr = strtok(line, " ");
    while (ptr != NULL) {
        int row = elementIndex / *order;
        int col = elementIndex % *order;
        matrix[row][col] = atof(ptr);
        ptr = strtok(NULL, " ");
        elementIndex++;
    }

    free(lineCopy);
    fclose(file);
    return matrix;
}

// Функция для вывода справки
void printHelp(const char *programName) {
    printf("Usage: %s [options] <filename>\n", programName);
    printf("Options:\n");
    printf("  -h           Show this help message\n");
    printf("  -s           Read matrix from a single-line file\n");
    printf("  -m           Read matrix from a multi-line file\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [options] <filename>\n", argv[0]);
        return 1;
    }

    int singleLine = 0;
    int multiLine = 0;
    const char *filename = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "hsm")) != -1) {
        switch (opt) {
            case 'h':
                printHelp(argv[0]);
                return 0;
            case 's':
                singleLine = 1;
                break;
            case 'm':
                multiLine = 1;
                break;
            default:
                printHelp(argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    } else {
        fprintf(stderr, "Error: Filename not provided.\n");
        return 1;
    }

    if (singleLine && multiLine) {
        fprintf(stderr, "Error: Please specify either -s or -m option, not both.\n");
        return 1;
    }

    if (!singleLine && !multiLine) {
        fprintf(stderr, "Error: Please specify either -s or -m option.\n");
        return 1;
    }

    int order;
    double **matrix;

    if (singleLine) {
        matrix = readMatrixFromSingleLineFile(filename, &order);
    } else {
        matrix = readMatrixFromFile(filename, &order);
    }

    printf("Read matrix of size %dx%d:\n", order, order);
    for (int i = 0; i < order; i++) {
        for (int j = 0; j < order; j++) {
            if (fmod(matrix[i][j], 1.0) == 0.0) {
                printf("%d\t", (int)matrix[i][j]);
            } else {
                printf("%.2f\t", matrix[i][j]);
            }
        }
        printf("\n");
    }

    double det = determinant(matrix, order);
    printf("Determinant of the matrix: %.2f\n", det);

    for (int i = 0; i < order; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}
