# Matrix Multiplication with Processes and Threads

## Overview

This project demonstrates matrix multiplication using a variety of approaches: a naive approach, process-based parallelism, and thread-based parallelism. The goal is to explore performance optimization using processes and threads to speed up matrix multiplication. The program is implemented in C, using `pthread` for threading and `fork()` for process creation, with inter-process communication (IPC) via pipes.

## Key Features

1. **Naive Approach**: Traditional matrix multiplication using three nested loops, without parallelism.
2. **Process-Based Parallelism**: The matrix multiplication task is divided among multiple processes, each computing a specific range of rows, with communication between processes done using pipes.
3. **Thread-Based Parallelism**:
   - **Joinable Threads**: Threads are used to perform matrix multiplication in parallel. Each thread handles a specific range of rows.
   - **Detached Threads**: Similar to joinable threads, but the threads are detached immediately after creation, and the parent doesn't join them.
4. **Performance Measurement**: Execution times are measured for each approach and displayed in microseconds.

## Requirements

- **Operating System**: Linux or any Unix-like system (e.g., Ubuntu).
- **Libraries**:
  - `pthread.h` for thread management.
  - `sys/wait.h` for managing child processes.
  - `sys/time.h` for timing execution.
  - `stdio.h`, `stdlib.h`, `string.h` for standard input/output, memory management, and string operations.

## Compilation

To compile the program, use the following command:

gcc -o matrix_multiplication matrix_multiplication.c -pthread

## Execution
To run the program:

./matrix_multiplication

## Sample Output
The program will output execution times for the naive, process-based, and thread-based approaches:


Naive Approach:
Execution Time: <time_in_microseconds> microseconds

Process-Based Approach:
Execution Time: <time_in_microseconds> microseconds

Joinable Threads Approach:
Execution Time: <time_in_microseconds> microseconds

Detached Threads Approach:
Execution Time: <time_in_microseconds> microseconds
