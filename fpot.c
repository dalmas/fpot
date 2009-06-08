#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

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
  int index;
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

dbar_t dbar[1000];

struct dlin_t {
  int from;
  int to;
  double r;
  double x;
  double b;
  double tap;
  double tmn;
  double tmx;
} dlin[1000];
  
  
int print_usage()
{
  printf("%s\n\n", PACKAGE_STRING);
  printf("Usage:\nfpot filename\n");

  return 0;
}

int input_data (FILE * fd)
{
  char line[LINE_LENGHT];
  int lnlen;
  int ch;
  char excode[EXCODE_LENGHT];
  char mnm [MNM_LENGHT];
  double cte;
  int i;
  char *sptr;
  char data[13];
  
  while (!feof(fd)) {
    fgets(line, LINE_LENGHT, fd);
    if (line[0] == '(') continue;


    sscanf(line, "%s", excode);

    if (strncmp("TITU", excode, EXCODE_LENGHT) == 0) {
      fgets(titu, LINE_LENGHT, fd);
    }

    else if (strncmp("DCTE", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
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

      i = 0;
      
      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          dbar[i].index = 9999;
          break;
        }

        lnlen = strlen(line);

        if (lnlen < 4) continue;
        sptr = line;
        strncpy(data, sptr, 4);
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dbar[i].index = atoi(data);
        }

        if (lnlen < 6) continue;
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

        if (lnlen < 8) continue;
        sptr = line + 7;
        switch (*sptr) {
          case ' ':
          case '0':
            dbar[i].type = 0;
            break;
          case '1':
            dbar[i].type = 1;
            break;
          case '2':
            dbar[i].type = 2;
            break;
          case '3':
            dbar[i].type = 3;
            break;

          default:
            printf("Unknown bar type %c\n", *sptr);
            return -1;
        }

        if (lnlen < 9) continue;
        sptr = line + 8;
        dbar[i].base = *sptr == ' ' ? '0' : *sptr ;

        if (lnlen < 21) continue;
        sptr = line + 9;
        strncpy(dbar[i].name, sptr, NAME_LENGHT);

        if (lnlen < 22) continue;
        sptr = line + 21;
        dbar[i].group = *sptr == ' ' ? '0' : *sptr ;

        if (lnlen < 26) continue;
        sptr = line + 22;
        strncpy(data, sptr, 4);
        dbar[i].voltage = 0.001 * atoi(data);

        if (lnlen < 30) continue;
        sptr = line + 26;
        strncpy(data, sptr, 4);
        sscanf(data, "%lf", &dbar[i].phase);

        if (lnlen < 35) continue;
        sptr = line + 30;
        strncpy(data, sptr, 5);
        sscanf(data, "%lf", &dbar[i].pg);

        if (lnlen < 40) continue;
        sptr = line + 35;
        strncpy(data, sptr, 5);
        sscanf(data, "%lf", &dbar[i].qg);

        if (lnlen < 45) continue;
        sptr = line + 40;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[i].type == 2) {
            dbar[i].qn = -9999.;
          }
          else {
            dbar[i].qn = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[i].qn);
        }

        if (lnlen < 50) continue;
        sptr = line + 45;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          if (dbar[i].type == 2) {
            dbar[i].qm = -9999.;
          }
          else {
            dbar[i].qm = 0.;
          }
        }
        else {
          sscanf(data, "%lf", &dbar[i].qm);
        }

        if (lnlen < 55) continue;
        sptr = line + 50;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dbar[i].bc = dbar[i].index;
        }
        else {
          dbar[i].bc = atoi(data);
        }

        if (lnlen < 60) continue;
        sptr = line + 55;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dbar[i].pl = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[i].pl);
        }

        if (lnlen < 65) continue;
        sptr = line + 60;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dbar[i].ql = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[i].ql);
        }

        if (lnlen < 70) continue;
        sptr = line + 65;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dbar[i].sh = 0.;
        }
        else {
          sscanf(data, "%lf", &dbar[i].sh);
        }

        if (lnlen < 72) continue;
        sptr = line + 70;
        strncpy(data, sptr, 2);
        if (strncmp(data, "  ", 2) == 0) {
          dbar[i].area = 0;
        }
        else {
          dbar[i].area = atoi(data);
        }

        if (lnlen < 76) continue;
        sptr = line + 72;
        strncpy(data, sptr, 4);
        dbar[i].cf = 0.001 * atoi(data);
        
        i++;
      }
    }

    else if (strncmp("DLIN", excode, EXCODE_LENGHT) == 0) {

      i = 0;
      
      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }

        lnlen = strlen(line);

        if (lnlen < 4) continue;
        sptr = line;
        strncpy(data, sptr, 4);
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[i].from = atoi(data);
        }

        if (lnlen < 6) continue;
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

        if (lnlen < 12) continue;
        sptr = line + 8;
        strncpy(data, sptr, 4);
        if (strncmp(data, "    ", 4) == 0) {
          printf("Bar number field is mandatory.\n");
          return -1;
        }
        else {
          dlin[i].to = atoi(data);
        }

        if (lnlen < 23) continue;
        sptr = line + 17;
        strncpy(data, sptr, 6);
        if (strncmp(data, "      ", 6) == 0) {
          dlin[i].r = 0.;
        }
        else {
          dlin[i].r = atof(data);
        }

        if (lnlen < 29) continue;
        sptr = line + 23;
        strncpy(data, sptr, 6);
        if (strncmp(data, "      ", 6) == 0) {
          dlin[i].x = 0.;
        }
        else {
          dlin[i].x = atof(data);
        }

        if (lnlen < 35) continue;
        sptr = line + 29;
        strncpy(data, sptr, 6);
        if (strncmp(data, "      ", 6) == 0) {
          dlin[i].b = 0.;
        }
        else {
          dlin[i].b = atof(data);
        }

        if (lnlen < 40) continue;
        sptr = line + 35;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dlin[i].tap = 0.;
        }
        else {
          dlin[i].tap = atof(data);
        }

        if (lnlen < 45) continue;
        sptr = line + 40;
        
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dlin[i].tmn = 0.;
        }
        else {
          dlin[i].tmn = atof(data);
        }

        if (lnlen < 45) continue;
        sptr = line + 40;
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dlin[i].tmn = 0.;
        }
        else {
          dlin[i].tmn = atof(data);
        }

        if (lnlen < 50) continue;
        sptr = line + 45;
        
        strncpy(data, sptr, 5);
        if (strncmp(data, "     ", 5) == 0) {
          dlin[i].tmx = 0.;
        }
        else {
          dlin[i].tmx = atof(data);
        }
          
        i++;
      }
    }

    else if (strncmp("DGER", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "9999", 4) == 0) {
          break;
        }
      }
    }

    else if (strncmp("DGLT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
        } while (!feof(fd) && (line[0] == '('));
      
        if (strncmp(line, "99", 2) == 0) {
          break;
        }
      }
    }

    else if (strncmp("DGBT", excode, EXCODE_LENGHT) == 0) {

      for (;;) {
        
        do {
          fgets(line, LINE_LENGHT, fd);
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

int main (int argc, char **argv)
{
  FILE *fd;
  
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
  
  return 0;
}
