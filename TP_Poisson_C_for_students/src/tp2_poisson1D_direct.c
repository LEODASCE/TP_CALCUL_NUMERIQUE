/******************************************/
/* tp2_poisson1D_direct.c                 */
/* This file contains the main function   */
/* to solve the Poisson 1D problem        */
/******************************************/
#include "lib_poisson1D.h"
int main(int argc,char *argv[])
/* ** argc: Number of arguments */
/* ** argv: Values of arguments */
{
  int ierr;
  int jj;
  int nbpoints, la;
  int ku, kl, kv, lab;
  int *ipiv;
  int info;
  int NRHS;
  double T0, T1;
  double *RHS, *EX_SOL, *X, *X_test_dgmv, *Y_test_dgmv;
  double *AB;
    
  double temp, relres;
  //
  double alpha = 1;
  double beta  = 0;
  int   incx   = 1;
  int   incy   = 1;
  
  

  NRHS=1;
  nbpoints=102;
  la=nbpoints-2;
  T0=-5.0;
  T1=5.0;

  printf("--------- Poisson 1D ---------\n\n");
  RHS=(double *) malloc(sizeof(double)*la);
  EX_SOL=(double *) malloc(sizeof(double)*la);
  X=(double *) malloc(sizeof(double)*la);
  
  //
  X_test_dgmv =(double *) malloc(sizeof (double) * la);
  Y_test_dgmv =(double*) malloc(sizeof (double) * la);                                                          

  set_grid_points_1D(X, &la);
  set_dense_RHS_DBC_1D(RHS,&la,&T0,&T1);
  set_analytical_solution_DBC_1D(EX_SOL, X, &la, &T0, &T1);
  
  //
  for(int i= 0; i<la; i++)
  {
      Y_test_dgmv[i] = i;
  }
  
  //
  for(int i = 0; i<la ; i++)
  {
      X_test_dgmv[i] = EX_SOL[i];
  }
  
  write_vec(RHS, &la, "RHS.dat");
  write_vec(EX_SOL, &la, "EX_SOL.dat");
  write_vec(X, &la, "X_grid.dat");

  kv=1;
  ku=1;
  kl=1;
  lab=kv+kl+ku+1;

  AB = (double *) malloc(sizeof(double)*lab*la);

  info=0;

  /* working array for pivot used by LU Factorization */
  ipiv = (int *) calloc(la, sizeof(int));

  int row = 0; //

  if (row == 1){ // LAPACK_ROW_MAJOR
    set_GB_operator_rowMajor_poisson1D(AB, &lab, &la);
    //write_GB_operator_rowMajor_poisson1D(AB, &lab, &la, "AB_row.dat");
    
    info = LAPACKE_dgbsv(LAPACK_ROW_MAJOR,la, kl, ku, NRHS, AB, la, ipiv, RHS, NRHS);
  
  } 
  else { // LAPACK_COL_MAJOR
    set_GB_operator_colMajor_poisson1D(AB, &lab, &la, &kv);
    //write_GB_operator_colMajor_poisson1D(AB, &lab, &la, "AB_col.dat");

    info = LAPACKE_dgbsv(LAPACK_COL_MAJOR,la, kl, ku, NRHS, AB, lab, ipiv, RHS, la);
  }    

  
  if(row == 1){// BLASS_ROW_MAJOR
      set_GB_operator_rowMajor_poisson1D(AB, &lab, &la);
      //write_GB_operator_rowMajor_poisson1D(AB, &lab, &la, "AB_row.dat");
      cblas_dgbmv(CblasRowMajor,CblasNoTrans,la,la,ku,kl,alpha,AB,lab,X_test_dgmv,incx,beta,Y_test_dgmv,incy);
  }
  else
  {
    // BLASS_COL_MAJOR
      set_GB_operator_colMajor_poisson1D(AB, &lab, &la,&kv);
      //write_GB_operator_rowMajor_poisson1D(AB, &lab, &la, "AB_row.dat");
      cblas_dgbmv(CblasColMajor,CblasNoTrans,la,la,ku,kl,alpha,AB,lab,X_test_dgmv,incx,beta,Y_test_dgmv,incy);
  }
  write_vec(Y_test_dgmv, &la, "Y_test_dgmv.dat"); 
    write_vec(X_test_dgmv, &la, "X_test_dgmv.dat"); 
  printf("\n INFO DGBSV = %d\n",info);

  write_xy(RHS, X, &la, "SOL.dat");

  /* Relative residual */
  temp = cblas_ddot(la, RHS, 1, RHS,1);
  temp = sqrt(temp);
  cblas_daxpy(la, -1.0, RHS, 1, EX_SOL, 1);
  relres = cblas_ddot(la, EX_SOL, 1, EX_SOL,1);
  relres = sqrt(relres);
  relres = relres / temp;
  
  printf("\nThe relative residual error is relres = %e\n",relres);

  free(RHS);
  free(EX_SOL);
  free(X);
  free(AB);
  free(ipiv);

  printf("\n\n--------- End -----------\n");
}
