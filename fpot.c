#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define DEBUG 1

#define LENGHT 1000
#define LINE_LENGHT 80
#define EXCODE_LENGHT 4
#define MNM_LENGHT 4
#define NAME_LENGHT 12

char titu[LINE_LENGHT];

double tepa;
double tepr;
int base;
int acit;

typedef struct {
  int id;
  int type;
  char base;
  char *name[NAME_LENGHT];
  char group;
  double voltage;
  double phase;
  double pg;
  double qg;
  double qn;
  double qm;
  int bc;
  double pl;
  double ql;
  double sh;
  int area;
  double cf;
} dbar_t;

dbar_t dbar[LENGHT];

struct dlin_t {
  int from;
  int to;
  double r;
  double x;
  double b;
  double tap;
  double tmn;
  double tmx;
} dlin[LENGHT];

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

#define pow2(x) ((x) * (x))

int print_usage()
{
  printf("%s\n\n", PACKAGE_STRING);
  printf("Usage:\nfpot filename\n");

  return 0;
}

int input_data (FILE * fd)
{
  char line[LINE_LENGHT];
  int ch;
  char excode[EXCODE_LENGHT];
  char mnm [MNM_LENGHT];
  double cte;
  int i;
  char *sptr;
  char data[13];
  
  while (!feof(fd)) {

    if (fgets(line, LINE_LENGHT, fd) == NULL) {
      printf("Unable to read file.\n");
      return -1;
    }
    
    if (line[0] == '(') continue;


    sscanf(line, "%s", excode);

    if (strncmp("TITU", excode, EXCODE_LENGHT) == 0) {
      if (fgets(titu, LINE_LENGHT, fd) == NULL) {
        printf("Unable to read file.\n");
        return -1;
      }
    }

    else if (strncmp("DCTE", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) break;
        
        i = 0;
        while (sscanf(&line[i], "%s %lf", mnm, &cte) == 2) {
          i +=12;

          if (strncmp("TEPA", mnm, MNM_LENGHT) == 0) {
            tepa = cte;
          }
          else if (strncmp("TEPR", mnm, MNM_LENGHT) == 0) {
            tepr = cte;
          }
          else if (strncmp("BASE", mnm, MNM_LENGHT) == 0) {
            base = cte;
          }
          else if (strncmp("ACIT", mnm, MNM_LENGHT) == 0) {
            acit = cte;
          }
        }    
      }
    }

    else if (strncmp("DBAR", excode, EXCODE_LENGHT) == 0) {

      nbar = 0;
      npv = 0;
      npq = 0;
      
      for (;;) {
        
        do {
          memset(line, ' ', LINE_LENGHT);
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          dbar[nbar].id = 9999;
          break;
        }

        line[strlen(line)-1] = ' ';

        sptr = line;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dbar[nbar].id = atoi(data);
        }

        sptr = line + 5;
        switch (*sptr) {
          case ' ':
          case 'A':
          case '0':
            break;
          case 'E':
          case '1':
          case 'M':
          case '2':
            printf("Operation not implemeted %c.\n", *sptr);
            return -1;
          default:
            printf("Unknown operation %c.\n", *sptr);
            return -1;
        }

        sptr = line + 7;
        switch (*sptr) {
          case ' ':
          case '0':
            dbar[nbar].type = 0;
            npq++;
            break;
          case '1':
            dbar[nbar].type = 1;
            npv++;
            break;
          case '2':
            dbar[nbar].type = 2;
            break;
          case '3':
            npq++;
            dbar[nbar].type = 3;
            break;

          default:
            printf("Unknown bar type %c\n", *sptr);
            return -1;
        }

        sptr = line + 8;
        dbar[nbar].base = *sptr == ' ' ? '0' : *sptr ;

        sptr = line + 9;
        strncpy((char *)dbar[nbar].name, sptr, NAME_LENGHT);

        sptr = line + 21;
        dbar[nbar].group = *sptr == ' ' ? '0' : *sptr ;

        sptr = line + 22;
        strncpy(data, sptr, 4); data[4] = 0;
        dbar[nbar].voltage = 0.001 * atoi(data);

        sptr = line + 26;
        strncpy(data, sptr, 4); data[4] = 0;
        sscanf(data, "%lf", &dbar[nbar].phase);

        dbar[nbar].phase = M_PI * dbar[nbar].phase / 180.;

        sptr = line + 30;
        strncpy(data, sptr, 5); data[5] = 0;
        sscanf(data, "%lf", &dbar[nbar].pg);

        sptr = line + 35;
        strncpy(data, sptr, 5); data[5] = 0;
        sscanf(data, "%lf", &dbar[nbar].qg);

        sptr = line + 40;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[nbar].type == 2) {
            dbar[nbar].qn = -9999.;
          }
          else {
            dbar[nbar].qn = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].qn);
        }

        sptr = line + 45;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[nbar].type == 2) {
            dbar[nbar].qm = -9999.;
          }
          else {
            dbar[nbar].qm = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].qm);
        }

        sptr = line + 50;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].bc = dbar[nbar].id;
        }
        else {
          dbar[nbar].bc = atoi(data);
        }

        sptr = line + 55;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].pl = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].pl);
        }

        sptr = line + 60;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].ql = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].ql);
        }

        sptr = line + 65;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dbar[nbar].sh = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[nbar].sh);
        }

        sptr = line + 70;
        strncpy(data, sptr, 2); data[2] = 0;
        if (strncmp(data, "  ", 2) == 0) {
          dbar[nbar].area = 0;
        }
        else {
          dbar[nbar].area = atoi(data);
        }

        sptr = line + 72;
        strncpy(data, sptr, 4); data[4] = 0;
        dbar[nbar].cf = 0.001 * atoi(data);
        
        nbar++;
      }
    }

    else if (strncmp("DLIN", excode, EXCODE_LENGHT) == 0) {

      nlin = 0;
      
      for (;;) {
        
        do {
          memset(line, ' ', LINE_LENGHT);
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }

        line[strlen(line)-1] = ' ';
        
        sptr = line;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[nlin].from = atoi(data);
        }

        sptr = line + 5;
        switch (*sptr) {
          case ' ':
          case 'A':
          case '0':
            break;
          case 'E':
          case '1':
          case 'M':
          case '2':
            printf("Operation not implemeted %c.\n", *sptr);
            return -1;
          default:
            printf("Unknown operation %c.\n", *sptr);
            return -1;
        }

        sptr = line + 8;
        strncpy(data, sptr, 4); data[4] = 0;
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[nlin].to = atoi(data);
        }

        sptr = line + 17;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].r = 0.;
        }
        else {
          dlin[nlin].r = atof(data);
        }

        sptr = line + 23;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].x = 0.;
        }
        else {
          dlin[nlin].x = atof(data);
        }

        sptr = line + 29;
        strncpy(data, sptr, 6); data[6] = 0;
        if (strncmp(data, "      ", 6) == 0) {
          dlin[nlin].b = 0.;
        }
        else {
          dlin[nlin].b = atof(data);
        }

        sptr = line + 35;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tap = 0.;
        }
        else {
          dlin[nlin].tap = atof(data);
        }

        sptr = line + 40;
        
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmn = 0.;
        }
        else {
          dlin[nlin].tmn = atof(data);
        }

        sptr = line + 40;
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmn = 0.;
        }
        else {
          dlin[nlin].tmn = atof(data);
        }

        sptr = line + 45;
        
        strncpy(data, sptr, 5); data[5] = 0;
        if (strncmp(data, "     ", 5) == 0) {
          dlin[nlin].tmx = 0.;
        }
        else {
          dlin[nlin].tmx = atof(data);
        }
          
        nlin++;
      }
    }

    else if (strncmp("DGER", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }
      }
    }

    else if (strncmp("DGLT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "99", 2) == 0) {
          break;
        }
      }
    }

    else if (strncmp("DGBT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          if (fgets(line, LINE_LENGHT, fd) == NULL) {
            printf("Unable to read file.\n");
            return -1;
          }
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "99", 2) == 0) {
          break;
        }
      }
    }

    else if (strncmp("FIM", excode, EXCODE_LENGHT) == 0) {
      return 0;
    }

    else {
      printf("Execution code %s not implemented\n", excode);
      return -1;
    }
  }
  
  return -1;
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
      printf("%i %i %f\n", index, k, (yg[index][k] * cos(tetha)) + (yb[index][k] * sin(tetha)) * voltage[k]);
      
      s += (yg[index][k] * cos(tetha)) + (yb[index][k] * sin(tetha)) * voltage[k];
    }
    dp[i] = (((dbar[index].pg - dbar[b1idx[i]].pl) / base) - (voltage[i] * s)) / voltage[i];
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
    dq[i] = (((dbar[index].qg - dbar[b1idx[i]].ql) / base) - (voltage[i] * s)) / voltage[i];
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
