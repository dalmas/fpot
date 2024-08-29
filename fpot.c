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
int b1id[LENGHT];
double b1[LENGHT][LENGHT];
int b2id[LENGHT];
double b2[LENGHT][LENGHT];
double voltage[LENGHT];
double phase[LENGHT];
double dv[LENGHT];
double dth[LENGHT];
double p[LENGHT];
double q[LENGHT];
double dp[LENGHT];
double dq[LENGHT];
double phi;

int print_usage()
{
  printf("%s\n\n", PACKAGE_STRING);
  printf("Usage:\nfpot filename phi\n");

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
  double rl, xl;

  memset(yg, 0, sizeof(yg));
  memset(yb, 0, sizeof(yb));
  memset(yb1, 0, sizeof(yb));
  memset(yb2, 0, sizeof(yb));
  
  for (index = 0; index < nlin; index ++) {

    from = get_index(dlin[index].from);
    to = get_index(dlin[index].to);

    rl = ((dlin[index].r * cos(phi)) - (dlin[index].x * sin(phi)));
    xl = ((dlin[index].r * sin(phi)) + (dlin[index].x * cos(phi)));
    
    den = (pow2(rl) + pow2(xl));
    g = rl / den;
    b = -xl / den;
    sh = 0.5 * dlin[index].b;

    yg[from][from] += g;
    yb[from][from] += b + sh + dbar[from].sh;
    yg[to][to] += g;
    yb[to][to] += b + sh + dbar[to].sh;

    yg[from][to] += -g;
    yb[from][to] += -b;
    yg[to][from] += -g;
    yb[to][from] += -b;

    yb2[from][from] += b + 2. * (sh + dbar[from].sh);
    yb2[to][to] += b + 2. * (sh + dbar[to].sh);
    yb2[from][to] += -b;
    yb2[to][from] += -b;

    b = -1. / xl;
    yb1[from][from] += b;
    yb1[to][to] += b;
    yb1[from][to] += -b;
    yb1[to][from] += -b;
  }
  
  return 0;
}

int formb()
{
  int index, b1i, b2i;
  int row, col, i;
  
  memset(b1, 0, sizeof(yg));
  memset(b2, 0, sizeof(yb));

  b1i = b2i = 0;
  
  for (index = 0; index < nbar; index++) {

    if (dbar[index].type != 2) {
      b1id[b1i++] = dbar[index].id;      
    }

    if (dbar[index].type == 0) {
      b2id[b2i++] = dbar[index].id;
    }
  }
  
  for (row = 0; row < (npv+npq); row++) {
    for (col = 0; col < (npv+npq); col++) {
      b1[row][col] = -yb1[get_index(b1id[row])][get_index(b1id[col])];
    }
  }

  for (col = 0; col < (npv+npq); col++) {

    for (row = col + 1; row < (npv+npq); row++) {
      
      if (b1[row][col] != 0.) {

        b1[row][col] = b1[row][col] / b1[col][col];

        for (i = row; i < (npv+npq); i++) {
          b1[row][i] -= b1[row][col] * b1[col][i];
        }
      }
    }
  }


  for (row = 0; row < npq; row++) {
    for (col = 0; col < npq; col++) {
      b2[row][col] = -yb2[get_index(b2id[row])][get_index(b2id[col])];
    }
  }

  for (col = 0; col < npq; col++) {

    for (row = col + 1; row < npq; row++) {
      
      if (b2[row][col] != 0.) {

        b2[row][col] = b2[row][col] / b2[col][col];

        for (i = row; i < npq; i++) {
          b2[row][i] -= b2[row][col] * b2[col][i];
        }
      }
    }
  }
  
  return 0;
}

int updatedp()
{
  int index, i, k;
  double tetha, s, pl;

  for (i = 0; i < nbar; i++) {

    s = 0;
    for (k = 0; k < nbar; k++) {
      tetha = phase[i] - phase[k];
      s += (((yg[i][k] * cos(tetha)) + (yb[i][k] * sin(tetha))) * voltage[k]);
    }
    p[i] = (voltage[i] * s);
    pl = ((((dbar[i].pg - dbar[i].pl) / base) * cos(phi)) - (((dbar[i].qg - dbar[i].ql) / base) * sin(phi)));

    dp[i] = (pl - p[i]) / voltage[i];
  }

  return 0;
}

int getdth()
{
  int index;
  int n, i, j;
  double r;
  double b[LENGHT];
  
  memset(dth, 0, sizeof(dth));

  for (i = 0; i < (npv+npq); i++) {
    index = get_index(b1id[i]);
    b[i] = dp[index];
  }
  
  for (j = 0; j < (npv+npq); j++) {
    for (n = j + 1; n < (npv+npq); n++) {  
      b[n] = b[n] - (b1[n][j] * b[j]);
    }
  }

  for (i = (npv+npq) - 1; i >= 0; i--) {

    index = get_index(b1id[i]);
    
    r = 0.;
    
    for (j = (npv+npq) - 1; j > i; j--) {
      r += b1[i][j] * dth[get_index(b1id[j])];
    }

    dth[index] = (b[i] - r) / b1[i][i];
  }

  return 0;
}

int updatedq()
{
  int index, i, k;
  double tetha, s, ql;

  for (i = 0; i < nbar; i++) {

    s = 0.;
    for (k = 0; k < nbar; k++) {
      tetha = phase[i] - phase[k];
      s += (((yg[i][k] * sin(tetha)) - (yb[i][k] * cos(tetha))) * voltage[k]);
    }
    q[i] = (voltage[i] * s);
    ql = ((((dbar[i].pg - dbar[i].pl) / base) * sin(phi)) + (((dbar[i].qg - dbar[i].ql) / base) * cos(phi)));
    dq[i] = (ql - q[i]) / voltage[i];
  }

  return 0;
}

int getdv()
{
  int index;
  int n, i, j;
  double r;
  double b[LENGHT];

  memset(dv, 0, sizeof(dv));

  for (i = 0; i < npq; i++) {
    index = get_index(b2id[i]);
    b[i] = dq[index];
  }
    
  for (j = 0; j < npq; j++) {
    for (n = j + 1; n < npq; n++) {  
      b[n] = b[n] - (b2[n][j] * b[j]);
    }
  }

  for (i = npq - 1; i >= 0; i--) {

    index = get_index(b2id[i]);

    r = 0.;
    
    for (j = npq - 1; j > i; j--) {
      r += b2[i][j] * dv[get_index(b2id[j])];
    }

    dv[index] = (b[i] - r) / b2[i][i];
  }

  return 0;
}


int main (int argc, char **argv)
{
  FILE *fd;
  int index, i;
  int iter;
  int pconv, qconv;
  
  if (argc != 3) {
    print_usage();

    return 1;
  }

  phi = atof(argv[2]);
  phi = M_PI * phi / 180.;
  
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
  }

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
        index = get_index(b1id[i]);
        phase[index] += dth[index];
      }

      pconv = 1;
      for (i = 0; i < (npv+npq); i++) {
        index = get_index(b1id[i]);
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
        index = get_index(b2id[i]);
        voltage[index] += dv[index];
      }

      qconv = 1;
      for (i = 0; i < npq; i++) {
        index = get_index(b2id[i]);
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

  printf("\niter = %i\n", iter);
  for (i = 0; i < nbar; i++) {
    printf("%i %+0.3f\t%+0.3f\n", i, voltage[i], 180. * phase[i] / M_PI);
  }


  for (i = 0; i < nbar; i++) {
//    printf("%s %f %f\n", dbar[i].name, p[i]*base, q[i]*base);
    printf("%s %f %f\n", dbar[i].name, ((p[i]*cos(phi))+(q[i]*sin(phi)))*base, ((q[i]*cos(phi))-(p[i]*sin(phi)))*base);
  }


  return 0;
}
