#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "fpot.h"

char titu[LINE_LENGHT];
double tepa;
double tepr;
int base;
int acit;
dbar_t dbar[LENGHT];
dlin_t dlin[LENGHT];
int nbar;
int nlin;
int npv;
int npq;
double yg[LENGHT][LENGHT];
double yb[LENGHT][LENGHT];
int b1idx[LENGHT];
double b1[LENGHT][LENGHT];
int b2idx[LENGHT];
double b2[LENGHT][LENGHT];
double voltage[LENGHT];
double phase[LENGHT];
double dp[LENGHT];
double dq[LENGHT];
double dv[LENGHT];
double dth[LENGHT];

int print_usage()
{
  printf("%s\n\n", PACKAGE_STRING);
  printf("Usage:\nfpot filename\n");

  return 0;
}


int get_index(int id)
{
  int i;
  
  for (i = 0; i < nbar; i++) {
    if (dbar[i].id == id) {
      return i;
    }
  }

  return -1;
}


int formyb()
{
  int index, i, j;
  double den, g, b, sh;
  int from, to;

  memset(yg, 0, sizeof(yg));
  memset(yb, 0, sizeof(yb));
  
//  printf("nbar: %i nlin: %i npv: %i npq: %i\n", nbar, nlin, npv, npq);

  for (index = 0; index < nlin; index ++) {

    from = get_index(dlin[index].from);
    to = get_index(dlin[index].to);

#if 1
    printf("from %i to %i ", from, to);
    printf("dlin i: %i r: %f x: %f\n", index ,dlin[index].r, dlin[index].x);
#endif

    den = (pow2(dlin[index].r) + pow2(dlin[index].x));
    g = dlin[index].r / den;
    b = -dlin[index].x / den;
    sh = 0.5 * dlin[index].b;
    yg[from][from] += g;
    yb[from][from] += b + sh;
    yg[to][to] += g;
    yb[to][to] += b + sh;

    yg[from][to] += -g;
    yb[from][to] += -b;
    yg[to][from] += -g;
    yb[to][from] += -b;
    
  }

#if 1
  {
    int i, j;

    for (i = 0; i < nbar; i++) {
      for (j = 0; j < nbar; j++) {
        if ((yg[i][j] != 0.) || (yb[i][j] != 0.)) {
          printf("(%+.3lf %+.3lfj) ", yg[i][j], yb[i][j]);
        }
        else {
          printf("                 ");
        }
        
      }
      printf("\n");
    }
  }
#endif
  
  
  return 0;
}

int formb()
{
  int index, b1i, b2i;
  int row,col;
  
  memset(b1, 0, sizeof(yg));
  memset(b2, 0, sizeof(yb));

  b1i = b2i = 0;
  
  for (index = 0; index < nlin; index++) {

//    printf("lin %i index %i type %i\n",index, dbar[index].id, dbar[index].type);
    if (dbar[index].type != 2) {
      b1idx[b1i++] = dbar[index].id;
    }

    if (dbar[index].type == 0) {
      b2idx[b2i++] = dbar[index].id;
    }
  }
  
  for (row = 0; row < (npv+npq); row++) {
    for (col = 0; col < (npv+npq); col++) {
      b1[row][col] = -yb[get_index(b1idx[row])][get_index(b1idx[col])];
    }
  }

  for (row = 0; row < npq; row++) {
    for (col = 0; col < npq; col++) {
      b2[row][col] = -yb[get_index(b2idx[row])][get_index(b2idx[col])];
    }
  }

#if 0
  for (row = 0; row < (npv+npq); row++) {
    for (col = 0; col < (npv+npq); col++) {
      printf("%+0.6f ", b1[row][col]);
    }
    printf("\n");
  }

  printf("\n");
  for (row = 0; row < npq; row++) {
    for (col = 0; col < npq; col++) {
      printf("%+0.6f ", b2[row][col]);
    }
    printf("\n");
  }
#endif
  
  return 0;
}

int updatedp()
{
  int index, i, k;
  double tetha, s;

  for (i = 0; i < nbar; i++) {

    s = 0;
    for (k = 0; k < nbar; k++) {
      tetha = phase[i] - phase[k];
      s += (yg[i][k] * cos(tetha)) + (yb[i][k] * sin(tetha)) * voltage[k];
//      printf("P = ((%f * %f) + (%f * %f)) * %f = %f %f\n", yg[i][k], cos(tetha), yb[i][k], sin(tetha), voltage[k], (yg[i][k] * cos(tetha)) + (yb[i][k] * sin(tetha)) * voltage[k], s);
  
    }
    dp[i] = (((dbar[i].pg - dbar[i].ql) / base) - (voltage[i] * s)) / voltage[i];

    printf("dp[%i] %f P %f\n", i, dp[i], s * base);
  }

  return 0;
}

int getdth()
{
  int index;
  int n, i, j;
  double m, r;
  double b[LENGHT];
  
  memset(dth, 0, sizeof(dth));

  for (i = 0; i < (npv+npq); i++) {
    index = get_index(b1idx[i]);
    b[i] = dp[index];
  }
  
  for (j = 0; j < (npv+npq); j++) {

    for (n = j + 1; n < (npv+npq); n++) {
      
      if (b1[n][j] != 0.) {

        m = -b1[n][j] / b1[j][j];

        for (i = j; i < (npv+npq); i++) {
          b1[n][i] += m * b1[j][i];
        }
        b[n] += m * b[j];
      }
    }
  }

#if 0
  for (j = 0; j < (npv+npq); j++) {
    printf("%+0.6f ", b[j]);
    for (n = 0; n < (npv+npq); n++) {
      printf("%+0.6f ", b1[j][n]);
    }
    printf("\n");
  }
#endif

  for (i = (npv+npq) - 1; i >= 0; i--) {

    index = get_index(b1idx[i]);
    
    r = 0.;
    
    for (j = (npv+npq) - 1; j > i; j--) {
      r += b1[i][j] * dth[get_index(b1idx[j])];
    }

    r = b[i] - r;

    dth[index] = r / b1[i][i];
//    printf("dth[%i] = %f %f / %f\n", index, dth[index], r, b1[i][i]);
  }

  return 0;
}

int updatedq()
{
  int index, i, k;
  double tetha, s;

  for (i = 0; i < nbar; i++) {

    s = 0;
    for (k = 0; k < nbar; k++) {
      tetha = phase[i] - phase[k];
      s += (yg[i][k] * sin(tetha)) + (yb[i][k] * cos(tetha)) * voltage[k];
    }    

    dq[i] = (((dbar[i].qg - dbar[i].ql) / base) - (voltage[i] * s)) / voltage[i];
      
    printf("dq[%i] %f Q %f\n", i, dq[i], s * base);
  }

  return 0;
}

int getdv()
{
  int index;
  int n, i, j;
  double m, r;
  double b[LENGHT];
  
  memset(dv, 0, sizeof(dv));

  for (i = 0; i < npq; i++) {
    index = get_index(b2idx[i]);
    b[i] = dv[index];
  }

  for (j = 0; j < npq; j++) {

    for (n = j + 1; n < npq; n++) {
      
      if (b2[n][j] != 0.) {

        m = -b2[n][j] / b2[j][j];

        for (i = j; i < npq; i++) {
          b2[n][i] += m * b2[j][i];
        }
        b[n] += m * b[j];
      }
    }
  }

#if 0
  for (j = 0; j < npq; j++) {
    printf("%+0.6f ", b[j]);
    for (n = 0; n < npq; n++) {
      printf("%+0.6f ", b2[j][n]);
    }
    printf("\n");
  }
#endif
      
  for (i = npq - 1; i >= 0; i--) {

    index = get_index(b2idx[i]);

    r = 0.;
    
    for (j = npq - 1; j > i; j--) {
      r += b2[i][j] * dv[get_index(b2idx[j])];
    }

    r = dq[i] - r;
    
    dv[index] = r / b2[i][i];
//    printf("dv[%i] = %f %f / %f\n", index, dv[index], r, b2[i][i]);
  }

  return 0;
}


int main (int argc, char **argv)
{
  FILE *fd;
  int index, i;
  int iter;
  int conv;
  
  if (argc != 2) {
    print_usage();

    return 1;
  }

  fd = fopen(argv[1], "r");
  if (fd == NULL) {
    printf("Unable to open file %s.\n", argv[1]);

    return 2;
  }

  if (input_data(fd) != 0) {
    fclose(fd);
    
    return 3;
  }
  fclose(fd);

  formyb();

  formb();

  for (index = 0; index < nbar; index++) {
    voltage[index] = dbar[index].voltage;
    phase[index] = dbar[index].phase;
    printf("%i %f %f\n", index, voltage[index], phase[index]);
  }
  printf("\n");

  iter = 0;

  do {

    iter++;
    
    updatedp();

    conv = 1;
    for (i = 0; i < (npv+npq); i++) {
      index = get_index(b1idx[i]);
//      printf("dp %f %f\n", fabs(dp[index]), tepa);
      if (fabs(dp[index]) > tepa) {
        conv = 0;
      }
    }
    if (conv) {
      printf("potencia ativa convergiu na iteracao %i\n", iter);
    }    

    getdth();

    for (i = 0; i < (npv+npq); i++) {
      index = get_index(b1idx[i]);
      phase[index] += dth[index];
    }

    updatedq();

    conv = 1;
    for (i = 0; i < npq; i++) {
      index = get_index(b2idx[i]);
//      printf("dq %f %f\n", fabs(dq[index]), tepr);
      if (fabs(dq[index]) > tepr) {
        conv = 0;
      }
    }
    if (conv) {
      printf("potencia reativa convergiu na iteracao %i\n", iter);
    }

    getdv();

    for (i = 0; i < npq; i++) {
      index = get_index(b2idx[i]);
      voltage[index] += dv[index];
    }

    for (i = 0; i < nbar; i++) {
      printf("%i %f %f\n", i, voltage[i], phase[i]);
    }
    printf("iter = %i\n\n", iter);

  } while (conv || (iter < 10));
  
  return 0;
}
