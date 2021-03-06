/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 * 
 * @author Li Li
 * @e-mail Lil147@pitt.edu
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_others(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32)
    {
        transpose_32(M, N, A, B);
    }
    else if (M == 64 && N == 64)
    {
        transpose_64(M, N, A, B);
    }
    else
    {
        transpose_others(M, N, A, B);
    }
}

char transpose_32_desc[] = "Transpose 32";
void transpose_32(int M, int N, int A[N][M], int B[M][N])
{
    int j, k, kk, jj;
    int bsize = 8;

    for (kk = 0; kk < N; kk += bsize)
    {
        for (jj = 0; jj < M; jj += bsize)
        {
            for (k = kk; k < kk + bsize; k++)
            {
                for (j = jj; j < jj + bsize; j++)
                {
                    if ((k - kk) != (j - jj))
                    {
                        int temp = A[k][j];
                        B[j][k] = temp;
                    }
                }

                // handle with diagonal elements to avoid evicting
                B[jj + k - kk][k] = A[k][jj + k - kk];
            }
        }
    }
}

char transpose_submit_desc2[] = "Transpose submission 2";
void transpose_submit2(int M, int N, int A[N][M], int B[M][N])
{
    int kk, jj;
    int bsize = 2;

    for (kk = 0; kk < N; kk += bsize)
    {
        for (jj = 0; jj < M; jj += bsize)
        {
            if (jj != kk)
            {
                B[jj + 0][kk + 0] = A[kk + 0][jj + 0];
            }
            if (jj + 1 != kk)
            {
                B[jj + 1][kk + 0] = A[kk + 0][jj + 1];
            }
            if (jj != kk + 1)
            {
                B[jj + 0][kk + 1] = A[kk + 1][jj + 0];
            }
            if (jj + 1 != kk + 1)
            {
                B[jj + 1][kk + 1] = A[kk + 1][jj + 1];
            }
            B[jj + 0][kk + 0] = A[kk + 0][jj + 0];
            B[jj + 1][kk + 1] = A[kk + 1][jj + 1];
        }
    }
}

char transpose_desc64[] = "Transpose 64";
void transpose_64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    int x, y, x1, x2, x3, x4, x5, x6, x7, x8; // save in register
    for (i = 0; i < N; i += 8)
    {
        for (j = 0; j < M; j += 8)
        {
            // each line in subarray has 8 int elements (32 bytes)
            // Step1: write first 4 lines in array B
            for (x = i; x < i + 4; x++)
            {
                x1 = A[x][j + 0];
                x2 = A[x][j + 1];
                x3 = A[x][j + 2];
                x4 = A[x][j + 3];
                x5 = A[x][j + 4];
                x6 = A[x][j + 5];
                x7 = A[x][j + 6];
                x8 = A[x][j + 7];

                // transpose to correct place
                B[j + 0][x] = x1;
                B[j + 1][x] = x2;
                B[j + 2][x] = x3;
                B[j + 3][x] = x4;

                // not to visit the same line
                // save following elements in other place temporarily
                B[j + 0][x + 4] = x5;
                B[j + 1][x + 4] = x6;
                B[j + 2][x + 4] = x7;
                B[j + 3][x + 4] = x8;
            }

            // Step2: write last 4 column of first 4 lines
            //          and first 4 column of last 4 lines in array B
            for (y = j; y < j + 4; y++)
            {
                x1 = A[i + 4][y];
                x2 = A[i + 5][y];
                x3 = A[i + 6][y];
                x4 = A[i + 7][y];

                // retrive temporarily saved elements in Step1 to registers
                x5 = B[y][i + 4];
                x6 = B[y][i + 5];
                x7 = B[y][i + 6];
                x8 = B[y][i + 7];

                // transpose to correct place
                // cache friendly: load B[y][i + l] in cache directly
                B[y][i + 4] = x1;
                B[y][i + 5] = x2;
                B[y][i + 6] = x3;
                B[y][i + 7] = x4;

                // transpose retrived elements to correct place
                B[y + 4][i + 0] = x5;
                B[y + 4][i + 1] = x6;
                B[y + 4][i + 2] = x7;
                B[y + 4][i + 3] = x8;
            }

            // Step3: write last 4 lines and last 4 column in array B
            for (x = i + 4; x < i + 8; x++)
            {
                x1 = A[x][j + 4];
                x2 = A[x][j + 5];
                x3 = A[x][j + 6];
                x4 = A[x][j + 7];

                // transpose to correct place
                B[j + 4][x] = x1;
                B[j + 5][x] = x2;
                B[j + 6][x] = x3;
                B[j + 7][x] = x4;
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char transpose_others_desc[] = "Simple blocked transpose";
void transpose_others(int M, int N, int A[N][M], int B[M][N])
{
    int jj, i, j;
    int blk_bound;
    int bsize = 8;
    int EM = bsize * (M / bsize); // amount that fits evenly into blocks

    for (jj = 0; jj < EM; jj += bsize)
    {
        blk_bound = jj + bsize;
        for (i = 0; i < N; i++)
        {
            for (j = jj; j < blk_bound; j++)
            {
                B[j][i] = A[i][j];
            }
        }
    }

    // handle with the other elements outside blocks
    for (i = 0; i < N; i++)
    {
        for (j = jj; j < M; j++)
        {
            B[j][i] = A[i][j];
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
    registerTransFunction(transpose_submit2, transpose_submit_desc2);
    registerTransFunction(transpose_64, transpose_desc64);
    registerTransFunction(transpose_32, transpose_32_desc);
    registerTransFunction(transpose_others, transpose_others_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}