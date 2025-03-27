#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h> 

#define SIZE 100
#define MAX_THREADS 4
#define MAX_PROCESSES 4

int Matrix1[SIZE][SIZE];
int Matrix2[SIZE][SIZE];
int result[SIZE][SIZE];

// Thread argument structure
struct ThreadArgs
{
    int startRow;
    int endRow;
};

// Function to display a matrix
void displayMatrix(int matrix[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

// Function to perform matrix multiplication for a specific range of rows
void multiplyRows(int startRow, int endRow, int Matrix1[SIZE][SIZE], int Matrix2[SIZE][SIZE], int result[SIZE][SIZE])
{
    for (int i = startRow; i < endRow; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; ++k)
            {
                result[i][j] += Matrix1[i][k] * Matrix2[k][j];
            }
        }
    }
}

// Thread routine for joinable threads
void *multiplyRowsJoinable(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    multiplyRows(threadArgs->startRow, threadArgs->endRow, Matrix1, Matrix2, result);
    pthread_exit(NULL);
}

int main()
{
    // Initialize
    int StudentID[] = {1, 2, 1, 0, 0, 8, 4};
    int num[] = {2, 4, 2, 3, 7, 9, 8, 2, 5, 2};

    // Fill the first matrix with student id
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            Matrix1[i][j] = StudentID[(i * SIZE + j) % 7];
        }
    }

    // Fill the second matrix with (student id * birth year)
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            Matrix2[i][j] = num[(i * SIZE + j) % 10];
        }
    }

    // Naive approach (no child processes or threads)
    struct timeval start, end;

    gettimeofday(&start, NULL);

    // matrix multiplication
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; ++k)
            {
                result[i][j] += Matrix1[i][k] * Matrix2[k][j];
            }
        }
    }

    gettimeofday(&end, NULL);
    // Calculate execution time for Naive approach
    long long naiveExecutionTime = ((long long)end.tv_sec - start.tv_sec) * 1000000LL +
                                   (end.tv_usec - start.tv_usec);

    printf("Naive Approach:\n");
    printf("Execution Time: %lld microseconds\n", naiveExecutionTime);

    // Dynamic allocation for matrices
    int (*DynamicMatrix1)[SIZE] = malloc(sizeof(int[SIZE][SIZE]));
    int (*DynamicMatrix2)[SIZE] = malloc(sizeof(int[SIZE][SIZE]));
    int (*DynamicResult)[SIZE] = malloc(sizeof(int[SIZE][SIZE]));

    if (!DynamicMatrix1 || !DynamicMatrix2 || !DynamicResult)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Fill the dynamic matrices with student id and (student id * birth year)
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            DynamicMatrix1[i][j] = StudentID[(i * SIZE + j) % 7];
            DynamicMatrix2[i][j] = num[(i * SIZE + j) % 10];
        }
    }

    // Number of child processes and threads to create
    int numProcesses = 8;
    int numThreads = 8;

    // Calculate rows per child and thread
    int rowsPerChild = SIZE / numProcesses;
    int rowsPerThread = SIZE / numThreads;

    // Pipe array for IPC
    int pipes[numProcesses][2];
    // Create a buffer to hold the result for each child
    int (*childResults)[SIZE] = malloc(sizeof(int[SIZE][SIZE]));

    if (!childResults)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Get the start time for processes
    gettimeofday(&start, NULL);

    // Create child processes and pipes for process-based approach
    for (int i = 0; i < numProcesses; ++i)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }

        pid_t childPid = fork();

        if (childPid == -1)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) // Child process
        {
            close(pipes[i][0]); // Close unused read end of the pipe
            multiplyRows(i * rowsPerChild, (i + 1) * rowsPerChild, DynamicMatrix1, DynamicMatrix2, childResults);
            write(pipes[i][1], childResults, sizeof(int[SIZE][SIZE])); // Write the result to the pipe
            close(pipes[i][1]);                                       // Close the write end of the pipe

            exit(EXIT_SUCCESS);
        }
        else // Parent process
        {
            close(pipes[i][1]); // Close unused write end of the pipe
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < numProcesses; ++i)
    {
        wait(NULL);
    }

    // Read results from pipes and accumulate them in the result matrix for process-based approach
    for (int i = 0; i < numProcesses; ++i)
    {
        close(pipes[i][1]); // Close write end of the pipe in the parent

        // Read the result from the pipe and accumulate it in the result matrix
        read(pipes[i][0], childResults, sizeof(int[SIZE][SIZE]));
        close(pipes[i][0]); // Close read end of the pipe

        // Copy the child result into the main result matrix
        for (int row = 0; row < rowsPerChild; ++row)
        {
            memcpy(DynamicResult[i * rowsPerChild + row], childResults[row], sizeof(int[SIZE]));
        }
    }

    // Get the end time for joinable threads
    gettimeofday(&end, NULL);

    // Calculate execution time for process-based approach
    long long processExecutionTime = ((long long)end.tv_sec - start.tv_sec) * 1000000LL +
                                     (end.tv_usec - start.tv_usec);

    printf("Process-Based Approach:\n");
    printf("Execution Time: %lld microseconds\n", processExecutionTime);

    // Create thread array and thread argument array for threaded approaches
    pthread_t threads[MAX_THREADS];
    struct ThreadArgs threadArgs[MAX_THREADS];

    // Get the start time for joinable threads
    gettimeofday(&start, NULL);

    // Create and run threads with joinable for threaded approach
    for (int i = 0; i < numThreads; ++i)
    {
        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i + 1) * rowsPerThread;

        if (pthread_create(&threads[i], NULL, multiplyRowsJoinable, (void *)&threadArgs[i]) != 0)
        {
            fprintf(stderr, "Error: Failed to create thread.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Join threads for threaded approach
    for (int i = 0; i < numThreads; ++i)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            fprintf(stderr, "Error: Failed to join thread.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Get the end time for joinable threads
    gettimeofday(&end, NULL);

    // Calculate execution time for joinable threads approach
    long long joinableThreadsExecutionTime = ((long long)end.tv_sec - start.tv_sec) * 1000000LL +
                                             (end.tv_usec - start.tv_usec);

    printf("Joinable Threads Approach:\n");
    printf("Execution Time: %lld microseconds\n", joinableThreadsExecutionTime);

    pthread_t detachedThreads[MAX_THREADS];

    // Get the start time for detached threads
    gettimeofday(&start, NULL);

    for (int i = 0; i < numThreads; ++i)
    {
        threadArgs[i].startRow = i * rowsPerThread;
        threadArgs[i].endRow = (i + 1) * rowsPerThread;

        if (pthread_create(&threads[i], NULL, multiplyRowsJoinable, (void *)&threadArgs[i]) != 0)
        {
            fprintf(stderr, "Error: Failed to create thread.\n");
            exit(EXIT_FAILURE);
        }
    }



    // Get the end time for detached threads
    gettimeofday(&end, NULL);

    // Calculate execution time for detached threads approach
    long long detachedThreadsExecutionTime = ((long long)end.tv_sec - start.tv_sec) * 1000000LL +
                                             (end.tv_usec - start.tv_usec);

    printf("Detached Threads Approach:\n");
    printf("Execution Time: %lld microseconds\n", detachedThreadsExecutionTime);

    // Free allocated memory for dynamic matrices
    free(DynamicMatrix1);
    free(DynamicMatrix2);
    free(DynamicResult);
    free(childResults);

    return 0;
}