#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "fpot.h"

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

#if 0
    printf("from %i to %i ", from, to);
    printf("dlin i: %i r: %f x: %f\n", index ,dlin[index].r, dlin[index].x);
#endif

    den = (pow2(dlin[index].r) + pow2(dlin[index].x));
    g = dlin[index].r / den;
    b = dlin[index].x / den;
    sh = 0.5 * dlin[index].b / base;
    yg[from][from] += g;
    yb[from][from] += b + sh;
    yg[to][to] += g;
    yb[to][to] += b + sh;

    yg[from][to] += -g;
    yb[from][to] += -b;
    yg[to][from] += -g;
    yb[to][from] += -b;
    
  }

#if 0
  {
    int i, j;

    for (i = 0; i < (npv+npq); i++) {
      for (j = 0; j < (npv+npq); j++) {
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
      b2[row][col] = -yb[get_index(b1idx[row])][get_index(b1idx[col])];
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
      printf("%+0.6f ", b1[row][col]);
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

  for (i = 0; i < (npq+npv); i++) {

    index = get_index(b1idx[i]);

    s = 0;
    for (k = 0; k < nbar; k++) {
      tetha = phase[index] - phase[k];
//      printf("%i %i %f\n", index, k, (yg[index][k] * cos(tetha)) + (yb[index][k] * sin(tetha)) * voltage[k]);
      
      s += (yg[index][k] * cos(tetha)) + (yb[index][k] * sin(tetha)) * voltage[k];
    }
    dp[i] = (((dbar[index].pg - dbar[index].ql) / base) - (voltage[i] * s)) / voltage[i];
//    printf("dp[%i] %f %f %f %f %f\n", i, dp[i], dbar[index].pg, dbar[b1idx[i]].pl, voltage[i], s);
  }

  return 0;
}

int getdth()
{
  int index;
  int n, i, j;
  double m, r;
  
  memset(dth, 0, sizeof(dth));

  for (j = 0; j < (npv+npq); j++) {

    for (n = j + 1; n < (npv+npq); n++) {
      
      if (b1[n][j] != 0.) {

        m = -b1[n][j] / b1[j][j];

        for (i = j; i < (npv+npq); i++) {
          b1[n][i] += m * b1[j][i];
        }
        dp[n] += m * dp[j];
      }
    }
  }

  for (i = (npv+npq) - 1; i >= 0; i--) {

    index = get_index(b1idx[i]);
    
    r = 0.;
    
    for (j = i; j < (npv+npq) - 1; j++) {
      r += b1[i][j] * dp[j];
    }
    
    r = dp[i] - r;

    dth[index] = r / b1[i][i];
//    printf("dth[%i] %f\n", index, dth[index]);
  }

  return 0;
}

int updatedq()
{
  int index, i, k;
  double tetha, s;

  for (i = 0; i < npq; i++) {

    index = get_index(b2idx[i]);

    s = 0;
    for (k = 0; k < nbar; k++) {
      tetha = phase[index] - phase[k];
      s += (yg[index][k] * sin(tetha)) + (yb[index][k] * cos(tetha)) * voltage[k];
    }
    dq[i] = (((dbar[index].qg - dbar[index].ql) / base) - (voltage[i] * s)) / voltage[i];
  }

  return 0;
}

int getdv()
{
  int index;
  int n, i, j;
  double m, r;
  
  memset(dv, 0, sizeof(dv));

  for (j = 0; j < npq; j++) {

    for (n = j + 1; n < npq; n++) {
      
      if (b2[n][j] != 0.) {

        m = -b2[n][j] / b2[j][j];

        for (i = j; i < npq; i++) {
          b2[n][i] += m * b2[j][i];
        }
        dv[n] += m * dv[j];
      }
    }
  }

#if 0
  for (j = 0; j < npq; j++) {
    printf("%+0.6f ", dq[j]);
    for (n = 0; n < npq; n++) {
      printf("%+0.6f ", b2[j][n]);
    }
    printf("\n");
  }
#endif
      
  for (i = npq - 1; i >= 0; i--) {

    index = get_index(b2idx[i]);

    r = 0.;
    
    for (j = i; j < npq - 1; j++) {
      r += b2[i][j] * dv[j];
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
//    printf("%i %f %f\n", index, voltage[index], phase[index]);
  }

  iter = 0;

  do {

    iter++;
    
    updatedp();

    getdth();

    for (i = 0; i < (npv+npq); i++) {
      index = get_index(b1idx[i]);
//      printf("p[%i] %f %f\n", index, phase[index], dth[index]);
      phase[index] += dth[index];

    }

    updatedq();

    getdv();

    for (i = 0; i < npq; i++) {
      index = get_index(b2idx[i]);
      voltage[index] += dv[index];
    }

    for (i = 0; i < nbar; i++) {
      printf("%i %f %f\n", i, voltage[i], phase[i]);
    }
    printf("\n");
    
  } while (iter < 1);
  
  return 0;
}
