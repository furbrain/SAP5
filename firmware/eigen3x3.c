// ----------------------------------------------------------------------------
// Numerical diagonalization of 3x3 matrcies
// Copyright (C) 2006  Joachim Kopp, modified to use GSL formats 2018 Phil Underwood
// ----------------------------------------------------------------------------
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <math.h>
#include "eigen3x3.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>

// Macros
#define SQR(x)      ((x)*(x))                        // x^2 


// ----------------------------------------------------------------------------
int eigen3x3(gsl_matrix *A, gsl_matrix *Q, gsl_vector *w)
// ----------------------------------------------------------------------------
// Calculates the eigenvalues and normalized eigenvectors of a symmetric 3x3
// matrix A using the Jacobi algorithm.
// The upper triangular part of A is destroyed during the calculation,
// the diagonal elements are read but not destroyed, and the lower
// triangular elements are not referenced at all.
// ----------------------------------------------------------------------------
// Parameters:
//   A: The symmetric input matrix
//   Q: Storage buffer for eigenvectors
//   w: Storage buffer for eigenvalues
// ----------------------------------------------------------------------------
// Return value:
//   0: Success
//  -1: Error (no convergence)
// ----------------------------------------------------------------------------
{
  if ((A->size1 != 3) || (A->size2 != 3)) {
    GSL_ERROR("A must be a 3x3 matrix", GSL_EBADLEN);
  }
  if ((Q->size1 != 3) || (Q->size2 != 3)) {
    GSL_ERROR("Q must be a 3x3 matrix", GSL_EBADLEN);
  }
  if (w->size != 3)  {
    GSL_ERROR("w must be a 3 vector", GSL_EBADLEN);
  }
   
  const int n = 3;
  double sd, so;                  // Sums of diagonal resp. off-diagonal elements
  double s, c, t;                 // sin(phi), cos(phi), tan(phi) and temporary storage
  double g, h, z, theta;          // More temporary storage
  double thresh;
  double Apq, wp, wq;             // temporary variables to reduce number of matrix/vector_gets
  int i, p, q, r, nIter;                 // loop variables
  
  // Initialize Q to the identitity matrix
  gsl_matrix_set_identity(Q);
  // Initialize w to diag(A)
  for (i=0; i < n; i++)
    gsl_vector_set(w, i, gsl_matrix_get(A, i, i));


  // Calculate SQR(tr(A))  
  sd = gsl_blas_dasum(w);
  sd = SQR(sd);
 
  // Main iteration loop
  for (nIter=0; nIter < 50; nIter++)
  {
    // Test for convergence 
    so = 0.0;
    for (p=0; p < n; p++)
      for (q=p+1; q < n; q++)
        so += fabs(gsl_matrix_get(A, p, q));
    if (so == 0.0)
      return 0;

    if (nIter < 4)
      thresh = 0.2 * so / SQR(n);
    else
      thresh = 0.0;

    // Do sweep
    for (p=0; p < n; p++)
      for (q=p+1; q < n; q++)
      {
        Apq = gsl_matrix_get(A, p, q);
        wp = gsl_vector_get(w, p);
        wq = gsl_vector_get(w, q);
        g = 100.0 * fabs(Apq);
        if (nIter > 4  &&  fabs(wp) + g == fabs(wp)
                       &&  fabs(wq) + g == fabs(wq))
        {
          gsl_matrix_set(A, p, q, 0.0);
          Apq = 0.0;
        }
        else if (fabs(Apq) > thresh)
        {
          // Calculate Jacobi transformation
          h = wq-wp;
          if (fabs(h) + g == fabs(h))
          {
            t = Apq / h;
          }
          else
          {
            theta = 0.5 * h / Apq;
            if (theta < 0.0)
              t = -1.0 / (sqrt(1.0 + SQR(theta)) - theta);
            else
              t = 1.0 / (sqrt(1.0 + SQR(theta)) + theta);
          }
          c = 1.0/sqrt(1.0 + SQR(t));
          s = t * c;
          z = t * Apq;

          // Apply Jacobi transformation
          gsl_matrix_set(A, p, q, 0.0);
          Apq = 0.0;
          gsl_vector_set(w, p, wp - z);
          gsl_vector_set(w, q, wq + z);
          for (r=0; r < p; r++)
          {
            t = gsl_matrix_get(A, r, p);
            gsl_matrix_set(A, r, p, c*t - s * gsl_matrix_get(A, r, q));
            gsl_matrix_set(A, r, q, s*t + c * gsl_matrix_get(A, r, q));
            //A[r][p] = c*t - s*A[r][q];
            //A[r][q] = s*t + c*A[r][q];
          }
          for (r=p+1; r < q; r++)
          {
            t = gsl_matrix_get(A, p, r);
            gsl_matrix_set(A, p, r, c*t - s * gsl_matrix_get(A, r, q));
            gsl_matrix_set(A, r, q, s*t + c * gsl_matrix_get(A, r, q));
            //t = A[p][r];
            //A[p][r] = c*t - s*A[r][q];
            //A[r][q] = s*t + c*A[r][q];
          }
          for (r=q+1; r < n; r++)
          {
            t = gsl_matrix_get(A, p, r);
            gsl_matrix_set(A, p, r, c*t - s * gsl_matrix_get(A, q, r));
            gsl_matrix_set(A, q, r, s*t + c * gsl_matrix_get(A, q, r));
            //t = A[p][r];
            //A[p][r] = c*t - s*A[q][r];
            //A[q][r] = s*t + c*A[q][r];
          }

          // Update eigenvectors
          for (r=0; r < n; r++)
          {
            t = gsl_matrix_get(Q, r, p);
            gsl_matrix_set(Q, r, p, c*t - s * gsl_matrix_get(Q, r, q));
            gsl_matrix_set(Q, r, q, s*t + c * gsl_matrix_get(Q, r, q));
            //t = Q[r][p];
            //Q[r][p] = c*t - s*Q[r][q];
            //Q[r][q] = s*t + c*Q[r][q];
          }
        }
      }
  }

  GSL_ERROR("eigensystem failed to converge", GSL_ENOPROG);
}

