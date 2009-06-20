#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "fpot.h"

char titu[LINE_LENGHT];
double base;
double tepa ;
double tepr;
int acit;
dbar_t dbar[LENGHT];
dlin_t dlin[LENGHT];
dger_t dger[LENGHT];
int nbar;
int nlin;
int npv;
int npq;
int nger;
double yg[LENGHT][LENGHT];
double yb[LENGHT][LENGHT];
double yb1[LENGHT][LENGHT];
double yb2[LENGHT][LENGHT];
int b1idx[LENGHT];
double b1[LENGHT][LENGHT];
int b2idx[LENGHT];
double b2[LENGHT][LENGHT];
double voltage[LENGHT];
double phase[LENGHT];
double dv[LENGHT];
double dth[LENGHT];
double p[LENGHT];
double q[LENGHT];
double dp[LENGHT];
double dq[LENGHT];

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


int get_dger_index(int id)
{
  int i;

  for (i = 0; i < nger; i++) {
    if (dger[i].id == id) {
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
  memset(yb1, 0, sizeof(yb));
  memset(yb2, 0, sizeof(yb));
  
  printf("nbar: %i nlin: %i npv: %i npq: %i\n", nbar, nlin, npv, npq);

  for (index = 0; index < nlin; index ++) {

    from = get_index(dlin[index].from);
    to = get_index(dlin[index].to);

#if 0
    printf("from %i to %i ", from, to);
    printf("dlin i: %i r: %f x: %f\n", index ,dlin[index].r, dlin[index].x);
#endif

    den = (pow2(dlin[index].r) + pow2(dlin[index].x));
    if (den == 0.) {
      printf("dlin i: %i r: %f x: %f\n", index ,dlin[index].r, dlin[index].x);
      return -1;
    }
      
    g = dlin[index].r / den;
    b = -dlin[index].x / den;
    sh = 0.5 * dlin[index].b;
 
    yg[from][from] += g;
    yb[from][from] += b + sh + dbar[from].sh;
    yg[to][to] += g;
    yb[to][to] += b + sh + dbar[to].sh;

    yg[from][to] += -g;
    yb[from][to] += -b;
    yg[to][from] += -g;
    yb[to][from] += -b;

    yb2[from][from] += -yb[from][from] - (sh + dbar[from].sh);
    yb2[to][to] += -yb[to][to] -(sh + dbar[to].sh);
    yb2[from][to] += -b;
    yb2[to][from] += -b;

    b = -1. / dlin[index].x;
    yb1[from][from] += b;
    yb1[to][to] += b;
    yb1[from][to] += -b;
    yb1[to][from] += -b;


  }

#if 0
  {
    int i, j;

    for (i = 0; i < nbar; i++) {
      for (j = 0; j < nbar; j++) {
        if ((yg[i][j] != 0.) || (yb[i][j] != 0.)) {
          printf("%i %i (%+.3lf %+.3lfj) ", i , j,  yg[i][j], yb[i][j]);
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
  
  for (index = 0; index < nbar; index++) {

    if (dbar[index].type != 2) {
      b1idx[b1i++] = dbar[index].id;      
    }

    if (dbar[index].type == 0) {
      b2idx[b2i++] = dbar[index].id;
    }
  }
  
  for (row = 0; row < (npv+npq); row++) {
    for (col = 0; col < (npv+npq); col++) {
      b1[row][col] = -yb1[get_index(b1idx[row])][get_index(b1idx[col])];
    }
  }

  for (row = 0; row < npq; row++) {
    for (col = 0; col < npq; col++) {
      b2[row][col] = yb2[get_index(b2idx[row])][get_index(b2idx[col])];
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
      s += (((yg[i][k] * cos(tetha)) + (yb[i][k] * sin(tetha))) * voltage[k]);
//      printf("P = ((%f * %f) + (%f * %f)) * %f = %f %f\n", yg[i][k], cos(tetha), yb[i][k], sin(tetha), voltage[k], ((yg[i][k] * cos(tetha)) + (yb[i][k] * sin(tetha))) * voltage[k], s);
  
    }
#if 0
    if (dbar[i].type != 0) {

      index = get_dger_index(dbar[i].id);
      
      if (s < dger[index].pmn) {
        s = dger[i].pmn;
      }
      if (s > dger[index].pmx) {
        s = dger[i].pmx;
      }
    }
#endif
    p[i] = (voltage[i] * s);
    dp[i] = (((dbar[i].pg - dbar[i].ql) / base) - p[i]) / voltage[i];
    printf("dp[%i] = %f = %f - %f\n", i, dp[i], ((dbar[i].pg - dbar[i].ql) / base), (voltage[i] *s));

  }

  return 0;
}

int getdth()
{
  int index;
  int n, i, j;
  double m, r;
  double b[LENGHT];
  double A[LENGHT][LENGHT];
  
  memset(dth, 0, sizeof(dth));

  for (i = 0; i < (npv+npq); i++) {
    index = get_index(b1idx[i]);
    b[i] = dp[index];
    for (j = 0; j < (npv+npq); j++) {
      A[i][j] = b1[i][j];
    }
  }
  
  for (j = 0; j < (npv+npq); j++) {

    for (n = j + 1; n < (npv+npq); n++) {
      
      if (A[n][j] != 0.) {

        m = -A[n][j] / A[j][j];

        for (i = j; i < (npv+npq); i++) {
          A[n][i] += m * A[j][i];
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
      r += A[i][j] * dth[get_index(b1idx[j])];
    }

    r = b[i] - r;

    dth[index] = r / A[i][i];
    printf("dth[%i] = %f %f / %f\n", index, dth[index], r, A[i][i]);
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
      s += (((yg[i][k] * sin(tetha)) - (yb[i][k] * cos(tetha))) * voltage[k]);
//      printf("Q = ((%f * %f) - (%f * %f)) * %f = %f %f\n", yg[i][k], sin(tetha), yb[i][k], cos(tetha), voltage[k], (yg[i][k] * sin(tetha)) - (yb[i][k] * cos(tetha)) * voltage[k], s);
    }    
#if 0 
    if (dbar[i].type != 0) {
      if (s < dbar[i].qn) {
        printf("Qn[%i] de %f para %f\n", i, s, dbar[i].qn);
        s = dbar[i].qn;
      }
      if (s > dbar[i].qm) {
        printf("Qm[%i] de %f para %f\n", i, s, dbar[i].qm);
        s = dbar[i].qm;
      }
    }
#endif
    q[i] = (voltage[i] * s);
    dq[i] = (((dbar[i].qg - dbar[i].ql) / base) - q[i]) / voltage[i];
      
    printf("dq[%i] %f = %f - %f\n", i, dq[i], ((dbar[i].qg - dbar[i].ql) / base), (s * voltage[i]));
  }

  return 0;
}

int getdv()
{
  int index;
  int n, i, j;
  double m, r;
  double b[LENGHT];
  double A[LENGHT][LENGHT];

  memset(dv, 0, sizeof(dv));

  for (i = 0; i < npq; i++) {
    index = get_index(b2idx[i]);
    b[i] = dq[index];
    for (j = 0; j < npq; j++) {
      A[i][j] = b2[i][j];
    }
  }

  for (j = 0; j < npq; j++) {

    for (n = j + 1; n < npq; n++) {
      
      if (A[n][j] != 0.) {

        m = -A[n][j] / A[j][j];

        for (i = j; i < npq; i++) {
          A[n][i] += m * A[j][i];
        }
        b[n] += m * b[j];
      }
    }
  }

#if 0
  for (j = 0; j < npq; j++) {
    printf("%+0.6f ", b[j]);
    for (n = 0; n < npq; n++) {
      printf("%+0.6f ", A[j][n]);
    }
    printf("\n");
  }
#endif
      
  for (i = npq - 1; i >= 0; i--) {

    index = get_index(b2idx[i]);

    r = 0.;
    
    for (j = npq - 1; j > i; j--) {
      r += A[i][j] * dv[get_index(b2idx[j])];
    }

    r = b[i] - r;
    
    dv[index] = r / A[i][i];
    printf("dv[%i] = %f %f / %f\n", index, dv[index], r, A[i][i]);
  }

  return 0;
}


int main (int argc, char **argv)
{
  FILE *fd;
  int index, i;
  int iter;
  int pconv, qconv;
  
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

  if (formyb() != 0) {
    return 4;
  }

  formb();

  for (index = 0; index < nbar; index++) {
    voltage[index] = dbar[index].voltage;
    phase[index] = dbar[index].phase;
//    printf("%i %f %f\n", index, voltage[index], phase[index]);
  }
//  printf("\n");

  iter = 0;
  pconv = 0;
  qconv = 0;
  
  do {

    iter++;
    if(iter > acit) break;

    if (!pconv) {
      
      updatedp();

      getdth();

      for (i = 0; i < (npv+npq); i++) {
        index = get_index(b1idx[i]);
        phase[index] += dth[index];
      }

      pconv = 1;
      for (i = 0; i < (npv+npq); i++) {
        index = get_index(b1idx[i]);
//      printf("dp %f %f\n", fabs(dp[index]), tepa);
        if (fabs(dp[index]) > tepa) {
          pconv = 0;
        }
      }
      if (pconv) {
        printf("potencia ativa convergiu na iteracao %i\n", iter);
        for (i = 0; i < nbar; i++) {
          dp[i] = 0.;
        }
      }    
    }

    if (!qconv) {

      updatedq();

      getdv();

      for (i = 0; i < npq; i++) {
        index = get_index(b2idx[i]);
        voltage[index] += dv[index];
      }

      qconv = 1;
      for (i = 0; i < npq; i++) {
        index = get_index(b2idx[i]);
//      printf("dq %f %f\n", fabs(dq[index]), tepr);
        if (fabs(dq[index]) > tepr) {
          qconv = 0;
        }
      }
      if (qconv) {
        printf("potencia reativa convergiu na iteracao %i\n", iter);

        for (i = 0; i < nbar; i++) {
          dq[i] = 0.;
        }
      }
    }

  } while (!qconv || !pconv);

  printf("iter = %i\n\n", iter);
  for (i = 0; i < nbar; i++) {
    printf("%i %+0.3f\t%+0.3f\n", i, voltage[i], 180. * phase[i] / M_PI);
  }


  for (i = 0; i < nbar; i++) {
    int from, to;
    int j;
    
    from = get_index(dbar[i].id);
    
    for (j = 0; j < nlin; j++) {

      double tetha, p,q;    

      if (get_index(dlin[j].from) == from) {

        to = get_index(dlin[j].to);
        
        tetha = phase[from] - phase[to];
        p = (((yg[from][to] * cos(tetha)) + (yb[from][to] * sin(tetha))) * voltage[to]);
        q = (((yg[from][to] * sin(tetha)) - (yb[from][to] * cos(tetha))) * voltage[to]);
        printf("from %s to %s %+0.3f\t%+0.3f\n", dbar[from].name, dbar[to].name, p * base, q * base);
      }
    }
    printf("\n");
  }
  

  return 0;
}
