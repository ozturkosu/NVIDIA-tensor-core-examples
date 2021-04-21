/*
 * Copyright 1993-2017 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

/* This example demonstrates how to use the CUBLAS library
 * by scaling an array of floating-point values on the device
 * and comparing the result to the same operation performed
 * on the host.
 */

/* Includes, system */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>  
#include <cstdlib> 

using namespace std;

/* Includes, cuda */
#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

/* Matrix size - Size increased for Tesnor Core use. */
#define N (1024)
/* Matrix size - This has been kept small to reduce host runtime */
//#define N (512)

/* Host implementation of a simple version of sgemm */
// static void simple_sgemm(int n, float alpha, const float *A, const float *B,
//                          float beta, float *C) {
//   int i;
//   int j;
//   int k;

//   for (i = 0; i < n; ++i) {
//     for (j = 0; j < n; ++j) {
//       half prod = 0.0f;

//       for (k = 0; k < n; ++k) {
//         prod = prod + (half)A[k * n + i] * (half)B[j * n + k];
//       }

//       C[j * n + i] = (float)((half)alpha * (half)prod + (half)beta * (half)C[j * n + i]);
//     }
//   }
// }

static void simple_sgemm(int n, float alpha, const __half *A, const __half *B,
                         float beta, __half *C) {
  int i;
  int j;
  int k;

  for (i = 0; i < n; ++i) {
    for (j = 0; j < n; ++j) {
      __half prod = 0.0f;

      for (k = 0; k < n; ++k) {
        prod = prod + A[k * n + i] * B[j * n + k];
      }

      C[j * n + i] = alpha * prod + beta * C[j * n + i];
    }
  }
}



/* Main */
int main(int argc, char **argv) {
  //cublasStatus_t status;

  cublasHandle_t handle;
  cublasStatus_t status = cublasCreate(&handle);
  status = cublasSetMathMode(handle, CUBLAS_TENSOR_OP_MATH);

  int size = atoi(argv[1]) ;

  cout << " Size N =" << size << endl;

  // float *h_A;
  // float *h_B;
  // float *h_C;

  // float *h_C_ref;

  // float *d_A = 0;
  // float *d_B = 0;
  // float *d_C = 0;

  __half * h_A;
  __half*  h_B;
  __half*  h_C;

  __half*  h_final ;

  __half* d_A = 0 ;
  __half* d_B = 0;
  __half *d_C = 0;

  __half *h_C_ref;

  float alpha = 1.0f;
  float beta = 0.0f;
  //int n2 = N * N;
  int n2 = size * size ;
  int i;

  float error_norm;
  float ref_norm;
  float diff;


  // __half error_norm ;
  // __half ref_norm ;
  // __half diff ;

  

  int dev = findCudaDevice(argc, (const char **)argv);

  if (dev == -1) {
    return EXIT_FAILURE;
  }

  /* Initialize CUBLAS */
  printf("simpleCUBLAS test running..\n");

  //status = cublasCreate(&handle);

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! CUBLAS initialization error\n");
    return EXIT_FAILURE;
  }

  // /* Allocate host memory for the matrices */
  // h_A = reinterpret_cast<float *>(malloc(n2 * sizeof(h_A[0])));

  // if (h_A == 0) {
  //   fprintf(stderr, "!!!! host memory allocation error (A)\n");
  //   return EXIT_FAILURE;
  // }

  // h_B = reinterpret_cast<float *>(malloc(n2 * sizeof(h_B[0])));

  // if (h_B == 0) {
  //   fprintf(stderr, "!!!! host memory allocation error (B)\n");
  //   return EXIT_FAILURE;
  // }

  // h_C = reinterpret_cast<float *>(malloc(n2 * sizeof(h_C[0])));

  // if (h_C == 0) {
  //   fprintf(stderr, "!!!! host memory allocation error (C)\n");
  //   return EXIT_FAILURE;
  // }

  h_A = new __half[n2] ;
  h_B = new __half[n2] ;
  h_C = new __half[n2] ;
  h_final = new __half[n2] ;
  //h_C_ref = new __half[n2];

  /* Fill the matrices with test data */
  for (i = 0; i < n2; i++) {
    // h_A[i] = rand() / static_cast<float>(RAND_MAX);
    // h_B[i] = rand() / static_cast<float>(RAND_MAX);
    // h_C[i] = rand() / static_cast<float>(RAND_MAX);
    h_A[i] = static_cast<__half>(static_cast<float>(std::rand() % 10));
    h_B[i] = static_cast<__half>(static_cast<float>(std::rand() % 10));
    h_C[i] = static_cast<__half>(static_cast<float>(std::rand() % 10));
    h_final[i] = static_cast<__half>(static_cast<float>(std::rand() % 10));

  }

  /* Allocate device memory for the matrices */
  if (cudaMalloc(reinterpret_cast<void **>(&d_A), n2 * sizeof(d_A[0])) !=
      cudaSuccess) {
    fprintf(stderr, "!!!! device memory allocation error (allocate A)\n");
    return EXIT_FAILURE;
  }

  if (cudaMalloc(reinterpret_cast<void **>(&d_B), n2 * sizeof(d_B[0])) !=
      cudaSuccess) {
    fprintf(stderr, "!!!! device memory allocation error (allocate B)\n");
    return EXIT_FAILURE;
  }

  if (cudaMalloc(reinterpret_cast<void **>(&d_C), n2 * sizeof(d_C[0])) !=
      cudaSuccess) {
    fprintf(stderr, "!!!! device memory allocation error (allocate C)\n");
    return EXIT_FAILURE;
  }

  /* Initialize the device matrices with the host matrices */
  status = cublasSetVector(n2, sizeof(h_A[0]), h_A, 1, d_A, 1);

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! device access error (write A)\n");
    return EXIT_FAILURE;
  }

  status = cublasSetVector(n2, sizeof(h_B[0]), h_B, 1, d_B, 1);

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! device access error (write B)\n");
    return EXIT_FAILURE;
  }

  status = cublasSetVector(n2, sizeof(h_C[0]), h_C, 1, d_C, 1);

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! device access error (write C)\n");
    return EXIT_FAILURE;
  }


  //Create Cuda Event for time
  float time_cublassGemmEx = 0;
  cudaEvent_t timeStart_gemmEx, timeEnd_gemmEx ;

  cudaEventCreate(&timeStart_gemmEx);
  cudaEventCreate(&timeEnd_gemmEx) ;
  cudaEventRecord(timeStart_gemmEx, 0) ;



  // /* Performs operation using cublas */
  // status = cublasGemmEx(handle, CUBLAS_OP_N, CUBLAS_OP_N, N, N, N, &alpha, 
  //                       d_A, CUDA_R_32F, N,             /* 32-bit float A */
  //                       d_B, CUDA_R_32F, N, &beta,      /* 32-bit float B */
  //                       d_C, CUDA_R_32F, N,             /* 32-bit float C */
  //                       CUDA_R_32F,                     /* 32-bit computation */
  //                       CUBLAS_GEMM_DEFAULT_TENSOR_OP); /* Enable automatic conversion to 16-bit */

  /* Performs operation using cublas */
  status = cublasGemmEx(handle, CUBLAS_OP_N, CUBLAS_OP_N, size, size, size, &alpha, 
                        d_A, CUDA_R_16F, size,             /* 16-bit float A */
                        d_B, CUDA_R_16F, size, &beta,      /* 16-bit float B */
                        d_C, CUDA_R_16F, size,             /* 16-bit float C */
                        CUDA_R_32F,                     /* 32-bit computation */
                        CUBLAS_GEMM_DEFAULT_TENSOR_OP); /* Enable automatic conversion to 16-bit */

  cudaEventRecord(timeEnd_gemmEx, 0) ;
  cudaEventSynchronize(timeEnd_gemmEx);
  cudaEventElapsedTime(&time_cublassGemmEx , timeStart_gemmEx, timeEnd_gemmEx );


  fprintf(stderr, "Time  for cuBLASS GemmEx function  %f milisecond \n", time_cublassGemmEx);

  cout << "Host Operation is starting" << endl ;
  /* Performs operation using plain C code */
  simple_sgemm(N, alpha, h_A, h_B, beta, h_C);
  h_C_ref = h_C;

  cout << "Host Operation is done" << endl ;

  // cout << "Host Operation is starting" << endl ;

  // simple_sgemm(size, alpha, h_A, h_B, beta, h_C_ref);

  //  cout << "Host Operation is done" << endl ;


  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! kernel execution error.\n");
    return EXIT_FAILURE;
  }

  /* Allocate host memory for reading back the result from device memory */
  //h_C = reinterpret_cast<float *>(malloc(n2 * sizeof(h_C[0])));
  //h_C = reinterpret_cast<__half *>(malloc(n2 * sizeof(h_C[0])));

  //h_C = new __half[n2] ;

  if (h_C == 0) {
    fprintf(stderr, "!!!! host memory allocation error (C)\n");
    return EXIT_FAILURE;
  }

  /* Read the result back */

  //cout << "Get Final Result from GPU " << endl ;
  
  //status = cublasGetVector(n2, sizeof(h_C[0]), d_C, 1, h_C, 1);

  cout << "Get Final Result from GPU " << endl ;

  status = cublasGetVector(n2, sizeof(h_final[0]), d_C, 1, h_final, 1);

  cout << "GPU to CPU transfer is done" << endl ;

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! device access error (read C)\n");
    return EXIT_FAILURE;
  }

  /* Check result against reference */
  error_norm = 0;
  ref_norm = 0;

  // for (i = 0; i < n2; ++i) {
  //   diff = h_C_ref[i] - h_C[i];
  //   error_norm += diff * diff;
  //   ref_norm += h_C_ref[i] * h_C_ref[i];
    
  // }

  cout << "Validation Test" << endl ;

  for (i = 0; i < n2; ++i) {
    diff = h_C_ref[i] - h_final[i];
    error_norm += diff * diff;
    ref_norm += h_C_ref[i] * h_C_ref[i];
    
  }



  error_norm = static_cast<float>(sqrt(static_cast<double>(error_norm)));
  ref_norm = static_cast<float>(sqrt(static_cast<double>(ref_norm)));

  //  error_norm = static_cast<__half>(sqrt(static_cast<double>(error_norm)));
  //  ref_norm = static_cast<__half>(sqrt(static_cast<double>(ref_norm)));

  if (fabs(ref_norm) < 1e-7) {
    fprintf(stderr, "!!!! reference norm is 0\n");
    return EXIT_FAILURE;
  }

  /* Memory clean up */
  free(h_A);
  free(h_B);
  free(h_C);
  free(h_C_ref);
  free(h_final) ;

  if (cudaFree(d_A) != cudaSuccess) {
    fprintf(stderr, "!!!! memory free error (A)\n");
    return EXIT_FAILURE;
  }

  if (cudaFree(d_B) != cudaSuccess) {
    fprintf(stderr, "!!!! memory free error (B)\n");
    return EXIT_FAILURE;
  }

  if (cudaFree(d_C) != cudaSuccess) {
    fprintf(stderr, "!!!! memory free error (C)\n");
    return EXIT_FAILURE;
  }

  /* Shutdown */
  status = cublasDestroy(handle);

  if (status != CUBLAS_STATUS_SUCCESS) {
    fprintf(stderr, "!!!! shutdown error (A)\n");
    return EXIT_FAILURE;
  }

  if (error_norm / ref_norm < 1e-2f) {
    printf("simpleCUBLAS test passed.\n");
    exit(EXIT_SUCCESS);
  } else {
    printf("simpleCUBLAS test failed.\n");
    exit(EXIT_FAILURE);
  }
}
